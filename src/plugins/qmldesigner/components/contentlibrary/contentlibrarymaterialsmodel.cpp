// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "contentlibrarymaterialsmodel.h"

#include "contentlibrarybundleimporter.h"
#include "contentlibrarymaterial.h"
#include "contentlibrarymaterialscategory.h"
#include "contentlibrarywidget.h"

#include "designerpaths.h"
#include "filedownloader.h"
#include "fileextractor.h"
#include "multifiledownloader.h"

#include <qmldesignerplugin.h>

#include <utils/algorithm.h>
#include <utils/hostosinfo.h>
#include <utils/qtcassert.h>

#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QUrl>

namespace QmlDesigner {

ContentLibraryMaterialsModel::ContentLibraryMaterialsModel(ContentLibraryWidget *parent)
    : QAbstractListModel(parent)
    , m_widget(parent)
{
    m_downloadPath = Paths::bundlesPathSetting() + "/Materials";

    m_baseUrl = QmlDesignerPlugin::settings()
                    .value(DesignerSettingsKey::DOWNLOADABLE_BUNDLES_URL)
                    .toString() + "/materials/v1";

    qmlRegisterType<QmlDesigner::FileDownloader>("WebFetcher", 1, 0, "FileDownloader");
    qmlRegisterType<QmlDesigner::MultiFileDownloader>("WebFetcher", 1, 0, "MultiFileDownloader");
}

void ContentLibraryMaterialsModel::loadBundle()
{
    QDir bundleDir{m_downloadPath};
    if (fetchBundleMetadata(bundleDir) && fetchBundleIcons(bundleDir))
        loadMaterialBundle(bundleDir);
}

int ContentLibraryMaterialsModel::rowCount(const QModelIndex &) const
{
    return m_bundleCategories.size();
}

QVariant ContentLibraryMaterialsModel::data(const QModelIndex &index, int role) const
{
    QTC_ASSERT(index.isValid() && index.row() < m_bundleCategories.size(), return {});
    QTC_ASSERT(roleNames().contains(role), return {});

    return m_bundleCategories.at(index.row())->property(roleNames().value(role));
}

bool ContentLibraryMaterialsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !roleNames().contains(role))
        return false;

    QByteArray roleName = roleNames().value(role);
    ContentLibraryMaterialsCategory *bundleCategory = m_bundleCategories.at(index.row());
    QVariant currValue = bundleCategory->property(roleName);

    if (currValue != value) {
        bundleCategory->setProperty(roleName, value);

        emit dataChanged(index, index, {role});
        return true;
    }

    return false;
}

bool ContentLibraryMaterialsModel::isValidIndex(int idx) const
{
    return idx > -1 && idx < rowCount();
}

void ContentLibraryMaterialsModel::updateIsEmpty()
{
    const bool anyCatVisible = Utils::anyOf(m_bundleCategories,
                                            [&](ContentLibraryMaterialsCategory *cat) {
        return cat->visible();
    });

    const bool newEmpty = !anyCatVisible || m_bundleCategories.isEmpty()
            || !m_widget->hasMaterialLibrary() || !hasRequiredQuick3DImport();

    if (newEmpty != m_isEmpty) {
        m_isEmpty = newEmpty;
        emit isEmptyChanged();
    }
}

QHash<int, QByteArray> ContentLibraryMaterialsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        {Qt::UserRole + 1, "bundleCategoryName"},
        {Qt::UserRole + 2, "bundleCategoryVisible"},
        {Qt::UserRole + 3, "bundleCategoryExpanded"},
        {Qt::UserRole + 4, "bundleCategoryMaterials"}
    };
    return roles;
}

bool ContentLibraryMaterialsModel::fetchBundleIcons(const QDir &bundleDir)
{
    QString iconsPath = bundleDir.filePath("icons");

    QDir iconsDir(iconsPath);
    if (iconsDir.exists() && iconsDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot).length() > 0)
        return true;

    QString zipFileUrl = m_baseUrl + "/icons.zip";

    FileDownloader *downloader = new FileDownloader(this);
    downloader->setUrl(zipFileUrl);
    downloader->setProbeUrl(false);
    downloader->setDownloadEnabled(true);

    QObject::connect(downloader, &FileDownloader::finishedChanged, this,
                     [this, downloader, bundleDir] {
        FileExtractor *extractor = new FileExtractor(this);
        extractor->setArchiveName(downloader->completeBaseName());
        extractor->setSourceFile(downloader->outputFile());
        extractor->setTargetPath(bundleDir.absolutePath());
        extractor->setAlwaysCreateDir(false);
        extractor->setClearTargetPathContents(false);

        QObject::connect(extractor, &FileExtractor::finishedChanged, this,
                         [this, downloader, bundleDir, extractor] {
            downloader->deleteLater();
            extractor->deleteLater();

            loadMaterialBundle(bundleDir);
        });

        extractor->extract();
    });

    downloader->start();
    return false;
}

bool ContentLibraryMaterialsModel::fetchBundleMetadata(const QDir &bundleDir)
{
    QString matBundlePath = bundleDir.filePath("material_bundle.json");

    QFileInfo fi(matBundlePath);
    if (fi.exists() && fi.size() > 0)
        return true;

    QString metaFileUrl = m_baseUrl + "/material_bundle.json";
    FileDownloader *downloader = new FileDownloader(this);
    downloader->setUrl(metaFileUrl);
    downloader->setProbeUrl(false);
    downloader->setDownloadEnabled(true);
    downloader->setTargetFilePath(matBundlePath);

    QObject::connect(downloader, &FileDownloader::finishedChanged, this,
                     [this, downloader, bundleDir] {
        if (fetchBundleIcons(bundleDir))
            loadMaterialBundle(bundleDir);
        downloader->deleteLater();
    });

    downloader->start();
    return false;
}

void ContentLibraryMaterialsModel::downloadSharedFiles(const QDir &targetDir, const QStringList &)
{
    QString metaFileUrl = m_baseUrl + "/shared_files.zip";
    FileDownloader *downloader = new FileDownloader(this);
    downloader->setUrl(metaFileUrl);
    downloader->setProbeUrl(false);
    downloader->setDownloadEnabled(true);

    QObject::connect(downloader, &FileDownloader::finishedChanged, this,
                     [this, downloader, targetDir] {
        FileExtractor *extractor = new FileExtractor(this);
        extractor->setArchiveName(downloader->completeBaseName());
        extractor->setSourceFile(downloader->outputFile());
        extractor->setTargetPath(targetDir.absolutePath());
        extractor->setAlwaysCreateDir(false);
        extractor->setClearTargetPathContents(false);

        QObject::connect(extractor, &FileExtractor::finishedChanged, this, [downloader, extractor]() {
            downloader->deleteLater();
            extractor->deleteLater();
        });

        extractor->extract();
    });

    downloader->start();
}

QString ContentLibraryMaterialsModel::bundleId() const
{
    return m_bundleId;
}

void ContentLibraryMaterialsModel::loadMaterialBundle(const QDir &bundleDir)
{
    auto compUtils = QmlDesignerPlugin::instance()->documentManager().generatedComponentUtils();

    if (m_bundleExists && m_bundleId == compUtils.materialsBundleId())
        return;

    // clean up
    qDeleteAll(m_bundleCategories);
    m_bundleCategories.clear();
    m_bundleExists = false;
    m_isEmpty = true;
    m_bundleObj = {};
    m_bundleId.clear();

    QString bundlePath = bundleDir.filePath("material_bundle.json");

    QFile bundleFile(bundlePath);
    if (!bundleFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open material_bundle.json");
        resetModel();
        return;
    }

    QJsonDocument bundleJsonDoc = QJsonDocument::fromJson(bundleFile.readAll());
    if (bundleJsonDoc.isNull()) {
        qWarning("Invalid material_bundle.json file");
        resetModel();
        return;
    }

    m_bundleObj = bundleJsonDoc.object();

    QString bundleType = compUtils.materialsBundleType();
    m_bundleId = compUtils.materialsBundleId();

    const QJsonObject catsObj = m_bundleObj.value("categories").toObject();
    const QStringList categories = catsObj.keys();
    for (const QString &cat : categories) {
        auto category = new ContentLibraryMaterialsCategory(this, cat);

        const QJsonObject matsObj = catsObj.value(cat).toObject();
        const QStringList matsNames = matsObj.keys();
        for (const QString &matName : matsNames) {
            const QJsonObject matObj = matsObj.value(matName).toObject();

            QStringList files;
            const QJsonArray assetsArr = matObj.value("files").toArray();
            for (const QJsonValueConstRef &asset : assetsArr)
                files.append(asset.toString());

            QUrl icon = QUrl::fromLocalFile(bundleDir.filePath(matObj.value("icon").toString()));
            QString qml = matObj.value("qml").toString();
            TypeName type = QLatin1String("%1.%2")
                                .arg(bundleType, qml.chopped(4)).toLatin1(); // chopped(4): remove .qml

            auto bundleMat = new ContentLibraryMaterial(category, matName, qml, type, icon, files,
                                                        m_downloadPath, m_baseUrl);

            category->addBundleMaterial(bundleMat);
        }
        m_bundleCategories.append(category);
    }

    m_bundleSharedFiles.clear();
    const QJsonArray sharedFilesArr = m_bundleObj.value("sharedFiles").toArray();
    for (const QJsonValueConstRef &file : sharedFilesArr)
        m_bundleSharedFiles.append(file.toString());

    QStringList missingSharedFiles;
    for (const QString &s : std::as_const(m_bundleSharedFiles)) {
        if (!QFileInfo::exists(bundleDir.filePath(s)))
            missingSharedFiles.push_back(s);
    }

    if (missingSharedFiles.length() > 0)
        downloadSharedFiles(bundleDir, missingSharedFiles);

    m_bundleExists = true;
    updateIsEmpty();
    resetModel();
}

bool ContentLibraryMaterialsModel::hasRequiredQuick3DImport() const
{
    return m_widget->hasQuick3DImport() && m_quick3dMajorVersion == 6 && m_quick3dMinorVersion >= 3;
}

bool ContentLibraryMaterialsModel::matBundleExists() const
{
    return m_bundleExists;
}

void ContentLibraryMaterialsModel::setSearchText(const QString &searchText)
{
    QString lowerSearchText = searchText.toLower();

    if (m_searchText == lowerSearchText)
        return;

    m_searchText = lowerSearchText;

    for (int i = 0; i < m_bundleCategories.size(); ++i) {
        ContentLibraryMaterialsCategory *cat = m_bundleCategories.at(i);
        bool catVisibilityChanged = cat->filter(m_searchText);
        if (catVisibilityChanged)
            emit dataChanged(index(i), index(i), {roleNames().keys("bundleCategoryVisible")});
    }

    updateIsEmpty();
}

void ContentLibraryMaterialsModel::updateImportedState(const QStringList &importedItems)
{
    bool changed = false;
    for (ContentLibraryMaterialsCategory *cat : std::as_const(m_bundleCategories))
        changed |= cat->updateImportedState(importedItems);

    if (changed)
        resetModel();
}

void ContentLibraryMaterialsModel::setQuick3DImportVersion(int major, int minor)
{
    bool oldRequiredImport = hasRequiredQuick3DImport();

    m_quick3dMajorVersion = major;
    m_quick3dMinorVersion = minor;

    bool newRequiredImport = hasRequiredQuick3DImport();

    if (oldRequiredImport == newRequiredImport)
        return;

    emit hasRequiredQuick3DImportChanged();

    updateIsEmpty();
}

void ContentLibraryMaterialsModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

void ContentLibraryMaterialsModel::applyToSelected(ContentLibraryMaterial *mat, bool add)
{
    emit applyToSelectedTriggered(mat, add);
}

void ContentLibraryMaterialsModel::addToProject(ContentLibraryMaterial *mat)
{
    QString err = m_widget->importer()->importComponent(mat->dirPath(), mat->type(), mat->qml(),
                                                        mat->files() + m_bundleSharedFiles);

    if (err.isEmpty())
        m_widget->setImporterRunning(true);
    else
        qWarning() << __FUNCTION__ << err;
}

void ContentLibraryMaterialsModel::removeFromProject(ContentLibraryMaterial *mat)
{
    QString err = m_widget->importer()->unimportComponent(mat->type(), mat->qml());

    if (err.isEmpty())
        m_widget->setImporterRunning(true);
    else
        qWarning() << __FUNCTION__ << err;
}

} // namespace QmlDesigner

#include "moc_contentlibrarymaterialsmodel.cpp"
