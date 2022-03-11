#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QtTest/QtTest>

#include <gitrepo.h>

extern "C" {
#include <git2.h>
}
using namespace GitVcsChanges::Internal;

class GitRepoTester : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase()
    {
        git_libgit2_init();
        createRepository();
    }
    void cleanupTestCase()
    {
        destroyRepository();
        git_libgit2_shutdown();
    }
    void testUpdateFile()
    {
        auto repoPath = Utils::FilePath::fromString(m_repoPath);
        auto untrackedFile = repoPath.pathAppended(m_untrackedFileName).toString();
        auto modifiedFile = repoPath.pathAppended(m_modifedFileName).toString();
        QVERIFY(repoPath.exists());
        auto repo = GitRepo(repoPath);
        repo.updateFileStatus();
        const auto &filestatus = repo.fileStatus();

        QCOMPARE(filestatus.count(), 2);
        QVERIFY(filestatus.contains(modifiedFile));
        QCOMPARE(filestatus.value(modifiedFile), VcsChangeType::FileChanged);
        QVERIFY(filestatus.contains(untrackedFile));
        QCOMPARE(filestatus.value(untrackedFile), VcsChangeType::FileUntracked);
    }

private:
    void createRepository()
    {
        auto createFile = [this](const QString &filePath) {
            QFile file = QFile{QDir(m_repoPath).filePath(filePath)};
            file.open(QIODevice::NewOnly);
            file.close();
        };

        auto repoExists = QDir::current().mkdir(m_repoName);
        Q_ASSERT(repoExists);
        m_repoPath = QDir::current().filePath(m_repoName);

        auto cwd = QDir::currentPath();
        auto result = QDir::setCurrent(m_repoPath);
        Q_ASSERT(result);

        QProcess::execute("git", {"init"});
        createFile(m_untrackedFileName);
        createFile(m_modifedFileName);

        QProcess::execute("git", {"add", m_modifedFileName});
        QProcess::execute("git", {"commit", "-m", "empty"});

        QFile modifiedFile(m_modifedFileName);
        result = modifiedFile.open(QIODevice::Append | QIODevice::Text);
        Q_ASSERT(result);

        QTextStream out(&modifiedFile);
        out << "Empty\n";

        modifiedFile.close();

        result = QDir::setCurrent(cwd);
        Q_ASSERT(result);
    }
    void destroyRepository()
    {
        if (QFileInfo::exists(m_repoPath)) {
            auto result = QDir(m_repoPath).removeRecursively();
            Q_ASSERT(result);
        }
    }

private:
    const QString m_repoName = "testRepo";
    const char *m_modifedFileName = "modifiedFile.txt";
    const char *m_untrackedFileName = "untrackedFile.txt";
    QString m_repoPath;
};

QTEST_GUILESS_MAIN(GitRepoTester)
#include "testgitrepo.moc"
