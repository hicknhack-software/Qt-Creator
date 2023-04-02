// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#pragma once
#include "qmldesignerutils_global.h"

#include <QString>

namespace QmlDesigner {

class QMLDESIGNERUTILS_EXPORT ImageUtils
{
public:
    ImageUtils();

    static QString imageInfo(const QString &path);
};

} // namespace QmlDesigner
