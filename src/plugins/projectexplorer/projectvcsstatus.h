#pragma once

#include <projectexplorer/projectnodes.h>

#include <QObject>

#include <tuple>
#include <optional>

namespace Core {
class IVersionControl;
enum class VcsChangeType;
}

namespace ProjectExplorer {

class ProjectVcsStatus : public QObject
{
    Q_OBJECT

public:
    using VcsChangeSet = QHash<QString, Core::VcsChangeType>;

    using ProjectStatusMap = QHash<QString, VcsChangeSet>;
    using NodeFileList = QList<ProjectExplorer::FileNode *>;

    using UnSafeFolderNodePtr = const ProjectExplorer::FolderNode *;
    using UnSafeFileNodePtr = const ProjectExplorer::FileNode *;
    using UnSafeFolderNodeSet = QSet<UnSafeFolderNodePtr>;

    using FileNodeFolderMap = QHash<UnSafeFileNodePtr, UnSafeFolderNodeSet>;
    using FileNodeMap = QHash<QString, UnSafeFileNodePtr>;
    using NodeList = QList<QString>;

    static ProjectVcsStatus* instance();

    std::optional<Core::VcsChangeType> vcsStatusChanges(ProjectExplorer::Node *) const;

    ProjectVcsStatus(const ProjectVcsStatus&) = delete;
    ProjectVcsStatus &operator=(const ProjectVcsStatus &) = delete;

signals:
    void vcsNodeStatusChanged(const ProjectExplorer::Node *);
    void vcsStatusChanged();

private slots:
    void checkStatus();

private:
    ProjectVcsStatus(QObject *parent = nullptr);
    ~ProjectVcsStatus() override;

    bool updateProjectStatusCache();
    inline QString folderPathOf(const ProjectExplorer::FolderNode * node) const {
        if (node->filePath().isFile() &&
            node->asFolderNode() != nullptr) {
            return node->filePath().parentDir().toString();
        }
        return node->filePath().toString();
    }
    inline bool hasSameStartPath(const QString & parent, const QString & child) const
    {
        if (parent.isEmpty())
            return false;
        if (!child.startsWith(parent))
            return false;
        if (child.size() <= parent.size())
            return false;
        // s is root, '/' was already tested in startsWith
        if (parent.endsWith(QLatin1Char('/')))
            return true;
        // s is a directory, next character should be '/' (/tmpdir is NOT a child of /tmp)
        return child.at(parent.size()) == QLatin1Char('/');
    }
    NodeFileList changedFileNodesForCurrentProject() const;
    NodeList updateFileNodeCache(NodeFileList & changedFileNodes);
    void removeDisappearedFileNodes(const NodeList &);
    ProjectExplorer::FolderNode* findResourceTopLevelNode(const ProjectExplorer::FolderNode*) const;

private:
    static ProjectVcsStatus * m_instance;
    ProjectStatusMap m_projectStatusCache;
    QString m_currentProject;

    FileNodeFolderMap m_fileNodeFolderMap;
    FileNodeMap m_filePathNodeMap;
};

}// namespace ProjectExplorer
