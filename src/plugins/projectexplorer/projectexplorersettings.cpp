// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "projectexplorersettings.h"

#include "projectexplorer.h"
#include "projectexplorerconstants.h"
#include "projectexplorersettings.h"
#include "projectexplorertr.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/documentmanager.h>

#include <utils/environmentdialog.h>
#include <utils/layoutbuilder.h>
#include <utils/hostosinfo.h>
#include <utils/pathchooser.h>

#include <QAudioOutput>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QLabel>
#include <QMediaPlayer>
#include <QPushButton>
#include <QRadioButton>
#include <QRandomGenerator>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QVideoSink>
#include <QVideoWidget>

using namespace Core;
using namespace Utils;

namespace ProjectExplorer {

class VideoDialog final : public QDialog {
public:
    VideoDialog(QMediaPlayer* player, QVideoWidget* video) : QDialog() {
        setLayout(new QVBoxLayout);
        layout()->setContentsMargins(0, 0, 0, 0);
        layout()->addWidget(video);
        grabMouse();
        grabKeyboard();
        QObject::connect(
            player, &QMediaPlayer::playingChanged, this,
            [this](bool isPlaying) {
                if (isPlaying) return;
                if (isVisible()) close();
                else deleteLater();
            },
            Qt::QueuedConnection);
        QObject::connect(this, &QDialog::finished, this, [this, player]() {
            if (player->isPlaying()) player->stop();
            else deleteLater();
        });
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        QDialog::mousePressEvent(event);
        close();
    }
    void keyPressEvent(QKeyEvent* event) override {
        QDialog::keyPressEvent(event);
        close();
    }
};

static auto resolveSourcePath(QString sourcePath) -> QUrl {
    if (sourcePath.contains(';')) {
        auto list = sourcePath.split(';');
        auto index = QRandomGenerator::global()->bounded(0, list.size());
        return QUrl::fromLocalFile(list[index]);
    }
    return QUrl::fromLocalFile(sourcePath);
}

void ProjectExplorerSettings::playAlertMedia(QString sourcePath)
{
    static QMediaPlayer* player = []() {
        qWarning("Creating Player with AudioOutput...");
        auto player = new QMediaPlayer();
        player->setAudioOutput(new QAudioOutput{player});
        return player;
    }();
    player->stop();
    player->setPosition(0);
    player->setSource(resolveSourcePath(sourcePath));
    if (player->hasVideo()) {
        auto video = new QVideoWidget();
        player->setVideoOutput(video);
        QObject::connect(
            player->videoSink(), &QVideoSink::videoFrameChanged, video,
            [video]() {
                auto dialog = new VideoDialog(player, video);
                dialog->open();
            },
            Qt::SingleShotConnection);
        player->play();
    }
    else if (player->hasAudio()) {
        player->play();
    }
    else {
        qWarning("Nothing to play!");
    }
}

} // namespace ProjectExplorer

namespace ProjectExplorer::Internal {

enum { UseCurrentDirectory, UseProjectDirectory };

class ProjectExplorerSettingsWidget : public IOptionsPageWidget
{
public:
    ProjectExplorerSettingsWidget();

    ProjectExplorerSettings settings() const;
    void setSettings(const ProjectExplorerSettings  &s);

    FilePath projectsDirectory() const;
    void setProjectsDirectory(const FilePath &pd);

    bool useProjectsDirectory();
    void setUseProjectsDirectory(bool v);

    void apply() final
    {
        ProjectExplorerPlugin::setProjectExplorerSettings(settings());
        DocumentManager::setProjectsDirectory(projectsDirectory());
        DocumentManager::setUseProjectsDirectory(useProjectsDirectory());
    }

private:
    void slotDirectoryButtonGroupChanged();
    void updateAppEnvChangesLabel();

    mutable ProjectExplorerSettings m_settings;
    Utils::EnvironmentItems m_appEnvChanges;
    QRadioButton *m_currentDirectoryRadioButton;
    QRadioButton *m_directoryRadioButton;
    PathChooser *m_projectsDirectoryPathChooser;
    QCheckBox *m_closeSourceFilesCheckBox;
    QCheckBox *m_saveAllFilesCheckBox;
    QCheckBox *m_deployProjectBeforeRunCheckBox;
    QCheckBox *m_addLibraryPathsToRunEnvCheckBox;
    QCheckBox *m_promptToStopRunControlCheckBox;
    QCheckBox *m_automaticallyCreateRunConfiguration;
    QCheckBox *m_clearIssuesCheckBox;
    QCheckBox *m_abortBuildAllOnErrorCheckBox;
    QCheckBox *m_lowBuildPriorityCheckBox;
    QCheckBox *m_warnAgainstNonAsciiBuildDirCheckBox;
    QComboBox *m_buildBeforeDeployComboBox;
    QComboBox *m_stopBeforeBuildComboBox;
    QComboBox *m_terminalModeComboBox;
    QCheckBox *m_jomCheckbox;
    Utils::ElidingLabel *m_appEnvLabel;

    QSpinBox *m_longBuildThresholdSpinBox = new QSpinBox;
    Utils::PathChooser *m_longBuildSuccessMediaPath = new Utils::PathChooser;
    Utils::PathChooser *m_longBuildFailedMediaPath = new Utils::PathChooser;

    QButtonGroup *m_directoryButtonGroup;
};

ProjectExplorerSettingsWidget::ProjectExplorerSettingsWidget()
{
    m_currentDirectoryRadioButton = new QRadioButton(Tr::tr("Current directory"));
    m_directoryRadioButton = new QRadioButton(Tr::tr("Directory"));
    m_projectsDirectoryPathChooser = new PathChooser;
    m_closeSourceFilesCheckBox = new QCheckBox(Tr::tr("Close source files along with project"));
    m_saveAllFilesCheckBox = new QCheckBox(Tr::tr("Save all files before build"));
    m_deployProjectBeforeRunCheckBox = new QCheckBox(Tr::tr("Always deploy project before running it"));
    m_addLibraryPathsToRunEnvCheckBox =
            new QCheckBox(Tr::tr("Add linker library search paths to run environment"));
    m_promptToStopRunControlCheckBox = new QCheckBox(Tr::tr("Always ask before stopping applications"));
    m_automaticallyCreateRunConfiguration =
            new QCheckBox(Tr::tr("Create suitable run configurations automatically"));
    m_clearIssuesCheckBox = new QCheckBox(Tr::tr("Clear issues list on new build"));
    m_abortBuildAllOnErrorCheckBox = new QCheckBox(Tr::tr("Abort on error when building all projects"));
    m_lowBuildPriorityCheckBox = new QCheckBox(Tr::tr("Start build processes with low priority"));
    m_warnAgainstNonAsciiBuildDirCheckBox = new QCheckBox(
        Tr::tr("Warn against build directories with spaces or non-ASCII characters"));
    m_warnAgainstNonAsciiBuildDirCheckBox->setToolTip(
        Tr::tr("Some legacy build tools do not deal well with paths that contain \"special\" "
               "characters such as spaces, potentially resulting in spurious build errors.<p>"
               "Uncheck this option if you do not work with such tools."));
    m_buildBeforeDeployComboBox = new QComboBox;
    m_buildBeforeDeployComboBox->addItem(Tr::tr("Do Not Build Anything"),
                                         int(BuildBeforeRunMode::Off));
    m_buildBeforeDeployComboBox->addItem(Tr::tr("Build the Whole Project"),
                                         int(BuildBeforeRunMode::WholeProject));
    m_buildBeforeDeployComboBox->addItem(Tr::tr("Build Only the Application to Be Run"),
                                         int(BuildBeforeRunMode::AppOnly));
    const QSizePolicy cbSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    m_buildBeforeDeployComboBox->setSizePolicy(cbSizePolicy);
    m_stopBeforeBuildComboBox = new QComboBox;
    m_stopBeforeBuildComboBox->addItem(Tr::tr("None"), int(StopBeforeBuild::None));
    m_stopBeforeBuildComboBox->addItem(Tr::tr("All"), int(StopBeforeBuild::All));
    m_stopBeforeBuildComboBox->addItem(Tr::tr("Same Project"), int(StopBeforeBuild::SameProject));
    m_stopBeforeBuildComboBox->addItem(Tr::tr("Same Build Directory"),
                                       int(StopBeforeBuild::SameBuildDir));
    m_stopBeforeBuildComboBox->addItem(Tr::tr("Same Application"),
                                       int(StopBeforeBuild::SameApp));
    m_stopBeforeBuildComboBox->setSizePolicy(cbSizePolicy);
    m_terminalModeComboBox = new QComboBox;
    m_terminalModeComboBox->addItem(Tr::tr("Enabled"));
    m_terminalModeComboBox->addItem(Tr::tr("Disabled"));
    m_terminalModeComboBox->addItem(Tr::tr("Deduced from Project"));
    m_terminalModeComboBox->setSizePolicy(cbSizePolicy);
    m_jomCheckbox = new QCheckBox(Tr::tr("Use jom instead of nmake"));
    auto jomLabel = new QLabel("<i>jom</i> is a drop-in replacement for <i>nmake</i> which "
                               "distributes the compilation process to multiple CPU cores. "
                               "The latest binary is available at "
                               "<a href=\"http://download.qt.io/official_releases/jom/\">"
                               "http://download.qt.io/official_releases/jom/</a>. "
                               "Disable it if you experience problems with your builds.");
    jomLabel->setWordWrap(true);

    m_longBuildThresholdSpinBox->setSuffix("secs");
    m_longBuildThresholdSpinBox->setMinimum(0);
    m_longBuildSuccessMediaPath->setToolTip(Tr::tr("Media File played if long build succeeds (empty to disable)"));
    m_longBuildSuccessMediaPath->setExpectedKind(PathChooser::File);
    m_longBuildSuccessMediaPath->setHistoryCompleter("General.Media.History");
    auto playSuccessMediaButton = new QPushButton(Tr::tr("Play"));
    connect(playSuccessMediaButton, &QPushButton::clicked, [&]() {
        ProjectExplorerSettings::playAlertMedia(m_longBuildSuccessMediaPath->filePath().toString());
    });
    m_longBuildFailedMediaPath->setToolTip(Tr::tr("Media File played if long build fails (empty to disable)"));
    m_longBuildFailedMediaPath->setExpectedKind(PathChooser::File);
    m_longBuildFailedMediaPath->setHistoryCompleter("General.Media.History");
    auto playFailedMediaButton = new QPushButton(Tr::tr("Play"));
    connect(playFailedMediaButton, &QPushButton::clicked, [&]() {
        ProjectExplorerSettings::playAlertMedia(m_longBuildFailedMediaPath->filePath().toString());
    });

    const QString appEnvToolTip = Tr::tr("Environment changes to apply to run configurations, "
                                         "but not build configurations.");
    const auto appEnvDescriptionLabel = new QLabel(Tr::tr("Application environment:"));
    appEnvDescriptionLabel->setToolTip(appEnvToolTip);
    m_appEnvLabel = new Utils::ElidingLabel;
    m_appEnvLabel->setElideMode(Qt::ElideRight);
    m_appEnvLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    const auto appEnvButton = new QPushButton(Tr::tr("Change..."));
    appEnvButton->setSizePolicy(QSizePolicy::Fixed, appEnvButton->sizePolicy().verticalPolicy());
    appEnvButton->setToolTip(appEnvToolTip);
    connect(appEnvButton, &QPushButton::clicked, this, [appEnvButton, this] {
        std::optional<EnvironmentItems> changes
            = EnvironmentDialog::getEnvironmentItems(appEnvButton, m_appEnvChanges);
        if (!changes)
            return;
        m_appEnvChanges = *changes;
        updateAppEnvChangesLabel();
    });

    using namespace Layouting;
    Column {
        Group {
            title(Tr::tr("Projects Directory")),
            Column {
                m_currentDirectoryRadioButton,
                Row { m_directoryRadioButton, m_projectsDirectoryPathChooser },
            },
        },
        Group {
            title(Tr::tr("Closing Projects")),
            Column {
                m_closeSourceFilesCheckBox,
            },
        },
        Group {
            title(Tr::tr("Build and Run")),
            Column {
                m_saveAllFilesCheckBox,
                m_deployProjectBeforeRunCheckBox,
                m_addLibraryPathsToRunEnvCheckBox,
                m_promptToStopRunControlCheckBox,
                m_automaticallyCreateRunConfiguration,
                m_clearIssuesCheckBox,
                m_abortBuildAllOnErrorCheckBox,
                m_lowBuildPriorityCheckBox,
                m_warnAgainstNonAsciiBuildDirCheckBox,
                Form {
                    appEnvDescriptionLabel, Row{m_appEnvLabel, appEnvButton, st}, br,
                    Tr::tr("Build before deploying:"), m_buildBeforeDeployComboBox, br,
                    Tr::tr("Stop applications before building:"), m_stopBeforeBuildComboBox, br,
                    Tr::tr("Default for \"Run in terminal\":"), m_terminalModeComboBox, br,
                },
                m_jomCheckbox,
                jomLabel,
            },
        },
        Group {
            title(Tr::tr("Long Build Announcements")),
            Form {
                Tr::tr("Threshold:"), m_longBuildThresholdSpinBox, br,
                Tr::tr("Success Media File:"), m_longBuildSuccessMediaPath, playSuccessMediaButton, br,
                Tr::tr("Failed Media File:"), m_longBuildFailedMediaPath, playFailedMediaButton, br,
            },
        },
        st,
    }.attachTo(this);

    m_jomCheckbox->setVisible(HostOsInfo::isWindowsHost());
    jomLabel->setVisible(HostOsInfo::isWindowsHost());

    m_directoryButtonGroup = new QButtonGroup;
    m_directoryButtonGroup->setExclusive(true);
    m_directoryButtonGroup->addButton(m_currentDirectoryRadioButton, UseCurrentDirectory);
    m_directoryButtonGroup->addButton(m_directoryRadioButton, UseProjectDirectory);

    connect(m_directoryButtonGroup, &QButtonGroup::buttonClicked,
            this, &ProjectExplorerSettingsWidget::slotDirectoryButtonGroupChanged);

    setSettings(ProjectExplorerPlugin::projectExplorerSettings());
    setProjectsDirectory(DocumentManager::projectsDirectory());
    setUseProjectsDirectory(DocumentManager::useProjectsDirectory());
    updateAppEnvChangesLabel();
}

ProjectExplorerSettings ProjectExplorerSettingsWidget::settings() const
{
    m_settings.buildBeforeDeploy = static_cast<BuildBeforeRunMode>(
                m_buildBeforeDeployComboBox->currentData().toInt());
    m_settings.deployBeforeRun = m_deployProjectBeforeRunCheckBox->isChecked();
    m_settings.saveBeforeBuild = m_saveAllFilesCheckBox->isChecked();
    m_settings.useJom = m_jomCheckbox->isChecked();
    m_settings.addLibraryPathsToRunEnv = m_addLibraryPathsToRunEnvCheckBox->isChecked();
    m_settings.prompToStopRunControl = m_promptToStopRunControlCheckBox->isChecked();
    m_settings.automaticallyCreateRunConfigurations = m_automaticallyCreateRunConfiguration->isChecked();
    m_settings.stopBeforeBuild = static_cast<StopBeforeBuild>(
                m_stopBeforeBuildComboBox->currentData().toInt());
    m_settings.terminalMode = static_cast<ProjectExplorer::TerminalMode>(m_terminalModeComboBox->currentIndex());
    m_settings.closeSourceFilesWithProject = m_closeSourceFilesCheckBox->isChecked();
    m_settings.clearIssuesOnRebuild = m_clearIssuesCheckBox->isChecked();
    m_settings.abortBuildAllOnError = m_abortBuildAllOnErrorCheckBox->isChecked();
    m_settings.lowBuildPriority = m_lowBuildPriorityCheckBox->isChecked();
    m_settings.warnAgainstNonAsciiBuildDir = m_warnAgainstNonAsciiBuildDirCheckBox->isChecked();
    m_settings.appEnvChanges = m_appEnvChanges;
    m_settings.longBuildThreshold = m_longBuildThresholdSpinBox->value();
    m_settings.longBuildSuccessMediaPath = m_longBuildSuccessMediaPath->filePath().toString();
    m_settings.longBuildFailedMediaPath = m_longBuildFailedMediaPath->filePath().toString();
    return m_settings;
}

void ProjectExplorerSettingsWidget::setSettings(const ProjectExplorerSettings  &pes)
{
    m_settings = pes;
    m_appEnvChanges = pes.appEnvChanges;
    m_buildBeforeDeployComboBox->setCurrentIndex(
                m_buildBeforeDeployComboBox->findData(int(m_settings.buildBeforeDeploy)));
    m_deployProjectBeforeRunCheckBox->setChecked(m_settings.deployBeforeRun);
    m_saveAllFilesCheckBox->setChecked(m_settings.saveBeforeBuild);
    m_jomCheckbox->setChecked(m_settings.useJom);
    m_addLibraryPathsToRunEnvCheckBox->setChecked(m_settings.addLibraryPathsToRunEnv);
    m_promptToStopRunControlCheckBox->setChecked(m_settings.prompToStopRunControl);
    m_automaticallyCreateRunConfiguration->setChecked(m_settings.automaticallyCreateRunConfigurations);
    m_stopBeforeBuildComboBox->setCurrentIndex(
                m_stopBeforeBuildComboBox->findData(int(m_settings.stopBeforeBuild)));
    m_terminalModeComboBox->setCurrentIndex(static_cast<int>(m_settings.terminalMode));
    m_closeSourceFilesCheckBox->setChecked(m_settings.closeSourceFilesWithProject);
    m_clearIssuesCheckBox->setChecked(m_settings.clearIssuesOnRebuild);
    m_abortBuildAllOnErrorCheckBox->setChecked(m_settings.abortBuildAllOnError);
    m_lowBuildPriorityCheckBox->setChecked(m_settings.lowBuildPriority);
    m_warnAgainstNonAsciiBuildDirCheckBox->setChecked(m_settings.warnAgainstNonAsciiBuildDir);

    m_longBuildThresholdSpinBox->setValue(m_settings.longBuildThreshold);
    m_longBuildSuccessMediaPath->setPath(m_settings.longBuildSuccessMediaPath);
    m_longBuildFailedMediaPath->setPath(m_settings.longBuildFailedMediaPath);
}

FilePath ProjectExplorerSettingsWidget::projectsDirectory() const
{
    return m_projectsDirectoryPathChooser->filePath();
}

void ProjectExplorerSettingsWidget::setProjectsDirectory(const FilePath &pd)
{
    m_projectsDirectoryPathChooser->setFilePath(pd);
}

bool ProjectExplorerSettingsWidget::useProjectsDirectory()
{
    return m_directoryButtonGroup->checkedId() == UseProjectDirectory;
}

void ProjectExplorerSettingsWidget::setUseProjectsDirectory(bool b)
{
    if (useProjectsDirectory() != b) {
        (b ? m_directoryRadioButton : m_currentDirectoryRadioButton)->setChecked(true);
        slotDirectoryButtonGroupChanged();
    }
}

void ProjectExplorerSettingsWidget::slotDirectoryButtonGroupChanged()
{
    bool enable = useProjectsDirectory();
    m_projectsDirectoryPathChooser->setEnabled(enable);
}

void ProjectExplorerSettingsWidget::updateAppEnvChangesLabel()
{
    const QString shortSummary = EnvironmentItem::toStringList(m_appEnvChanges).join("; ");
    m_appEnvLabel->setText(shortSummary.isEmpty() ? Tr::tr("No changes to apply.")
                                                  : shortSummary);
}

// ProjectExplorerSettingsPage

ProjectExplorerSettingsPage::ProjectExplorerSettingsPage()
{
    setId(Constants::BUILD_AND_RUN_SETTINGS_PAGE_ID);
    setDisplayName(Tr::tr("General"));
    setCategory(Constants::BUILD_AND_RUN_SETTINGS_CATEGORY);
    setDisplayCategory(Tr::tr("Build & Run"));
    setCategoryIconPath(":/projectexplorer/images/settingscategory_buildrun.png");
    setWidgetCreator([] { return new ProjectExplorerSettingsWidget; });
}

} // ProjectExplorer::Internal
