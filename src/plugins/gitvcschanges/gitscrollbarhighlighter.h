#pragma once
#include <QObject>

#include <libgit2cpp/gitrepo.h>

#include <utils/filepath.h>

namespace ProjectExplorer {
class ProjectVcsStatus;
}

namespace TextEditor {
class TextDocument;
class TextEditorWidget;
} // namespace TextEditor

namespace GitVcsChanges {
namespace Internal {

enum class MoveDirection { Next, Previous };

class GitScrollBarHighlighter : public QObject
{
    Q_OBJECT

public:
    GitScrollBarHighlighter(QObject *parent = nullptr);
    ~GitScrollBarHighlighter() override;

    void gotoUncommitedLine(GitVcsChanges::Internal::MoveDirection direction) const;

private:
    void scheduleHighlightUpdate();
    std::optional<int> currentLineNumber(TextEditor::TextEditorWidget *textEditorWidget) const;
    void updateScrollbarHighlight(const LibGit2Cpp::LineNumberClassMap &changedLines);
    void update();

private:
    ProjectExplorer::ProjectVcsStatus* m_projectVcsStatus;
    bool m_updateScheduled = false;
};

} // namespace Internal
} // namespace GitVcsChanges
