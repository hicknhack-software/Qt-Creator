// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "buildaspects.h"

#include "buildconfiguration.h"
#include "buildpropertiessettings.h"
#include "projectexplorer.h"

#include <coreplugin/fileutils.h>

#include <utils/algorithm.h>
#include <utils/fileutils.h>
#include <utils/infolabel.h>
#include <utils/layoutbuilder.h>
#include <utils/pathchooser.h>

#include <QLayout>

using namespace Utils;

namespace ProjectExplorer {

class BuildDirectoryAspect::Private
{
public:
    FilePath sourceDir;
    FilePath savedShadowBuildDir;
    QString problem;
    QPointer<InfoLabel> problemLabel;
};

BuildDirectoryAspect::BuildDirectoryAspect(const BuildConfiguration *bc) : d(new Private)
{
    setSettingsKey("ProjectExplorer.BuildConfiguration.BuildDirectory");
    setLabelText(tr("Build directory:"));
    setDisplayStyle(PathChooserDisplay);
    setExpectedKind(Utils::PathChooser::Directory);
    setValidationFunction([this](FancyLineEdit *edit, QString *error) {
        const FilePath fixedDir = fixupDir(FilePath::fromUserInput(edit->text()));
        if (!fixedDir.isEmpty())
            edit->setText(fixedDir.toUserOutput());
        return pathChooser() ? pathChooser()->defaultValidationFunction()(edit, error) : true;
    });
    setOpenTerminalHandler([this, bc] {
        Core::FileUtils::openTerminal(FilePath::fromString(value()), bc->environment());
    });
}

BuildDirectoryAspect::~BuildDirectoryAspect()
{
    delete d;
}

void BuildDirectoryAspect::allowInSourceBuilds(const FilePath &sourceDir)
{
    d->sourceDir = sourceDir;
    makeCheckable(CheckBoxPlacement::Top, tr("Shadow build:"), QString());
    setChecked(d->sourceDir != filePath());
}

bool BuildDirectoryAspect::isShadowBuild() const
{
    return !d->sourceDir.isEmpty() && d->sourceDir != filePath();
}

void BuildDirectoryAspect::setProblem(const QString &description)
{
    d->problem = description;
    updateProblemLabel();
}

void BuildDirectoryAspect::toMap(QVariantMap &map) const
{
    StringAspect::toMap(map);
    if (!d->sourceDir.isEmpty()) {
        const FilePath shadowDir = isChecked() ? filePath() : d->savedShadowBuildDir;
        saveToMap(map, shadowDir.toString(), QString(), settingsKey() + ".shadowDir");
    }
}

void BuildDirectoryAspect::fromMap(const QVariantMap &map)
{
    StringAspect::fromMap(map);
    if (!d->sourceDir.isEmpty()) {
        d->savedShadowBuildDir = FilePath::fromString(map.value(settingsKey() + ".shadowDir")
                                                      .toString());
        if (d->savedShadowBuildDir.isEmpty())
            setFilePath(d->sourceDir);
        setChecked(d->sourceDir != filePath());
    }
}

void BuildDirectoryAspect::addToLayout(LayoutBuilder &builder)
{
    StringAspect::addToLayout(builder);
    d->problemLabel = new InfoLabel({}, InfoLabel::Warning);
    d->problemLabel->setElideMode(Qt::ElideNone);
    builder.addRow({{}, d->problemLabel.data()});
    updateProblemLabel();
    if (!d->sourceDir.isEmpty()) {
        connect(this, &StringAspect::checkedChanged, this, [this] {
            if (isChecked()) {
                setFilePath(d->savedShadowBuildDir.isEmpty()
                            ? d->sourceDir : d->savedShadowBuildDir);
            } else {
                d->savedShadowBuildDir = filePath();
                setFilePath(d->sourceDir);
            }
        });
    }
}

FilePath BuildDirectoryAspect::fixupDir(const FilePath &dir)
{
    if (dir.needsDevice())
        return dir;
    if (HostOsInfo::isWindowsHost() && !dir.startsWithDriveLetter())
        return {};
    const QString dirString = dir.toString().toLower();
    const QStringList drives = Utils::transform(QDir::drives(), [](const QFileInfo &fi) {
        return fi.absoluteFilePath().toLower().chopped(1);
    });
    if (!Utils::contains(drives, [&dirString](const QString &drive) {
            return dirString.startsWith(drive);
        }) && !drives.isEmpty()) {
        QString newDir = dir.path();
        newDir.replace(0, 2, drives.first());
        return dir.withNewPath(newDir);
    }
    return {};
}

void BuildDirectoryAspect::updateProblemLabel()
{
    if (!d->problemLabel)
        return;

    d->problemLabel->setText(d->problem);
    d->problemLabel->setVisible(!d->problem.isEmpty());
}

SeparateDebugInfoAspect::SeparateDebugInfoAspect()
{
    setDisplayName(tr("Separate debug info:"));
    setSettingsKey("SeparateDebugInfo");
    setValue(ProjectExplorerPlugin::buildPropertiesSettings().separateDebugInfo.value());
}

} // namespace ProjectExplorer

#include "moc_buildaspects.cpp"
