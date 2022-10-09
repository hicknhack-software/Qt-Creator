// Copyright (C) 2016 Tim Sander <tim@krieglstein.org>
// Copyright (C) 2016 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "baremetalconstants.h"
#include "baremetaldebugsupport.h"
#include "baremetaldevice.h"
#include "baremetalplugin.h"
#include "baremetalrunconfiguration.h"

#include "debugserverprovidermanager.h"
#include "debugserverproviderssettingspage.h"

#include "iarewtoolchain.h"
#include "keiltoolchain.h"
#include "sdcctoolchain.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icontext.h>
#include <coreplugin/icore.h>

#include <projectexplorer/deployconfiguration.h>
#include <projectexplorer/projectexplorerconstants.h>

using namespace ProjectExplorer;

namespace BareMetal {
namespace Internal {

class BareMetalDeployConfigurationFactory : public DeployConfigurationFactory
{
public:
    BareMetalDeployConfigurationFactory()
    {
        setConfigBaseId("BareMetal.DeployConfiguration");
        setDefaultDisplayName(QCoreApplication::translate("BareMetalDeployConfiguration",
                                                          "Deploy to BareMetal Device"));
        addSupportedTargetDeviceType(Constants::BareMetalOsType);
    }
};


// BareMetalPluginPrivate

class BareMetalPluginPrivate
{
public:
    IarToolChainFactory iarToolChainFactory;
    KeilToolChainFactory keilToolChainFactory;
    SdccToolChainFactory sdccToolChainFactory;
    BareMetalDeviceFactory deviceFactory;
    BareMetalRunConfigurationFactory runConfigurationFactory;
    BareMetalCustomRunConfigurationFactory customRunConfigurationFactory;
    DebugServerProvidersSettingsPage debugServerProviderSettinsPage;
    DebugServerProviderManager debugServerProviderManager;
    BareMetalDeployConfigurationFactory deployConfigurationFactory;

    RunWorkerFactory runWorkerFactory{
        RunWorkerFactory::make<BareMetalDebugSupport>(),
        {ProjectExplorer::Constants::NORMAL_RUN_MODE, ProjectExplorer::Constants::DEBUG_RUN_MODE},
        {runConfigurationFactory.runConfigurationId(),
         customRunConfigurationFactory.runConfigurationId()}
    };
};

// BareMetalPlugin

BareMetalPlugin::~BareMetalPlugin()
{
    delete d;
}

bool BareMetalPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    d = new BareMetalPluginPrivate;
    return true;
}

void BareMetalPlugin::extensionsInitialized()
{
    DebugServerProviderManager::instance()->restoreProviders();
}

} // namespace Internal
} // namespace BareMetal

#include "moc_baremetalplugin.cpp"
