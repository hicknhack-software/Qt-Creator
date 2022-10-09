// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "designertr.h"
#include "formeditorw.h"
#include "settingspage.h"

#include <coreplugin/icontext.h>

#include <utils/stringutils.h>

#include <QDesignerOptionsPageInterface>
#include <QCoreApplication>

using namespace Designer::Internal;

SettingsPage::SettingsPage(QDesignerOptionsPageInterface *designerPage) :
    Core::IOptionsPage(nullptr, false),
    m_designerPage(designerPage)
{
    setId(Utils::Id::fromString(m_designerPage->name()));
    setDisplayName(m_designerPage->name());
    setCategory(Designer::Constants::SETTINGS_CATEGORY);
}

QWidget *SettingsPage::widget()
{
    m_initialized = true;
    if (!m_widget)
        m_widget = m_designerPage->createPage(nullptr);
    return m_widget;

}

void SettingsPage::apply()
{
    if (m_initialized)
        m_designerPage->apply();
}

void SettingsPage::finish()
{
    if (m_initialized)
        m_designerPage->finish();
    delete m_widget;
}

SettingsPageProvider::SettingsPageProvider()
{
    setCategory(Designer::Constants::SETTINGS_CATEGORY);
    setDisplayCategory(Tr::tr(Designer::Constants::SETTINGS_TR_CATEGORY));
    setCategoryIcon(Utils::Icon({{":/core/images/settingscategory_design.png",
                    Utils::Theme::PanelTextColorDark}}, Utils::Icon::Tint));
}

QList<Core::IOptionsPage *> SettingsPageProvider::pages() const
{
    if (!m_initialized) {
        // get options pages from designer
        m_initialized = true;
        FormEditorW::ensureInitStage(FormEditorW::RegisterPlugins);
    }
    return FormEditorW::optionsPages();
}

bool SettingsPageProvider::matches(const QRegularExpression &regex) const
{
    // to avoid fully initializing designer when typing something in the options' filter edit
    // we hardcode matching of UI text from the designer pages, which are taken if the designer pages
    // were not yet loaded
    // luckily linguist cannot resolve the translated texts, so we do not end up with duplicate
    // translatable strings for Qt Creator
    static const struct { const char *context; const char *value; } uitext[] = {
        {"EmbeddedOptionsPage", "Embedded Design"},
        {"EmbeddedOptionsPage", "Device Profiles"},
        {"FormEditorOptionsPage", "Forms"},
        {"FormEditorOptionsPage", "Preview Zoom"},
        {"FormEditorOptionsPage", "Default Zoom"},
        {"FormEditorOptionsPage", "Default Grid"},
        {"qdesigner_internal::GridPanel", "Visible"},
        {"qdesigner_internal::GridPanel", "Snap"},
        {"qdesigner_internal::GridPanel", "Reset"},
        {"qdesigner_internal::GridPanel", "Grid"},
        {"qdesigner_internal::GridPanel", "Grid &X"},
        {"qdesigner_internal::GridPanel", "Grid &Y"},
        {"PreviewConfigurationWidget", "Print/Preview Configuration"},
        {"PreviewConfigurationWidget", "Style"},
        {"PreviewConfigurationWidget", "Style sheet"},
        {"PreviewConfigurationWidget", "Device skin"},
        {"TemplateOptionsPage", "Template Paths"},
        {"qdesigner_internal::TemplateOptionsWidget", "Additional Template Paths"}
    };
    static const size_t itemCount = sizeof(uitext)/sizeof(uitext[0]);
    if (m_keywords.isEmpty()) {
        m_keywords.reserve(itemCount);
        for (size_t i = 0; i < itemCount; ++i)
            m_keywords << Utils::stripAccelerator(QCoreApplication::translate(uitext[i].context, uitext[i].value));
    }
    for (const QString &key : std::as_const(m_keywords)) {
        if (key.contains(regex))
            return true;
    }
    return false;
}

#include "moc_settingspage.cpp"
