/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "gitvcschangesplugin.h"

#include "gitscrollbarhighlighter.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>

#include <QAction>
#include <QMenu>

namespace GitVcsChanges {
namespace Internal {

GitVcsChangesPlugin::GitVcsChangesPlugin() {}

GitVcsChangesPlugin::~GitVcsChangesPlugin()
{}

bool GitVcsChangesPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    m_scrollbarHighlighter = new GitScrollBarHighlighter{this};

    Core::ActionContainer *menu = Core::ActionManager::createMenu("GitVcsChanges.Menu");
    menu->menu()->setTitle(tr("&GitVcsChanges"));

    m_gotoNextLine = new QAction(tr("Goto &Next uncommited line."), this);
    Core::Command *nextCmd
        = Core::ActionManager::registerAction(m_gotoNextLine,
                                              "GitVcsChanges.GotoNextUncommitedLine");
    menu->addAction(nextCmd);
    connect(m_gotoNextLine, &QAction::triggered, this, [=](bool) {
        m_scrollbarHighlighter->gotoUncommitedLine(MoveDirection::Next);
    });

    m_gotoPreviousLine = new QAction(tr("Goto &Previous uncommited line."), this);
    Core::Command *previousCmd
        = Core::ActionManager::registerAction(m_gotoPreviousLine,
                                              "GitVcsChanges.GotoPreviousUncommitedLine");
    menu->addAction(previousCmd);
    connect(m_gotoPreviousLine, &QAction::triggered, this, [=](bool) {
        m_scrollbarHighlighter->gotoUncommitedLine(MoveDirection::Previous);
    });
    return true;
}

void GitVcsChangesPlugin::extensionsInitialized() {}

} // namespace Internal
} // namespace GitVcsChanges
