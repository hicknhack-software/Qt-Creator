#include "gitrepo.h"

#include <QtConcurrent>

extern "C" {
#include <git2.h>
}

namespace LibGit2Cpp {

namespace {
static bool isLibGit2Initialized = false;
}

void init()
{
    if (isLibGit2Initialized == false) {
        auto status = git_libgit2_init();
        isLibGit2Initialized = status >= 0;
    }
    Q_ASSERT(isLibGit2Initialized);
}

void shutdown()
{
    if (isLibGit2Initialized) {
        auto status = git_libgit2_shutdown();
        Q_ASSERT(status >= 0);
        isLibGit2Initialized = false;
    }
}

namespace {
auto getFileStatusChangeType(unsigned int flag) -> VcsChangeType
{
    if (flag & GIT_STATUS_WT_MODIFIED       //
        || flag & GIT_STATUS_WT_RENAMED     //
        || flag & GIT_STATUS_WT_TYPECHANGE  //
        || flag & GIT_STATUS_INDEX_MODIFIED //
        || flag & GIT_STATUS_INDEX_RENAMED  //
        || flag & GIT_STATUS_INDEX_TYPECHANGE) {
        return VcsChangeType::FileChanged;
    }
    if (flag & GIT_STATUS_WT_NEW        //
        || flag & GIT_STATUS_WT_DELETED //
        || flag & GIT_STATUS_INDEX_NEW  //
        || flag & GIT_STATUS_INDEX_DELETED) {
        return VcsChangeType::FileUntracked;
    }
    return VcsChangeType::Invalid;
}
} // namespace

GitRepo::GitRepo(const Utils::FilePath &path, QObject * parent)
    : QObject{parent}
    , m_path(path)
    , m_repository(nullptr, git_repository_free)
{
    git_repository *repository = nullptr;
    int error = git_repository_open(&repository, m_path.toString().toLatin1().constData());
    if (error != 0) {
        qDebug() << "Error while opening repo occured\n";
        return;
    }
    Q_ASSERT(repository != nullptr);
    m_repository.reset(repository);
}

GitRepo &GitRepo::operator=(GitRepo &&other)
{
    if (&other != this) {
        this->m_path = other.m_path;
        this->m_repository = std::move(other.m_repository);
    }
    return *this;
}

GitRepo::GitRepo(GitRepo &&other)
    : m_path(other.m_path)
    , m_repository(std::move(other.m_repository))
{}

void GitRepo::requestFileDiff(const Utils::FilePath &filePath)
{
    if (m_fileDiffWatcher.isRunning()) {
        return;
    }
    auto future = QtConcurrent::run([this, filePath] () -> void {
        auto pathString = filePath.relativeChildPath(m_path).toString().toStdString();

        int isIgnored = 0;
        int error = git_ignore_path_is_ignored(&isIgnored, m_repository.get(), pathString.data());
        Q_ASSERT(error == 0);

        if (isIgnored == 0) {
            char *path[1];
            path[0] = pathString.data();
            git_diff_options m_gitDiffOptions;
            git_diff_options_init(&m_gitDiffOptions, GIT_DIFF_OPTIONS_VERSION);
            m_gitDiffOptions.context_lines = 0;
            m_gitDiffOptions.pathspec.count = 1;
            m_gitDiffOptions.pathspec.strings = path;

            git_diff *diff = nullptr;
            error = git_diff_index_to_workdir(&diff, m_repository.get(), nullptr, &m_gitDiffOptions);
            Q_ASSERT(error == 0);
            Q_ASSERT(diff != nullptr);

            auto lineNumberDiff = extractLineNumbersOfDiff(diff);
            git_diff_free(diff);

            emit fileDiffReceived(filePath, lineNumberDiff);
        }
    });
    m_fileDiffWatcher.setFuture(future);
}

void GitRepo::requestFileChangeSet()
{
    if (m_fileChangeSetWatcher.isRunning()) {
        return;
    }
    auto future = QtConcurrent::run([this] () -> void {
        struct UpdateContext
        {
            const Utils::FilePath &projectPath;
            VcsChangeSet &fileStatus;
        };
        VcsChangeSet newChangeSet{};
        auto context = UpdateContext{m_path, newChangeSet};

        auto error = git_status_foreach(
            m_repository.get(),
            [](const char *path, unsigned int status_flags, void *payload) -> int {
                if (status_flags & GIT_STATUS_IGNORED) {
                    return 0;
                }
                auto *ctx = static_cast<UpdateContext *>(payload);
                Q_ASSERT(ctx != nullptr);

                const auto type = getFileStatusChangeType(status_flags);
                const auto pathKey = ctx->projectPath.pathAppended(QString::fromUtf8(path));

                ctx->fileStatus.insert(pathKey, type);

                return 0;
            },
            static_cast<void *>(&context));
        Q_ASSERT(error == 0);

        emit fileChangeSetReceived(m_path, newChangeSet);
    });
    m_fileChangeSetWatcher.setFuture(future);
}

LineNumberClassMap GitRepo::extractLineNumbersOfDiff(git_diff *diff) const
{
    Q_ASSERT(diff != nullptr);

    LineNumberClassMap lineNumbers;
    int error = git_diff_foreach(
        diff,
        nullptr,
        nullptr,
        [](const git_diff_delta *delta, const git_diff_hunk *hunk, void *payload) -> int {
            LineNumberClassMap *p = static_cast<LineNumberClassMap *>(payload);
            Q_ASSERT(p != nullptr);

            auto addLines = [&](int start, int lines, auto changeClass) -> void {
                for (int iter = start, end = start + lines; iter < end; ++iter) {
                    auto foundIter = p->find(iter);
                    if (foundIter != p->end()) {
                        (*p)[iter] |= changeClass;
                    } else {
                        p->insert({iter, changeClass});
                    }
                }
            };
            if (delta != nullptr && p != nullptr) {
                addLines(hunk->new_start, hunk->new_lines, ChangeClass::Added);
                addLines(hunk->old_start, hunk->old_lines, ChangeClass::Deleted);
            }
            return 0;
        },
        nullptr,
        static_cast<void *>(&lineNumbers));
    if (error != 0) {
        qDebug() << "Some error occured while traversing over diff.";
    }
    return lineNumbers;
}
} // namespace LibGit2Cpp
