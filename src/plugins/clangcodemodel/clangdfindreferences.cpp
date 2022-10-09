// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "clangdfindreferences.h"

#include "clangdast.h"
#include "clangdclient.h"

#include <coreplugin/documentmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/find/searchresultwindow.h>
#include <cplusplus/FindUsages.h>
#include <cppeditor/cppcodemodelsettings.h>
#include <cppeditor/cppfindreferences.h>
#include <cppeditor/cpptoolsreuse.h>
#include <languageclient/languageclientsymbolsupport.h>
#include <languageserverprotocol/lsptypes.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/session.h>
#include <texteditor/basefilefind.h>
#include <utils/filepath.h>

#include <QCheckBox>
#include <QMap>
#include <QSet>

using namespace Core;
using namespace CppEditor;
using namespace CPlusPlus;
using namespace LanguageClient;
using namespace LanguageServerProtocol;
using namespace ProjectExplorer;
using namespace TextEditor;
using namespace Utils;

namespace ClangCodeModel::Internal {

class ReferencesFileData {
public:
    QList<QPair<Range, QString>> rangesAndLineText;
    QString fileContent;
    ClangdAstNode ast;
};

class ClangdFindReferences::Private
{
public:
    Private(ClangdFindReferences *q) : q(q) {}

    ClangdClient *client() const { return qobject_cast<ClangdClient *>(q->parent()); }
    void handleFindUsagesResult(const QList<Location> &locations);
    void finishSearch();
    void reportAllSearchResultsAndFinish();
    void addSearchResultsForFile(const FilePath &file, const ReferencesFileData &fileData);
    std::optional<QString> getContainingFunctionName(const ClangdAstPath &astPath,
                                                       const Range& range);

    ClangdFindReferences * const q;
    QMap<DocumentUri, ReferencesFileData> fileData;
    QList<MessageId> pendingAstRequests;
    QPointer<SearchResult> search;
    bool canceled = false;
    bool categorize = false;
};

ClangdFindReferences::ClangdFindReferences(ClangdClient *client, TextDocument *document,
        const QTextCursor &cursor, const QString &searchTerm, bool categorize)
    : QObject(client), d(new ClangdFindReferences::Private(this))
{
    d->categorize = categorize;
    d->search = SearchResultWindow::instance()->startNewSearch(
                tr("C++ Usages:"),
                {},
                searchTerm,
                SearchResultWindow::SearchOnly,
                SearchResultWindow::PreserveCaseDisabled,
                "CppEditor");
    if (categorize)
        d->search->setFilter(new CppSearchResultFilter);
    connect(d->search, &SearchResult::activated, [](const SearchResultItem& item) {
        EditorManager::openEditorAtSearchResult(item);
    });
    SearchResultWindow::instance()->popup(IOutputPane::ModeSwitch | IOutputPane::WithFocus);

    const std::optional<MessageId> requestId = client->symbolSupport().findUsages(
                document, cursor, [self = QPointer(this)](const QList<Location> &locations) {
        if (self)
            self->d->handleFindUsagesResult(locations);
    });

    if (!requestId) {
        d->finishSearch();
        return;
    }
    QObject::connect(d->search, &SearchResult::canceled, this, [this, requestId] {
        d->client()->cancelRequest(*requestId);
        d->canceled = true;
        d->search->disconnect(this);
        d->finishSearch();
    });

    connect(client, &ClangdClient::initialized, this, [this] {
        // On a client crash, report all search results found so far.
        d->reportAllSearchResultsAndFinish();
    });
}

ClangdFindReferences::~ClangdFindReferences()
{
    delete d;
}

void ClangdFindReferences::Private::handleFindUsagesResult(const QList<Location> &locations)
{
    if (!search || canceled) {
        finishSearch();
        return;
    }
    search->disconnect(q);

    qCDebug(clangdLog) << "found" << locations.size() << "locations";
    if (locations.isEmpty()) {
        finishSearch();
        return;
    }

    QObject::connect(search, &SearchResult::canceled, q, [this] {
        canceled = true;
        search->disconnect(q);
        for (const MessageId &id : std::as_const(pendingAstRequests))
            client()->cancelRequest(id);
        pendingAstRequests.clear();
        finishSearch();
    });

    for (const Location &loc : locations)
        fileData[loc.uri()].rangesAndLineText.push_back({loc.range(), {}});
    for (auto it = fileData.begin(); it != fileData.end();) {
        const Utils::FilePath filePath = it.key().toFilePath();
        if (!filePath.exists()) { // https://github.com/clangd/clangd/issues/935
            it = fileData.erase(it);
            continue;
        }
        const QStringList lines = SymbolSupport::getFileContents(filePath);
        it->fileContent = lines.join('\n');
        for (auto &rangeWithText : it.value().rangesAndLineText) {
            const int lineNo = rangeWithText.first.start().line();
            if (lineNo >= 0 && lineNo < lines.size())
                rangeWithText.second = lines.at(lineNo);
        }
        ++it;
    }

    qCDebug(clangdLog) << "document count is" << fileData.size();
    if (!categorize) {
        qCDebug(clangdLog) << "skipping AST retrieval";
        reportAllSearchResultsAndFinish();
        return;
    }

    for (auto it = fileData.begin(); it != fileData.end(); ++it) {
        const TextDocument * const doc = client()->documentForFilePath(it.key().toFilePath());
        if (!doc)
            client()->openExtraFile(it.key().toFilePath(), it->fileContent);
        it->fileContent.clear();
        const auto docVariant = doc ? ClangdClient::TextDocOrFile(doc)
                                    : ClangdClient::TextDocOrFile(it.key().toFilePath());
        const auto astHandler = [sentinel = QPointer(q), this, loc = it.key()](
                const ClangdAstNode &ast, const MessageId &reqId) {
            qCDebug(clangdLog) << "AST for" << loc.toFilePath();
            if (!sentinel)
                return;
            if (!search || canceled)
                return;
            ReferencesFileData &data = fileData[loc];
            data.ast = ast;
            pendingAstRequests.removeOne(reqId);
            qCDebug(clangdLog) << pendingAstRequests.size() << "AST requests still pending";
            addSearchResultsForFile(loc.toFilePath(), data);
            fileData.remove(loc);
            if (pendingAstRequests.isEmpty()) {
                qDebug(clangdLog) << "retrieved all ASTs";
                finishSearch();
            }
        };
        const MessageId reqId = client()->getAndHandleAst(
                    docVariant, astHandler, ClangdClient::AstCallbackMode::AlwaysAsync, {});
        pendingAstRequests << reqId;
        if (!doc)
            client()->closeExtraFile(it.key().toFilePath());
    }
}

void ClangdFindReferences::Private::finishSearch()
{
    if (!client()->testingEnabled() && search) {
        search->finishSearch(canceled);
        search->disconnect(q);
    }
    emit q->done();
    q->deleteLater();
}

void ClangdFindReferences::Private::reportAllSearchResultsAndFinish()
{
    for (auto it = fileData.begin(); it != fileData.end(); ++it)
        addSearchResultsForFile(it.key().toFilePath(), it.value());
    finishSearch();
}

static Usage::Type getUsageType(const ClangdAstPath &path);

void ClangdFindReferences::Private::addSearchResultsForFile(const FilePath &file,
                                                            const ReferencesFileData &fileData)
{
    QList<SearchResultItem> items;
    qCDebug(clangdLog) << file << "has valid AST:" << fileData.ast.isValid();
    for (const auto &rangeWithText : fileData.rangesAndLineText) {
        const Range &range = rangeWithText.first;
        const ClangdAstPath astPath = getAstPath(fileData.ast, range);
        const Usage::Type usageType = fileData.ast.isValid() ? getUsageType(astPath)
                                                             : Usage::Type::Other;

        SearchResultItem item;
        item.setUserData(int(usageType));
        item.setStyle(CppEditor::colorStyleForUsageType(usageType));
        item.setFilePath(file);
        item.setMainRange(SymbolSupport::convertRange(range));
        item.setUseTextEditorFont(true);
        item.setLineText(rangeWithText.second);
        item.setContainingFunctionName(getContainingFunctionName(astPath, range));
        items << item;
    }
    if (client()->testingEnabled())
        emit q->foundReferences(items);
    else
        search->addResults(items, SearchResult::AddOrdered);
}

std::optional<QString> ClangdFindReferences::Private::getContainingFunctionName(
    const ClangdAstPath &astPath, const Range& range)
{
    const ClangdAstNode* containingFuncNode{nullptr};
    const ClangdAstNode* lastCompoundStmtNode{nullptr};

    for (auto it = astPath.crbegin(); it != astPath.crend(); ++it) {
        if (it->arcanaContains("CompoundStmt"))
            lastCompoundStmtNode = &*it;

        if (it->isFunction()) {
            if (lastCompoundStmtNode && lastCompoundStmtNode->hasRange()
                && lastCompoundStmtNode->range().contains(range)) {
                containingFuncNode = &*it;
                break;
            }
        }
    }

    if (!containingFuncNode || !containingFuncNode->isValid())
        return std::nullopt;

    return containingFuncNode->detail();
}

static Usage::Type getUsageType(const ClangdAstPath &path)
{
    bool potentialWrite = false;
    bool isFunction = false;
    const bool symbolIsDataType = path.last().role() == "type" && path.last().kind() == "Record";
    const auto isPotentialWrite = [&] { return potentialWrite && !isFunction; };
    for (auto pathIt = path.rbegin(); pathIt != path.rend(); ++pathIt) {
        if (pathIt->arcanaContains("non_odr_use_unevaluated"))
            return Usage::Type::Other;
        if (pathIt->kind() == "CXXDelete")
            return Usage::Type::Write;
        if (pathIt->kind() == "CXXNew")
            return Usage::Type::Other;
        if (pathIt->kind() == "Switch" || pathIt->kind() == "If")
            return Usage::Type::Read;
        if (pathIt->kind() == "Call")
            return isFunction ? Usage::Type::Other
                              : potentialWrite ? Usage::Type::WritableRef : Usage::Type::Read;
        if (pathIt->kind() == "CXXMemberCall") {
            const auto children = pathIt->children();
            if (children && children->size() == 1
                    && children->first() == path.last()
                    && children->first().arcanaContains("bound member function")) {
                return Usage::Type::Other;
            }
            return isPotentialWrite() ? Usage::Type::WritableRef : Usage::Type::Read;
        }
        if ((pathIt->kind() == "DeclRef" || pathIt->kind() == "Member")
                && pathIt->arcanaContains("lvalue")) {
            if (pathIt->arcanaContains(" Function "))
                isFunction = true;
            else
                potentialWrite = true;
        }
        if (pathIt->role() == "declaration") {
            if (symbolIsDataType)
                return Usage::Type::Other;
            if (pathIt->arcanaContains("cinit")) {
                if (pathIt == path.rbegin())
                    return Usage::Type::Initialization;
                if (pathIt->childContainsRange(0, path.last().range()))
                    return Usage::Type::Initialization;
                if (isFunction)
                    return Usage::Type::Read;
                if (!pathIt->hasConstType())
                    return Usage::Type::WritableRef;
                return Usage::Type::Read;
            }
            return Usage::Type::Declaration;
        }
        if (pathIt->kind() == "MemberInitializer")
            return pathIt == path.rbegin() ? Usage::Type::Write : Usage::Type::Read;
        if (pathIt->kind() == "UnaryOperator"
                && (pathIt->detailIs("++") || pathIt->detailIs("--"))) {
            return Usage::Type::Write;
        }

        // LLVM uses BinaryOperator only for built-in types; for classes, CXXOperatorCall
        // is used. The latter has an additional node at index 0, so the left-hand side
        // of an assignment is at index 1.
        const bool isBinaryOp = pathIt->kind() == "BinaryOperator";
        const bool isOpCall = pathIt->kind() == "CXXOperatorCall";
        if (isBinaryOp || isOpCall) {
            if (isOpCall && symbolIsDataType) // Constructor invocation.
                return Usage::Type::Other;

            const QString op = pathIt->operatorString();
            if (op.endsWith("=") && op != "==") { // Assignment.
                const int lhsIndex = isBinaryOp ? 0 : 1;
                if (pathIt->childContainsRange(lhsIndex, path.last().range()))
                    return Usage::Type::Write;
                return isPotentialWrite() ? Usage::Type::WritableRef : Usage::Type::Read;
            }
            return Usage::Type::Read;
        }

        if (pathIt->kind() == "ImplicitCast") {
            if (pathIt->detailIs("FunctionToPointerDecay"))
                return Usage::Type::Other;
            if (pathIt->hasConstType())
                return Usage::Type::Read;
            potentialWrite = true;
            continue;
        }
    }

    return Usage::Type::Other;
}

class ClangdFindLocalReferences::Private
{
public:
    Private(ClangdFindLocalReferences *q, TextDocument *document, const QTextCursor &cursor,
            const RenameCallback &callback)
        : q(q), document(document), cursor(cursor), callback(callback),
          uri(DocumentUri::fromFilePath(document->filePath())),
          revision(document->document()->revision())
    {}

    ClangdClient *client() const { return qobject_cast<ClangdClient *>(q->parent()); }
    void findDefinition();
    void getDefinitionAst(const Link &link);
    void checkDefinitionAst(const ClangdAstNode &ast);
    void handleReferences(const QList<Location> &references);
    void finish();

    ClangdFindLocalReferences * const q;
    const QPointer<TextDocument> document;
    const QTextCursor cursor;
    RenameCallback callback;
    const DocumentUri uri;
    const int revision;
    Link defLink;
};

ClangdFindLocalReferences::ClangdFindLocalReferences(
        ClangdClient *client, TextDocument *document, const QTextCursor &cursor,
        const RenameCallback &callback)
    : QObject(client), d(new Private(this, document, cursor, callback))
{
    d->findDefinition();
}

ClangdFindLocalReferences::~ClangdFindLocalReferences()
{
    delete d;
}

void ClangdFindLocalReferences::Private::findDefinition()
{
    const auto linkHandler = [sentinel = QPointer(q), this](const Link &l) {
        if (sentinel)
            getDefinitionAst(l);
    };
    client()->symbolSupport().findLinkAt(document, cursor, linkHandler, true);
}

void ClangdFindLocalReferences::Private::getDefinitionAst(const Link &link)
{
    qCDebug(clangdLog) << "received go to definition response" << link.targetFilePath
                       << link.targetLine << (link.targetColumn + 1);

    if (!link.hasValidTarget() || !document
            || link.targetFilePath.canonicalPath() != document->filePath().canonicalPath()) {
        finish();
        return;
    }

    defLink = link;
    qCDebug(clangdLog) << "sending ast request for link";
    const auto astHandler = [sentinel = QPointer(q), this]
            (const ClangdAstNode &ast, const MessageId &) {
        if (sentinel)
            checkDefinitionAst(ast);
    };
    client()->getAndHandleAst(document, astHandler, ClangdClient::AstCallbackMode::SyncIfPossible,
                              {});
}

void ClangdFindLocalReferences::Private::checkDefinitionAst(const ClangdAstNode &ast)
{
    qCDebug(clangdLog) << "received ast response";
    if (!ast.isValid() || !document) {
        finish();
        return;
    }

    const Position linkPos(defLink.targetLine - 1, defLink.targetColumn);
    const ClangdAstPath astPath = getAstPath(ast, linkPos);
    bool isVar = false;
    for (auto it = astPath.rbegin(); it != astPath.rend(); ++it) {
        if (it->role() == "declaration"
                && (it->kind() == "Function" || it->kind() == "CXXMethod"
                    || it->kind() == "CXXConstructor" || it->kind() == "CXXDestructor"
                    || it->kind() == "Lambda")) {
            if (!isVar)
                break;

            qCDebug(clangdLog) << "finding references for local var";
            const auto refsHandler = [sentinel = QPointer(q), this](const QList<Location> &refs) {
                if (sentinel)
                    handleReferences(refs);
            };
            client()->symbolSupport().findUsages(document, cursor, refsHandler);
            return;
        }
        if (!isVar && it->role() == "declaration"
                && (it->kind() == "Var" || it->kind() == "ParmVar")) {
            isVar = true;
        }
    }
    finish();
}

void ClangdFindLocalReferences::Private::handleReferences(const QList<Location> &references)
{
    qCDebug(clangdLog) << "found" << references.size() << "local references";
    const Utils::Links links = Utils::transform(references, &Location::toLink);

    // The callback only uses the symbol length, so we just create a dummy.
    // Note that the calculation will be wrong for identifiers with
    // embedded newlines, but we've never supported that.
    QString symbol;
    if (!references.isEmpty()) {
        const Range r = references.first().range();
        symbol = QString(r.end().character() - r.start().character(), 'x');
    }
    callback(symbol, links, revision);
    callback = {};
    finish();
}

void ClangdFindLocalReferences::Private::finish()
{
    if (callback)
        callback({}, {}, revision);
    emit q->done();
}

} // namespace ClangCodeModel::Internal

#include "moc_clangdfindreferences.cpp"
