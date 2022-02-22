// License: GPL
#pragma once
#include "projectnodes.h"

#include <QObject>

namespace Core {
class IVersionControl;
enum class VcsChangeType;
}

namespace ProjectExplorer {

class ProjectVcsStatus : public QObject
{
    Q_OBJECT
public:
    static ProjectVcsStatus* instance();

    Core::VcsChangeType vcsStatusChanges(ProjectExplorer::Node *) const;

    ProjectVcsStatus(const ProjectVcsStatus&) = delete;
    ProjectVcsStatus &operator=(const ProjectVcsStatus &) = delete;

signals:
    void vcsStatusChanged();

private slots:
    void checkStatus();
    void updateProjectFolder(ProjectExplorer::FolderNode *node);

private:
    ProjectVcsStatus(QObject *parent = nullptr);
    ~ProjectVcsStatus() override;

    bool updateProjectStatusCache();

    using FilePath = Utils::FilePath;
    using VcsChangeSet = QHash<FilePath, Core::VcsChangeType>;
    struct ProjectStatus {
        VcsChangeSet fileChanges;
        mutable bool isFolderNodesCurrent{};
        mutable QSet<FolderNode*> folderNodes;
    };
    using ProjectStatusMap = QHash<FilePath, ProjectStatus>;

    ProjectStatusMap m_projectStatusCache;
    FilePath m_currentProject;
};

} // namespace ProjectExplorer
