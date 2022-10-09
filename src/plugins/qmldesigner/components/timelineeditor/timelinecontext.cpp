// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "timelinecontext.h"
#include "timelineconstants.h"
#include "timelinewidget.h"

namespace QmlDesigner {

TimelineContext::TimelineContext(QWidget *widget)
    : IContext(widget)
{
    setWidget(widget);
    setContext(Core::Context(TimelineConstants::C_QMLTIMELINE));
}

void TimelineContext::contextHelp(const Core::IContext::HelpCallback &callback) const
{
    if (auto *widget = qobject_cast<TimelineWidget *>(m_widget))
        widget->contextHelp(callback);
}

} // namespace QmlDesigner

#include "moc_timelinecontext.cpp"
