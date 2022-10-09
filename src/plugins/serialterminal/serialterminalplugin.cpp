// Copyright (C) 2018 Benjamin Balga
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "serialterminalplugin.h"

#include "serialcontrol.h"

#include <coreplugin/icore.h>

namespace SerialTerminal {
namespace Internal {

bool SerialTerminalPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    m_settings.load(Core::ICore::settings());

    // Create serial output pane
    m_serialOutputPane = std::make_unique<SerialOutputPane>(m_settings);
    connect(m_serialOutputPane.get(), &SerialOutputPane::settingsChanged,
            this, &SerialTerminalPlugin::settingsChanged);

    connect(Core::ICore::instance(), &Core::ICore::saveSettingsRequested,
            this, [this] { m_settings.save(Core::ICore::settings()); });

    return true;
}

ExtensionSystem::IPlugin::ShutdownFlag SerialTerminalPlugin::aboutToShutdown()
{
    m_serialOutputPane->closeTabs(SerialOutputPane::CloseTabNoPrompt);

    return SynchronousShutdown;
}

void SerialTerminalPlugin::settingsChanged(const Settings &settings)
{
    m_settings = settings;
    m_settings.save(Core::ICore::settings());

    m_serialOutputPane->setSettings(m_settings);
}

} // namespace Internal
} // namespace SerialTerminal

#include "moc_serialterminalplugin.cpp"
