// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "imageprovider.h"

using namespace ScxmlEditor::PluginInterface;

ImageProvider::ImageProvider(QObject *parent)
    : QObject(parent)
{
}

#include "moc_imageprovider.cpp"
