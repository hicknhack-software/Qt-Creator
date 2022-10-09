// Copyright (C) 2016 Denis Mingulov
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "classviewplugin.h"
#include "classviewmanager.h"
#include "classviewnavigationwidgetfactory.h"

namespace ClassView {
namespace Internal {

///////////////////////////////// Plugin //////////////////////////////////

/*!
    \class ClassViewPlugin
    \brief The ClassViewPlugin class implements the Class View plugin.

    The Class View shows the namespace and class hierarchy of the currently open
    projects in the sidebar.
*/

class ClassViewPluginPrivate
{
public:
    NavigationWidgetFactory navigationWidgetFactory;
    Manager manager;
};

static ClassViewPluginPrivate *dd = nullptr;

ClassViewPlugin::~ClassViewPlugin()
{
    delete dd;
    dd = nullptr;
}

bool ClassViewPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    dd = new ClassViewPluginPrivate;

    return true;
}

} // namespace Internal
} // namespace ClassView

#include "moc_classviewplugin.cpp"
