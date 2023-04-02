// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "clangtidyclazyrunner.h"

#include "clangtoolssettings.h"
#include "clangtoolsutils.h"

#include <coreplugin/icore.h>

#include <cppeditor/clangdiagnosticconfigsmodel.h>
#include <cppeditor/compileroptionsbuilder.h>
#include <cppeditor/cpptoolsreuse.h>

#include <utils/qtcassert.h>
#include <utils/qtcprocess.h>

#include <QDebug>
#include <QDir>
#include <QLoggingCategory>

static Q_LOGGING_CATEGORY(LOG, "qtc.clangtools.runner", QtWarningMsg)

using namespace CppEditor;

namespace ClangTools {
namespace Internal {

static bool isClMode(const QStringList &options)
{
    return options.contains("--driver-mode=cl");
}

static QStringList tidyChecksArguments(const ClangDiagnosticConfig diagnosticConfig)
{
    const ClangDiagnosticConfig::TidyMode tidyMode = diagnosticConfig.clangTidyMode();
    // The argument "-config={}" stops stating/evaluating the .clang-tidy file.
    if (tidyMode == ClangDiagnosticConfig::TidyMode::UseDefaultChecks)
        return {"-config={}", "-checks=-clang-diagnostic-*"};
    if (tidyMode == ClangDiagnosticConfig::TidyMode::UseCustomChecks)
        return {"-config=" + diagnosticConfig.clangTidyChecksAsJson()};
    return {"--warnings-as-errors=-*", "-checks=-clang-diagnostic-*"};
}

static QStringList clangHeaderArguments(const QString file) {
    if (file.endsWith(".h")) {
        return {"-Wno-pragma-once-outside-header"};
    }
    else return {};
}

static QStringList clazyChecksArguments(const ClangDiagnosticConfig diagnosticConfig)
{
    const QString clazyChecks = diagnosticConfig.clazyChecks();
    if (!clazyChecks.isEmpty())
        return {"-checks=" + diagnosticConfig.clazyChecks()};
    return {};
}

static QStringList clangArguments(const ClangDiagnosticConfig &diagnosticConfig,
                                  const QStringList &baseOptions)
{
    QStringList arguments;
    arguments << ClangDiagnosticConfigsModel::globalDiagnosticOptions()
              << (isClMode(baseOptions) ? CppEditor::clangArgsForCl(diagnosticConfig.clangOptions())
                                        : diagnosticConfig.clangOptions())
              << baseOptions;

    if (LOG().isDebugEnabled())
        arguments << QLatin1String("-v");

    return arguments;
}

ClangTidyRunner::ClangTidyRunner(const ClangDiagnosticConfig &config, QObject *parent)
    : ClangToolRunner(parent)
{
    setName(tr("Clang-Tidy"));
    setOutputFileFormat(OutputFileFormat::Yaml);
    setExecutable(clangTidyExecutable());
    setArgsCreator([this, config](const QStringList &baseOptions) {
        return QStringList() << tidyChecksArguments(config)
                             << mainToolArguments()
                             << "--"
                             << clangHeaderArguments(fileToAnalyze())
                             << clangArguments(config, baseOptions);
    });
}

ClazyStandaloneRunner::ClazyStandaloneRunner(const ClangDiagnosticConfig &config, QObject *parent)
    : ClangToolRunner(parent)
{
    setName(tr("Clazy"));
    setOutputFileFormat(OutputFileFormat::Yaml);
    setExecutable(clazyStandaloneExecutable());
    setArgsCreator([this, config](const QStringList &baseOptions) {
        return QStringList() << clazyChecksArguments(config)
                             << mainToolArguments()
                             << "--"
                             << clangHeaderArguments(fileToAnalyze())
                             << clangArguments(config, baseOptions);
    });
}

} // namespace Internal
} // namespace ClangTools
