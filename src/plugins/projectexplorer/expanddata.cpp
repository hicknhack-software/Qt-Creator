// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "expanddata.h"

#include <QVariant>

using namespace ProjectExplorer;
using namespace ProjectExplorer::Internal;

ExpandData::ExpandData(const QString &path, const QString &displayName, int priority) : path(path), displayName(displayName), priority(priority) {}

bool ExpandData::operator==(const ExpandData &other) const
{
    return path == other.path && displayName == other.displayName && priority == other.priority;
}

ExpandData ExpandData::fromSettings(const QVariant &v)
{
    const QVariantList list = v.toList();
    if (list.size() == 3) return ExpandData(list.at(0).toString(), list.at(1).toString(), list.at(2).toInt());
    return list.size() == 2 ? ExpandData(list.at(0).toString(), QString(), list.at(1).toInt()) : ExpandData();
}

QVariant ExpandData::toSettings() const
{
    return QVariantList{path, displayName, priority};
}

size_t ProjectExplorer::Internal::qHash(const ExpandData &data)
{
    return qHash(data.path) ^ qHash(data.displayName) ^ data.priority;
}
