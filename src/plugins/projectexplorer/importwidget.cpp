// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "importwidget.h"

#include <utils/detailswidget.h>
#include <utils/pathchooser.h>

#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

namespace ProjectExplorer {
namespace Internal {

ImportWidget::ImportWidget(QWidget *parent) :
    QWidget(parent),
    m_pathChooser(new Utils::PathChooser)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto vboxLayout = new QVBoxLayout();
    setLayout(vboxLayout);
    vboxLayout->setContentsMargins(0, 0, 0, 0);
    auto detailsWidget = new Utils::DetailsWidget(this);
    detailsWidget->setUseCheckBox(false);
    detailsWidget->setSummaryText(tr("Import Build From..."));
    detailsWidget->setSummaryFontBold(true);
    // m_detailsWidget->setIcon(); // FIXME: Set icon!
    vboxLayout->addWidget(detailsWidget);

    auto widget = new QWidget;
    auto layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pathChooser);

    m_pathChooser->setExpectedKind(Utils::PathChooser::ExistingDirectory);
    m_pathChooser->setHistoryCompleter(QLatin1String("Import.SourceDir.History"));
    auto importButton = new QPushButton(tr("Import"), widget);
    layout->addWidget(importButton);

    connect(importButton, &QAbstractButton::clicked, this, &ImportWidget::handleImportRequest);
    connect(m_pathChooser->lineEdit(), &QLineEdit::returnPressed, this, [this] {
        if (m_pathChooser->isValid()) {
            m_ownsReturnKey = true;
            handleImportRequest();

            // The next return should trigger the "Configure" button.
            QTimer::singleShot(0, this, [this] {
                setFocus();
                m_ownsReturnKey = false;
            });
        }
    });

    detailsWidget->setWidget(widget);
}

void ImportWidget::setCurrentDirectory(const Utils::FilePath &dir)
{
    m_pathChooser->setBaseDirectory(dir);
    m_pathChooser->setFilePath(dir);
}

bool ImportWidget::ownsReturnKey() const
{
    return m_ownsReturnKey;
}

void ImportWidget::handleImportRequest()
{
    Utils::FilePath dir = m_pathChooser->filePath();
    emit importFrom(dir);

    m_pathChooser->setFilePath(m_pathChooser->baseDirectory());
}

} // namespace Internal
} // namespace ProjectExplorer

#include "moc_importwidget.cpp"
