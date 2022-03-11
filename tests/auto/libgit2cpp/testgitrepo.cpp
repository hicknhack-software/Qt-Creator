#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QtTest/QtTest>

#include <QThread>

#include <libgit2cpp/gitrepo.h>

class GitRepoTester : public QObject
{
    Q_OBJECT

signals:
    void finished();

private slots:

    void initTestCase()
    {

        LibGit2Cpp::init();
        createRepository();
    }
    void cleanupTestCase()
    {
        destroyRepository();
        LibGit2Cpp::shutdown();
    }
    void testUpdateFile()
    {
        QTestEventLoop loop;
        connect(this, &GitRepoTester::finished, &loop, &QTestEventLoop::exitLoop);

        auto repoPath = Utils::FilePath::fromString(m_repoPath);
        auto untrackedFile = repoPath.pathAppended(m_untrackedFileName);
        auto modifiedFile = repoPath.pathAppended(m_modifedFileName);
        QVERIFY(repoPath.exists());
        auto repo = LibGit2Cpp::GitRepo(repoPath);

        QSignalSpy spy(&repo, &LibGit2Cpp::GitRepo::fileChangeSetReceived);
        connect(&repo, &LibGit2Cpp::GitRepo::fileChangeSetReceived, this, [&] (Utils::FilePath projectPath, LibGit2Cpp::VcsChangeSet changeSet) {
            QCOMPARE(projectPath, repoPath);
            QCOMPARE(changeSet.count(), 2);
            QVERIFY(changeSet.contains(modifiedFile));
            QCOMPARE(changeSet.value(modifiedFile), LibGit2Cpp::VcsChangeType::FileChanged);
            QVERIFY(changeSet.contains(untrackedFile));
            QCOMPARE(changeSet.value(untrackedFile), LibGit2Cpp::VcsChangeType::FileUntracked);
            emit finished();
        });

        repo.requestFileChangeSet();
        loop.enterLoopMSecs(3000);
        QCOMPARE(spy.count(), 1);
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
