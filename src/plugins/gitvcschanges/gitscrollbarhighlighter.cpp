#include "gitscrollbarhighlighter.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/find/highlightscrollbarcontroller.h>
#include <coreplugin/icore.h>
#include <coreplugin/imode.h>
#include <coreplugin/iversioncontrol.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/vcsmanager.h>

#include <projectexplorer/project.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/projectvcsstatus.h>
#include <projectexplorer/session.h>

#include <texteditor/textdocument.h>
#include <texteditor/textdocumentlayout.h>
#include <texteditor/texteditor.h>
#include <texteditor/texteditorconstants.h>

#include <QAction>
#include <QtConcurrent>

#include <optional>
#include <type_traits>
#include <unordered_map>

namespace GitVcsChanges {
namespace Internal {

using namespace ProjectExplorer;
using namespace Core;
using namespace LibGit2Cpp;

using ScrollbarSegment = Highlight::ScrollbarSegment;

GitScrollBarHighlighter::GitScrollBarHighlighter(QObject *parent)
    : QObject{parent}
{
    m_projectVcsStatus = ProjectVcsStatus::instance();
    Q_ASSERT(m_projectVcsStatus != nullptr);
    connect(m_projectVcsStatus, &ProjectVcsStatus::currentFileDiffChanged, this, &GitScrollBarHighlighter::scheduleHighlightUpdate);
}

std::optional<int> GitScrollBarHighlighter::currentLineNumber(
    TextEditor::TextEditorWidget *textEditorWidget) const
{
    if (textEditorWidget != nullptr) {
        QTextCursor cursor = textEditorWidget->textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);

        int lines = 1;
        while (cursor.positionInBlock() > 0) {
            cursor.movePosition(QTextCursor::Up);
            lines++;
        }
        QTextBlock block = cursor.block().previous();

        while (block.isValid()) {
            lines += block.lineCount();
            block = block.previous();
        }
        return lines;
    }
    return std::nullopt;
}

void GitScrollBarHighlighter::gotoUncommitedLine(MoveDirection direction) const
{
    auto *editorManager = Core::EditorManager::instance();
    if (editorManager != nullptr) {
        auto *textEditorWidget = qobject_cast<TextEditor::TextEditorWidget *>(
            editorManager->currentEditor()->widget());
        Q_ASSERT(textEditorWidget != nullptr);

        auto currentLine = currentLineNumber(textEditorWidget);
        if (currentLine.has_value()) {
            auto futureLine = [=] () -> std::optional<int> {
                if (direction == MoveDirection::Next) {
                    return m_projectVcsStatus->nextChangedLineNumber(*currentLine);
                }
                return m_projectVcsStatus->previousChangedLineNumber(*currentLine);
            };
            if (auto line = futureLine(); line.has_value()) {
                return textEditorWidget->gotoLine(*line, 0);
            }
        }
    }
}

GitScrollBarHighlighter::~GitScrollBarHighlighter() {}

void GitScrollBarHighlighter::scheduleHighlightUpdate()
{
    if (m_updateScheduled)
        return;

    m_updateScheduled = true;
    QMetaObject::invokeMethod(this, &GitScrollBarHighlighter::update, Qt::QueuedConnection);
}

void GitScrollBarHighlighter::update()
{
    m_updateScheduled = false;
    const auto* fileDiff = m_projectVcsStatus->currentVcsFileDiff();
    if (fileDiff != nullptr) {
        updateScrollbarHighlight(fileDiff->fileChangeSet);
    }
}

void GitScrollBarHighlighter::updateScrollbarHighlight(const LineNumberClassMap &changedLines)
{
    auto *editorManager = Core::EditorManager::instance();
    if (editorManager != nullptr && editorManager->currentEditor() != nullptr) {
        auto *currentEditor = editorManager->currentEditor();
        if (auto *textEditorWidget = qobject_cast<TextEditor::TextEditorWidget *>(
                currentEditor->widget())) {
            auto *highlighterController = textEditorWidget->highlightScrollBarController();
            if (highlighterController != nullptr) {
                auto *textDocument = qobject_cast<TextEditor::TextDocument *>(currentEditor->document());
                Q_ASSERT(textDocument != nullptr);
                auto lineCount = textDocument->document()->lineCount();

                highlighterController->removeHighlights(
                    TextEditor::Constants::SCROLL_BAR_CHANGED_LINES);

                for (auto iter = changedLines.cbegin(); iter != changedLines.cend(); ++iter) {
                    auto wasAdded = (iter->second & ChangeClass::Added) == ChangeClass::Added;
                    auto wasDeleted = (iter->second & ChangeClass::Deleted) == ChangeClass::Deleted;

                    if (iter->first > lineCount) {
                        continue;
                    }
                    if (wasAdded && wasDeleted) {
                        highlighterController->addHighlight(
                            {TextEditor::Constants::SCROLL_BAR_CHANGED_LINES,
                             iter->first - 1,
                             Utils::Theme::TextEditor_AddedAndDeletedLine_ScrollBarColor,
                             Highlight::HighPriority,
                             ScrollbarSegment::Left});
                    } else if (wasAdded) {
                        highlighterController->addHighlight(
                            {TextEditor::Constants::SCROLL_BAR_CHANGED_LINES,
                             iter->first - 1,
                             Utils::Theme::TextEditor_AddedLine_ScrollBarColor,
                             Highlight::HighPriority,
                             ScrollbarSegment::Left});
                    } else {
                        highlighterController->addHighlight(
                            {TextEditor::Constants::SCROLL_BAR_CHANGED_LINES,
                             iter->first - 1,
                             Utils::Theme::TextEditor_DeletedLine_ScrollBarColor,
                             Highlight::HighPriority,
                             ScrollbarSegment::Left});
                    }
                }
            }
        }
    }
}

} // namespace Internal
} // namespace GitVcsChanges
