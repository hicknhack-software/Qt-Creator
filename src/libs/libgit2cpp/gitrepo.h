#pragma once

#include "libgit2cpp_global.h"

#include <utils/filepath.h>

#include <QObject>
#include <QHash>
#include <QMap>
#include <QFutureWatcher>

#include <map>
#include <functional>
#include <memory>

extern "C" {
struct git_repository;
struct git_diff;
}

namespace LibGit2Cpp {
enum class VcsChangeType {
    FileChanged,
    FileUntracked,
    FolderContainsChanges,
    Unchanged,
    Invalid,
};
using VcsChangeSet = QHash<Utils::FilePath, VcsChangeType>;

enum class ChangeClass {
    Invalid = 0,
    Added = 1,
    Deleted = 2,
};

using LineNumberClassMap = std::map<int, ChangeClass>;
using GitFileDiffMap = QHash<Utils::FilePath, LineNumberClassMap>;

inline ChangeClass operator|(ChangeClass a, ChangeClass b)
{
    auto tmp = static_cast<std::underlying_type<ChangeClass>::type>(a)
               | static_cast<std::underlying_type<ChangeClass>::type>(b);
    return static_cast<ChangeClass>(tmp);
}
inline ChangeClass &operator|=(ChangeClass &a, ChangeClass b)
{
    a = a | b;
    return a;
}
inline ChangeClass operator&(ChangeClass a, ChangeClass b)
{
    auto tmp = static_cast<std::underlying_type<ChangeClass>::type>(a)
               & static_cast<std::underlying_type<ChangeClass>::type>(b);
    return static_cast<ChangeClass>(tmp);
}

LIBGIT2CPP_EXPORT void init();
LIBGIT2CPP_EXPORT void shutdown();

class LIBGIT2CPP_EXPORT GitRepo : public QObject
{
    Q_OBJECT

public:
    explicit GitRepo(const Utils::FilePath &path, QObject * parent = nullptr);

    GitRepo(const GitRepo &) = delete;
    GitRepo &operator=(const GitRepo &) = delete;
    GitRepo &operator=(GitRepo &&other);
    GitRepo(GitRepo &&other);

public:
    void requestFileDiff(const Utils::FilePath &filePath);
    void requestFileChangeSet();

signals:
    void fileDiffReceived(Utils::FilePath filePath, LibGit2Cpp::LineNumberClassMap fileDiff);
    void fileChangeSetReceived(Utils::FilePath repositoryPath, LibGit2Cpp::VcsChangeSet fileChangeSet);

private:
    LineNumberClassMap extractLineNumbersOfDiff(git_diff *diff) const;

private:
    using GitRepositoryDeleter = std::function<void(git_repository *repo)>;
    using GitRepositoryPtr = std::unique_ptr<git_repository, GitRepositoryDeleter>;

    Utils::FilePath m_path;
    GitRepositoryPtr m_repository;

    QFutureWatcher<void> m_fileDiffWatcher;
    QFutureWatcher<void> m_fileChangeSetWatcher;
};
} // namespace LibGit2Cpp
