// License: GPL
#include "projectvcsstatus.h"
#include "project.h"
#include "projecttree.h"
#include "session.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/vcsmanager.h>
#include <libgit2cpp/gitrepo.h>

#include <QApplication>
#include <QtConcurrent>
#include <QJsonArray>
#include <QTimer>

namespace ProjectExplorer {

namespace {

QSet<FolderNode *> extractFolderNodes(const LibGit2Cpp::VcsChangeSet &fileChangeSet,
                                      Project *project)
{
    QSet<FolderNode *> result;
    if (fileChangeSet.empty()) {
        return result; // no changes
    }
    if (auto *containerNode = project->containerNode()) {
        result.insert(containerNode);
    }
    if (auto *projectNode = project->rootProjectNode()) {
        result.insert(projectNode);
    }
    for (auto it = fileChangeSet.keyBegin(); it != fileChangeSet.keyEnd(); ++it) {
        if (auto *fileNode = ProjectTree::nodeForFile(*it)) {
            for (auto *folderNode = fileNode->parentFolderNode(); folderNode;
                 folderNode = folderNode->parentFolderNode()) {
                if (result.contains(folderNode)) {
                    break;
                }
                result.insert(folderNode);
            }
        }
    }
    return result;
}

} // namespace

ProjectVcsStatus::ProjectVcsStatus(QObject *parent)
    : QObject(parent)
{
    LibGit2Cpp::init();

    connect(Core::EditorManager::instance(),
            &Core::EditorManager::saved,
            this,
            &ProjectVcsStatus::checkStatus);
    connect(Core::VcsManager::instance(),
            &Core::VcsManager::repositoryChanged,
            this,
            &ProjectVcsStatus::checkStatus);
    connect(ProjectTree::instance(), &ProjectTree::subtreeChanged, this, [this](FolderNode *node) {
        checkStatus();
        updateProjectFolder(node);
    });
    connect(ProjectTree::instance(),
            &ProjectTree::currentProjectChanged,
            this,
            [&](ProjectExplorer::Project *project) {
                if (project != nullptr) {
                    m_currentProject = project->projectDirectory();
                } else {
                    m_currentProject.clear();
                }
            });
    connect(qApp, &QApplication::applicationStateChanged, this, [=](Qt::ApplicationState state) {
        if (state == Qt::ApplicationState::ApplicationActive) {
            checkStatus();
        }
    });
    connect(SessionManager::instance(),
            &SessionManager::projectAdded,
            this,
            [&](ProjectExplorer::Project *project) {
                auto projectPath = project->rootProjectDirectory();
                auto projectIter = m_projectStatusCache.find(projectPath);
                if (projectIter == m_projectStatusCache.end()) {
                    auto [iter, _] = m_projectStatusCache.emplace(projectPath, projectPath);
                    connect(&iter->second.gitRepo, &LibGit2Cpp::GitRepo::fileDiffReceived, this, &ProjectVcsStatus::handleChangedFileDiff);
                    connect(&iter->second.gitRepo, &LibGit2Cpp::GitRepo::fileChangeSetReceived, this, &ProjectVcsStatus::handleReceivedFileChangeSet);
                }
                qDebug() << "Register project" << projectPath;
            });
    connect(SessionManager::instance(),
            &SessionManager::projectRemoved,
            this,
            [&](ProjectExplorer::Project *project) {
                auto projectPath = project->rootProjectDirectory();
                auto iter = m_projectStatusCache.find(projectPath);
                if (iter != m_projectStatusCache.end()) {
                    qDebug() << "Deregister project" << projectPath;
                    disconnect(&iter->second.gitRepo, &LibGit2Cpp::GitRepo::fileDiffReceived, this, &ProjectVcsStatus::handleChangedFileDiff);
                    disconnect(&iter->second.gitRepo, &LibGit2Cpp::GitRepo::fileChangeSetReceived, this, &ProjectVcsStatus::handleReceivedFileChangeSet);
                    m_projectStatusCache.erase(iter);
                }
            });
    connect(Core::EditorManager::instance(),
            &Core::EditorManager::currentEditorChanged,
            this,
            &ProjectVcsStatus::setCurrentDocument);
    connect(Core::EditorManager::instance(),
            &Core::EditorManager::documentClosed,
            this,
            &ProjectVcsStatus::handleClosedDocument);
}

ProjectVcsStatus::~ProjectVcsStatus() {
    LibGit2Cpp::shutdown();
}

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
    if (m_currentProject.isEmpty() == false) {
        auto projectIter = m_projectStatusCache.find(m_currentProject);
        if (projectIter != m_projectStatusCache.end()) {
            projectIter->second.gitRepo.requestFileChangeSet();
        }
        if (m_currentDocument != nullptr
            && projectIter != m_projectStatusCache.end()) {
            projectIter->second.gitRepo.requestFileDiff(m_currentDocument->filePath());
        }
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
    auto &changeSet = cacheIter->second;
    changeSet.isFolderNodesCurrent = false;
}

LibGit2Cpp::VcsChangeType ProjectVcsStatus::vcsStatusChanges(ProjectExplorer::Node *node) const
{
    if (node == nullptr) {
        return LibGit2Cpp::VcsChangeType::Unchanged;
    }
    auto *project = node->getProject();
    if (project == nullptr) {
        return LibGit2Cpp::VcsChangeType::Unchanged;
    }
    auto projectPath = project->projectDirectory();
    auto cacheIter = m_projectStatusCache.find(projectPath);
    if (cacheIter == m_projectStatusCache.end()) {
        return LibGit2Cpp::VcsChangeType::Unchanged;
    }
    auto &changeSet = cacheIter->second;
    if (auto *fileNode = node->asFileNode()) {
        const auto& filePath = fileNode->filePath();
        auto fileChangeSetIter = changeSet.fileChangeSet.constFind(filePath);
        if (fileChangeSetIter != changeSet.fileChangeSet.cend()) {
            return fileChangeSetIter.value();
        }
    }
    else if (auto *folderNode = node->asFolderNode()) {
        if (!changeSet.isFolderNodesCurrent) {
            changeSet.folderNodes = extractFolderNodes(changeSet.fileChangeSet, project);
            changeSet.isFolderNodesCurrent = true;
        }
        auto iter = changeSet.folderNodes.find(folderNode);
        if (iter != changeSet.folderNodes.end()) {
            return LibGit2Cpp::VcsChangeType::FolderContainsChanges;
        }
    }
    return LibGit2Cpp::VcsChangeType::Unchanged;
}

const ProjectVcsStatus::FileDiff* ProjectVcsStatus::currentVcsFileDiff() const
{
    if (m_currentDocument != nullptr) {
        auto diffCacheIter = m_fileDiffCache.find(m_currentDocument->filePath());
        if (diffCacheIter != m_fileDiffCache.end()) {
            return diffCacheIter->second.get();
        }
    }
    return nullptr;
}

std::optional<int> ProjectVcsStatus::nextChangedLineNumber(int currentLineNumber)
{
    if (m_currentDocument != nullptr) {
        auto fileDiffIter = m_fileDiffCache.find(m_currentDocument->filePath());
        if (fileDiffIter != m_fileDiffCache.end()) {
            const auto & lines = fileDiffIter->second->fileChangeSet;
            for (auto iter = lines.cbegin(), end = lines.cend(); iter != end; ++iter) {
                if (iter->first > currentLineNumber) {
                    return iter->first;
                }
            }
        }
    }
    return std::nullopt;
}

std::optional<int> ProjectVcsStatus::previousChangedLineNumber(int currentLineNumber)
{
    if (m_currentDocument != nullptr) {
        auto fileDiffIter = m_fileDiffCache.find(m_currentDocument->filePath());
        if (fileDiffIter != m_fileDiffCache.end()) {
            const auto & lines = fileDiffIter->second->fileChangeSet;
            for (auto iter = lines.crbegin(), end = lines.crend(); iter != end; ++iter) {
                if (iter->first < currentLineNumber) {
                    return iter->first;
                }
            }
        }
    }
    return std::nullopt;
}

void ProjectVcsStatus::setCurrentDocument(Core::IEditor *editor)
{
    auto findProjectStatus = [this] (const Utils::FilePath & filePath) -> ProjectStatusMap::iterator {
        return std::find_if(m_projectStatusCache.begin(),
                            m_projectStatusCache.end(),
                            [=](const ProjectStatusMap::value_type &item) -> bool {
                                return filePath.isChildOf(item.first);
                            });
    };
    if (editor != nullptr && editor->document() != nullptr) {
        auto repoIterator = findProjectStatus(editor->document()->filePath());
        if (repoIterator == m_projectStatusCache.end()) {
            return;
        }
        m_currentDocument = editor->document();
        connect(m_currentDocument, &QObject::destroyed, this, [=]() {
            m_currentDocument = nullptr;
        });

        if (repoIterator->second.fileChangeSet.contains(m_currentDocument->filePath())) {
            repoIterator->second.gitRepo.requestFileDiff(m_currentDocument->filePath());
        }
    }
}

void ProjectVcsStatus::handleClosedDocument(Core::IDocument *document)
{
    if (document == nullptr) {
        return;
    }
    auto fileDiffIter = m_fileDiffCache.find(document->filePath());
    if (fileDiffIter != m_fileDiffCache.end()) {
        m_fileDiffCache.erase(fileDiffIter);
    }
}

void ProjectVcsStatus::handleChangedFileDiff(Utils::FilePath filePath, LibGit2Cpp::LineNumberClassMap fileDiff)
{
    auto fileDiffIter = m_fileDiffCache.find(filePath);
    if (fileDiffIter == m_fileDiffCache.end()) {
        m_fileDiffCache.emplace(filePath, new FileDiff{fileDiff});
    }
    else {
        if (fileDiffIter->second->fileChangeSet == fileDiff) {
            return;
        }
        fileDiffIter->second->fileChangeSet = fileDiff;
    }
    if (m_currentDocument != nullptr && filePath == m_currentDocument->filePath()) {
        emit currentFileDiffChanged();
    }
}

void ProjectVcsStatus::handleReceivedFileChangeSet(Utils::FilePath repositoryPath, LibGit2Cpp::VcsChangeSet fileChangeSet)
{
    auto projectIter = m_projectStatusCache.find(repositoryPath);
    if (projectIter != m_projectStatusCache.end()
        && projectIter->second.fileChangeSet != fileChangeSet) {
        projectIter->second.fileChangeSet = fileChangeSet;
        projectIter->second.isFolderNodesCurrent = false;
        emit vcsStatusChanged();
    }
}

} // namespace ProjectExplorer
