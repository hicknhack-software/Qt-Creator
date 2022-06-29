// License: GPL
#pragma once
#include "projectnodes.h"

#include <libgit2cpp/gitrepo.h>

#include <QObject>

#include <unordered_map>
#include <optional>

namespace Core {
class IEditor;
class IDocument;
} // namespace Core

namespace ProjectExplorer {

using LibGit2Cpp::VcsChangeType;

class PROJECTEXPLORER_EXPORT ProjectVcsStatus : public QObject
{
    Q_OBJECT
public:
    struct FileDiff {
        LibGit2Cpp::LineNumberClassMap fileChangeSet;
    };

    static ProjectVcsStatus *instance();

    VcsChangeType vcsStatusChanges(ProjectExplorer::Node *) const;
    const FileDiff* currentVcsFileDiff() const;
    std::optional<int> nextChangedLineNumber(int currentLineNumber);
    std::optional<int> previousChangedLineNumber(int currentLineNumber);

    ProjectVcsStatus(const ProjectVcsStatus &) = delete;
    ProjectVcsStatus &operator=(const ProjectVcsStatus &) = delete;

signals:
    void vcsStatusChanged();
    void currentFileDiffChanged();

private slots:
    void checkStatus();
    void updateProjectFolder(ProjectExplorer::FolderNode *node);

private:
    ProjectVcsStatus(QObject *parent = nullptr);
    ~ProjectVcsStatus() override;

    void handleChangedFileDiff(Utils::FilePath filePath, LibGit2Cpp::LineNumberClassMap fileDiff);
    void handleReceivedFileChangeSet(Utils::FilePath repositoryPath, LibGit2Cpp::VcsChangeSet fileChangeSet);
    void setCurrentDocument(Core::IEditor *editor);
    void handleClosedDocument(Core::IDocument *document);

    struct ProjectStatus
    {
        LibGit2Cpp::GitRepo gitRepo;
        LibGit2Cpp::VcsChangeSet fileChangeSet;
        mutable bool isFolderNodesCurrent;
        mutable QSet<FolderNode *> folderNodes;

        ProjectStatus(const Utils::FilePath& path)
            : gitRepo(path){}
    };
    using ProjectStatusMap = std::unordered_map<Utils::FilePath, ProjectStatus>;
    using FileDiffPointer = std::unique_ptr<FileDiff>;
    using FileDiffCache = std::unordered_map<Utils::FilePath, FileDiffPointer>;

    FileDiffCache m_fileDiffCache;
    ProjectStatusMap m_projectStatusCache;
    Utils::FilePath m_currentProject;
    Core::IDocument *m_currentDocument = nullptr;
};

} // namespace ProjectExplorer
