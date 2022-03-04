#include "coreplugin/iversioncontrol.h"
#include "coreplugin/vcsmanager.h"
#include "coreplugin/editormanager/editormanager.h"
#include "qbsprojectmanager/qbsnodes.h"
#include "projecttree.h"
#include "projectvcsstatus.h"
#include "session.h"

#include <qbsprojectmanager/qbsnodes.h>

#include <QApplication>
#include <QJsonArray>
#include <QTimer>

#include <algorithm>

using namespace Core;

namespace ProjectExplorer {

using NodeList = ProjectVcsStatus::NodeList;
using NodeFileList = ProjectVcsStatus::NodeFileList;
using ChangedFileNodeMap = ProjectVcsStatus::FileNodeFolderMap;

ProjectVcsStatus* ProjectVcsStatus::m_instance;

ProjectVcsStatus::ProjectVcsStatus(QObject *parent) : QObject(parent)
{
    connect(EditorManager::instance(), &EditorManager::saved,
            this, &ProjectVcsStatus::checkStatus);
    connect(VcsManager::instance(), &VcsManager::repositoryChanged,
            this, &ProjectVcsStatus::checkStatus);
    connect(ProjectTree::instance(), &ProjectTree::treeChanged,
            this, &ProjectVcsStatus::checkStatus);
    connect(ProjectTree::instance(), &ProjectTree::currentProjectChanged,
            this, [&] (ProjectExplorer::Project *project) {
                if (project != nullptr) {
                    m_currentProject = project->projectDirectory().toString();
                }
                else {
                    m_currentProject.clear();
                }
            });
    connect(qApp, &QApplication::applicationStateChanged, this, [=] (Qt::ApplicationState state) {
        if (state == Qt::ApplicationState::ApplicationActive) {
            checkStatus();
        }
    });
    connect(SessionManager::instance(), &SessionManager::projectAdded,
            this, [&] (ProjectExplorer::Project *project){
                auto projectPath = project->rootProjectDirectory().toString();
                auto * vcs = Core::VcsManager::findVersionControlForDirectory(Utils::FilePath::fromString(projectPath));
                if (!m_projectStatusCache.contains(projectPath) && vcs != nullptr) {
                    qDebug() << "Register project" << projectPath;
                    m_projectStatusCache.insert(projectPath, ChangeSets{});
                }
            });
    connect(SessionManager::instance(), &SessionManager::projectRemoved,
            this, [&] (ProjectExplorer::Project *project){
                auto projectPath = project->rootProjectDirectory().toString();
                auto iter = m_projectStatusCache.find(projectPath);
                if (iter != m_projectStatusCache.end()) {
                    qDebug() << "Deregister project" << projectPath;
                    m_projectStatusCache.erase(iter);
                }
            });
}

ProjectVcsStatus::~ProjectVcsStatus()
{}

ProjectVcsStatus* ProjectVcsStatus::instance()
{
    if (m_instance == nullptr) {
        m_instance = new ProjectVcsStatus();
    }
    return m_instance;
}

void ProjectVcsStatus::checkStatus()
{
    if (m_projectStatusCache.empty()) {
        return;
    }
    updateProjectStatusCache();
    auto changedFileNodes = changedFileNodesForCurrentProject();
    auto insertedNodes = updateFileNodeCache(changedFileNodes);
    removeDisappearedFileNodes(insertedNodes);
    emit vcsStatusChanged();
}

bool ProjectVcsStatus::hasVcsStatusChanges(ProjectExplorer::Node * node) const
{
    if (node == nullptr) {
        return false;
    }
    if (auto * fileNode = node->asFileNode()) {
        return m_fileNodeFolderMap.contains(fileNode);
    }
    else if (auto * folderNode = node->asFolderNode()) {
        for (auto iter = m_filePathNodeMap.constBegin(); iter != m_filePathNodeMap.constEnd(); ++iter) {
            if (hasSameStartPath(folderPathOf(folderNode), iter.key()) && m_fileNodeFolderMap.value(iter.value()).contains(folderNode)) {
                return true;
            }
        }
    }
    return false;
}

bool ProjectVcsStatus::updateProjectStatusCache()
{
    bool hasTrackedChanges = false;
    bool hasUnTrackedChanges = false;
    if (m_currentProject.count() != 0) {
        auto & changeSet = m_projectStatusCache[m_currentProject];
        auto * vcs = Core::VcsManager::findVersionControlForDirectory(Utils::FilePath::fromString(m_currentProject));
        if (vcs != nullptr) {
            auto [newTrackedChanges, newUntrackedChanges] = vcs->localChanges(Utils::FilePath::fromString(m_currentProject));
            hasTrackedChanges |= newTrackedChanges != changeSet.trackedChanges;
            if (hasTrackedChanges) {
                changeSet.trackedChanges = newTrackedChanges;
            }
            hasUnTrackedChanges |= newUntrackedChanges != changeSet.untrackedChanges;
            if (hasUnTrackedChanges) {
                changeSet.untrackedChanges = newUntrackedChanges;
            }
        }
    }
    return hasTrackedChanges || hasUnTrackedChanges;
}

NodeFileList ProjectVcsStatus::changedFileNodesForCurrentProject() const
{
    auto * project = ProjectTree::currentProject();
    if (project == nullptr || ! m_projectStatusCache.contains(project->projectDirectory().toString())) {
        return {};
    }
    auto * projectNode = project->rootProjectNode();
    if (projectNode == nullptr) {
        return {};
    }
    QHash<QString, ProjectExplorer::FileNode *> fileMap;
    const auto & projectStatusCache = m_projectStatusCache.value(project->projectDirectory().toString()).trackedChanges;
    if (projectNode != nullptr) {
        projectNode->forEachNode(
            [&](FileNode * fileNode) {
                if (fileNode != nullptr) {
                    auto filePathString = fileNode->filePath().toString();
                    auto result = projectStatusCache.find(filePathString);
                    if (result != projectStatusCache.end()) {
                        if (! fileMap.contains(filePathString)) {
                            fileMap.insert(filePathString, fileNode);
                        }
                    }
                }
            },
            {},
            [&](const FolderNode * folderNode) -> bool {
                if (folderNode == nullptr) {
                    return false;
                }
                return projectStatusCache.contains(folderPathOf(folderNode));
            });
    }
    return fileMap.values();
}

NodeList ProjectVcsStatus::updateFileNodeCache(NodeFileList & changedFileNodes)
{
    auto insertUpdateList = NodeList{};
    for (auto * fileNode: changedFileNodes) {
        auto * parentFolderNode = fileNode->parentFolderNode();
        insertUpdateList.append(fileNode->filePath().toString());
        auto & fileNodeSet = m_fileNodeFolderMap[fileNode];
        fileNodeSet.clear();
        m_filePathNodeMap[fileNode->filePath().toString()] = fileNode;
        while (parentFolderNode != nullptr) {
            fileNodeSet.insert(parentFolderNode);
            parentFolderNode = parentFolderNode->parentFolderNode();
        }
    }
    return insertUpdateList;
}

void ProjectVcsStatus::removeDisappearedFileNodes(const NodeList & insertedNodes)
{
    auto iter = m_filePathNodeMap.begin();
    while (iter != m_filePathNodeMap.end()) {
        if (! insertedNodes.contains(iter.key())) {
            m_fileNodeFolderMap.remove(iter.value());
            iter = m_filePathNodeMap.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

}// namespace ProjectExplorer
