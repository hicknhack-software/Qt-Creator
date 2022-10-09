// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "clangeditordocumentprocessor.h"

#include "clangmodelmanagersupport.h"

#include <cppeditor/builtincursorinfo.h>
#include <cppeditor/builtineditordocumentparser.h>
#include <cppeditor/clangdiagnosticconfigsmodel.h>
#include <cppeditor/compileroptionsbuilder.h>
#include <cppeditor/cppcodemodelsettings.h>
#include <cppeditor/cppmodelmanager.h>
#include <cppeditor/cpptoolsreuse.h>
#include <cppeditor/cppworkingcopy.h>
#include <cppeditor/editordocumenthandle.h>

#include <texteditor/fontsettings.h>
#include <texteditor/texteditor.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>

#include <cplusplus/CppDocument.h>

#include <utils/algorithm.h>
#include <utils/textutils.h>
#include <utils/qtcassert.h>
#include <utils/runextensions.h>

#include <QTextBlock>
#include <QVBoxLayout>
#include <QWidget>

namespace ClangCodeModel {
namespace Internal {

ClangEditorDocumentProcessor::ClangEditorDocumentProcessor(TextEditor::TextDocument *document)
    : BuiltinEditorDocumentProcessor(document), m_document(*document)
{
    connect(parser().data(), &CppEditor::BaseEditorDocumentParser::projectPartInfoUpdated,
            this, &BaseEditorDocumentProcessor::projectPartInfoUpdated);
    connect(static_cast<CppEditor::BuiltinEditorDocumentParser *>(parser().data()),
            &CppEditor::BuiltinEditorDocumentParser::finished,
            this, [this] {
        emit parserConfigChanged(Utils::FilePath::fromString(filePath()), parserConfig());
    });
    setSemanticHighlightingChecker([this] {
        return !ClangModelManagerSupport::clientForFile(m_document.filePath());
    });
}

void ClangEditorDocumentProcessor::semanticRehighlight()
{
    const auto matchesEditor = [this](const Core::IEditor *editor) {
        return editor->document()->filePath() == m_document.filePath();
    };
    if (!Utils::contains(Core::EditorManager::visibleEditors(), matchesEditor))
        return;
    if (ClangModelManagerSupport::clientForFile(m_document.filePath()))
        return;
    BuiltinEditorDocumentProcessor::semanticRehighlight();
}

bool ClangEditorDocumentProcessor::hasProjectPart() const
{
    return !m_projectPart.isNull();
}

CppEditor::ProjectPart::ConstPtr ClangEditorDocumentProcessor::projectPart() const
{
    return m_projectPart;
}

void ClangEditorDocumentProcessor::clearProjectPart()
{
    m_projectPart.clear();
}

::Utils::Id ClangEditorDocumentProcessor::diagnosticConfigId() const
{
    return m_diagnosticConfigId;
}

void ClangEditorDocumentProcessor::setParserConfig(
        const CppEditor::BaseEditorDocumentParser::Configuration &config)
{
    CppEditor::BuiltinEditorDocumentProcessor::setParserConfig(config);
    emit parserConfigChanged(Utils::FilePath::fromString(filePath()), config);
}

CppEditor::BaseEditorDocumentParser::Configuration ClangEditorDocumentProcessor::parserConfig()
{
    return parser()->configuration();
}

ClangEditorDocumentProcessor *ClangEditorDocumentProcessor::get(const QString &filePath)
{
    return qobject_cast<ClangEditorDocumentProcessor*>(
                CppEditor::CppModelManager::cppEditorDocumentProcessor(filePath));
}

} // namespace Internal
} // namespace ClangCodeModel

#include "moc_clangeditordocumentprocessor.cpp"
