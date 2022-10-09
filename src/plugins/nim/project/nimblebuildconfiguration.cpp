// Copyright (C) Filippo Cucchetto <filippocucchetto@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "nimblebuildconfiguration.h"

#include "nimconstants.h"
//#include "nimblebuildstep.h"
#include "nimbleproject.h"
#include "nimblebuildsystem.h"

#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildstep.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <utils/fileutils.h>
#include <utils/osspecificaspects.h>

#include <QFileInfo>
#include <QDir>

using namespace Nim;
using namespace ProjectExplorer;
using namespace Utils;

NimbleBuildConfiguration::NimbleBuildConfiguration(Target *target, Utils::Id id)
    : BuildConfiguration(target, id)
{
    setConfigWidgetDisplayName(tr("General"));
    setConfigWidgetHasFrame(true);
    setBuildDirectorySettingsKey("Nim.NimbleBuildConfiguration.BuildDirectory");
    appendInitialBuildStep(Constants::C_NIMBLEBUILDSTEP_ID);

    setInitializer([this](const BuildInfo &info) {
        setBuildType(info.buildType);
        setBuildDirectory(project()->projectDirectory());
    });
}

BuildConfiguration::BuildType NimbleBuildConfiguration::buildType() const
{
    return m_buildType;
}

bool NimbleBuildConfiguration::fromMap(const QVariantMap &map)
{
    m_buildType = static_cast<BuildType>(map[Constants::C_NIMBLEBUILDCONFIGURATION_BUILDTYPE].toInt());
    return BuildConfiguration::fromMap(map);
}

QVariantMap NimbleBuildConfiguration::toMap() const
{
    auto map = BuildConfiguration::toMap();
    map[Constants::C_NIMBLEBUILDCONFIGURATION_BUILDTYPE] = buildType();
    return map;
}

void NimbleBuildConfiguration::setBuildType(BuildConfiguration::BuildType buildType)
{
    if (buildType == m_buildType)
        return;
    m_buildType = buildType;
    emit buildTypeChanged();
}

NimbleBuildConfigurationFactory::NimbleBuildConfigurationFactory()
{
    registerBuildConfiguration<NimbleBuildConfiguration>(Constants::C_NIMBLEBUILDCONFIGURATION_ID);
    setSupportedProjectType(Constants::C_NIMBLEPROJECT_ID);
    setSupportedProjectMimeTypeName(Constants::C_NIMBLE_MIMETYPE);

    setBuildGenerator([](const Kit *, const FilePath &projectPath, bool forSetup) {
        const auto oneBuild = [&](BuildConfiguration::BuildType buildType, const QString &typeName) {
            BuildInfo info;
            info.buildType = buildType;
            info.typeName = typeName;
            if (forSetup) {
                info.displayName = info.typeName;
                info.buildDirectory = projectPath.parentDir();
            }
            return info;
        };
        return QList<BuildInfo>{
            oneBuild(BuildConfiguration::Debug, BuildConfiguration::tr("Debug")),
            oneBuild(BuildConfiguration::Release, BuildConfiguration::tr("Release"))
        };
    });
}

#include "moc_nimblebuildconfiguration.cpp"
