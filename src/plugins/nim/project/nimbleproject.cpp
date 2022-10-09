// Copyright (C) Filippo Cucchetto <filippocucchetto@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "nimbleproject.h"
#include "nimconstants.h"
#include "nimblebuildsystem.h"

#include <coreplugin/icontext.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

using namespace Nim;
using namespace ProjectExplorer;

NimbleProject::NimbleProject(const Utils::FilePath &fileName)
    : ProjectExplorer::Project(Constants::C_NIMBLE_MIMETYPE, fileName)
{
    setId(Constants::C_NIMBLEPROJECT_ID);
    setDisplayName(fileName.completeBaseName());
    // ensure debugging is enabled (Nim plugin translates nim code to C code)
    setProjectLanguages(Core::Context(ProjectExplorer::Constants::CXX_LANGUAGE_ID));
    setBuildSystemCreator([] (Target *t) { return new NimbleBuildSystem(t); });
}

QVariantMap NimbleProject::toMap() const
{
    QVariantMap result = Project::toMap();
    result[Constants::C_NIMPROJECT_EXCLUDEDFILES] = m_excludedFiles;
    return result;
}

Project::RestoreResult NimbleProject::fromMap(const QVariantMap &map, QString *errorMessage)
{
    auto result = Project::fromMap(map, errorMessage);
    m_excludedFiles = map.value(Constants::C_NIMPROJECT_EXCLUDEDFILES).toStringList();
    return result;
}

QStringList NimbleProject::excludedFiles() const
{
    return m_excludedFiles;
}

void NimbleProject::setExcludedFiles(const QStringList &excludedFiles)
{
    m_excludedFiles = excludedFiles;
}



#include "moc_nimbleproject.cpp"
