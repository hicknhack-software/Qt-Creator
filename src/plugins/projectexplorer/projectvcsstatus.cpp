// License: GPL
#include "coreplugin/iversioncontrol.h"
#include "coreplugin/vcsmanager.h"
#include "coreplugin/editormanager/editormanager.h"
#include "projecttree.h"
#include "projectvcsstatus.h"
#include "session.h"
#include "project.h"

#include <QApplication>
#include <QJsonArray>
#include <QTimer>

namespace ProjectExplorer {

namespace {

QSet<FolderNode*> extractFolderNodes(const QHash<Utils::FilePath, Core::VcsChangeType> &fileChanges, const Utils::FilePath& topLevel)
{
    QSet<FolderNode*> result;
    auto *topNode = ProjectTree::nodeForFile(topLevel);
    if (topNode == nullptr) {
        return result;
    }
    auto *topFolder = topNode->asFolderNode();
    if (topFolder == nullptr) {
        return result;
    }
    result.insert(topFolder);
    for (auto it = fileChanges.keyBegin(); it != fileChanges.keyEnd(); ++it) {
        auto *node = ProjectTree::nodeForFile(*it);
        if (node == nullptr) {
            continue;
        }
        auto *folderNode = node->parentFolderNode();
        while (folderNode && !result.contains(folderNode)) {
            result.insert(folderNode);
            folderNode = folderNode->parentFolderNode();
        }
    }
    return result;
}

} // namespace

ProjectVcsStatus::ProjectVcsStatus(QObject *parent) : QObject(parent)
{
    connect(Core::EditorManager::instance(), &Core::EditorManager::saved,
            this, &ProjectVcsStatus::checkStatus);
    connect(Core::VcsManager::instance(), &Core::VcsManager::repositoryChanged,
            this, &ProjectVcsStatus::checkStatus);
    connect(ProjectTree::instance(), &ProjectTree::subtreeChanged,
            this, &ProjectVcsStatus::updateProjectFolder);
    connect(ProjectTree::instance(), &ProjectTree::currentProjectChanged,
            this, [&] (ProjectExplorer::Project *project) {
                if (project != nullptr) {
                    m_currentProject = project->projectDirectory();
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
                auto projectPath = project->rootProjectDirectory();
                auto *vcs = Core::VcsManager::findVersionControlForDirectory(projectPath);
                if (!m_projectStatusCache.contains(projectPath) && vcs != nullptr) {
                    qDebug() << "Register project" << projectPath;
                    m_projectStatusCache.insert(projectPath, ProjectStatus{});
                }
            });
    connect(SessionManager::instance(), &SessionManager::projectRemoved,
            this, [&] (ProjectExplorer::Project *project){
                auto projectPath = project->rootProjectDirectory();
                auto iter = m_projectStatusCache.find(projectPath);
                if (iter != m_projectStatusCache.end()) {
                    qDebug() << "Deregister project" << projectPath;
                    m_projectStatusCache.erase(iter);
                }
            });
}

ProjectVcsStatus::~ProjectVcsStatus()
{}

ProjectVcsStatus *ProjectVcsStatus::instance()
{
    static ProjectVcsStatus *s_instance = new ProjectVcsStatus();
    return s_instance;
}

void ProjectVcsStatus::checkStatus()
{
    if (m_projectStatusCache.empty()) {
        return;
    }
    if (updateProjectStatusCache()) {
        emit vcsStatusChanged();
    }
}

void ProjectVcsStatus::updateProjectFolder(FolderNode *node)
{
    if (node == nullptr) {
        return;
    }
    auto *project = node->getProject();
    if (project == nullptr) {
        return;
    }
    auto projectPath = project->projectDirectory();
    auto cacheIter = m_projectStatusCache.find(projectPath);
    if (cacheIter == m_projectStatusCache.end()) {
        return;
    }
    auto &changeSet = cacheIter.value();
    changeSet.isFolderNodesCurrent = false;
}

Core::VcsChangeType ProjectVcsStatus::vcsStatusChanges(ProjectExplorer::Node *node) const
{
    using Core::VcsChangeType;
    if (node == nullptr) {
        return VcsChangeType::Unchanged;
    }
    auto *project = node->getProject();
    if (project == nullptr) {
        return VcsChangeType::Unchanged;
    }
    auto projectPath = project->projectDirectory();
    auto cacheIter = m_projectStatusCache.find(projectPath);
    if (cacheIter == m_projectStatusCache.end()) {
        return VcsChangeType::Unchanged;
    }
    auto &changeSet = cacheIter.value();
    if (auto *fileNode = node->asFileNode()) {
        auto filePath = fileNode->filePath();
        if (auto iter = changeSet.fileChanges.find(filePath); iter != changeSet.fileChanges.end()) {
            return iter.value();
        }
    }
    else if (auto *folderNode = node->asFolderNode()) {
        if (!changeSet.isFolderNodesCurrent) {
            changeSet.folderNodes = extractFolderNodes(changeSet.fileChanges, projectPath);
            changeSet.isFolderNodesCurrent = true;
        }
        if (auto iter = changeSet.folderNodes.find(folderNode); iter != changeSet.folderNodes.end()) {
            return VcsChangeType::FolderContainsChanges;
        }
    }
    return VcsChangeType::Unchanged;
}

bool ProjectVcsStatus::updateProjectStatusCache()
{
    const auto &currentProject = m_currentProject;
    if (currentProject.isEmpty()) {
        return false;
    }
    QString topLevelDirectory;
    auto *vcs = Core::VcsManager::findVersionControlForDirectory(currentProject, &topLevelDirectory);
    if (vcs == nullptr) {
        return false;
    }
    auto topLevelPath = Utils::FilePath::fromString(topLevelDirectory);
    auto newChangeSet = vcs->localChanges(topLevelPath);
    auto &changeSet = m_projectStatusCache[currentProject];
    if (newChangeSet == changeSet.fileChanges) {
        return false;
    }
    changeSet.fileChanges = newChangeSet;
    changeSet.isFolderNodesCurrent = false;
    return true;
}

} // namespace ProjectExplorer
