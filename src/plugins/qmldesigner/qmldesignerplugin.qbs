import qbs 1.0
import qbs.FileInfo
import qbs.File

QtcPlugin {
    name: "QmlDesigner"
    condition: Qt.quickwidgets.present && Qt.svg.present && qbs.architecture == 'x86_64'

    Depends {
        name: "Qt"; versionAtLeast: "6.2"; required: false
        submodules: ["core-private", "quickwidgets", "xml", "svg"]
    }
    Depends { name: "AdvancedDockingSystem" }
    Depends { name: "Core" }
    Depends { name: "Qt"; submodules: ["gui", "qml-private"] }
    Depends { name: "Nanotrace"; required: false }
    Depends { name: "QmlJS" }
    Depends { name: "QmlEditorWidgets" }
    Depends { name: "TextEditor" }
    Depends { name: "QmlJSEditor" }
    Depends { name: "QmakeProjectManager" }
    Depends { name: "QmlProjectManager" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "LanguageUtils" }
    Depends { name: "QtSupport" }
    Depends { name: "app_version_header" }
    Depends { name: "Sqlite" }
    Depends { name: "QmlPuppetCommunication" }
    Depends { name: "QmlDesignerBase" }

    Probe {
        id: componentDirProbe
        property path basePath: FileInfo.joinPaths(product.sourceDirectory, "components")
        property stringList componentDirs
        configure: {
            componentDirs = File.directoryEntries(basePath, File.Dirs | File.NoDotAndDotDot);
            found = true;
        }
    }

    cpp.defines: base.concat([
        "QMLDESIGNERUTILS_LIBRARY",
        "QMLDESIGNER_LIBRARY",
        "QMLDESIGNERCORE_LIBRARY",
        "QMLDESIGNERUTILS_LIBRARY",
        "QMLDESIGNERCOMPONENTS_LIBRARY",
    ])
    cpp.enableExceptions: true
    cpp.includePaths: Array.prototype.concat(base, [
        ".",
        "utils",
        "designercore",
        "designercore/include",
        "designercore/instances",
        "designercore/projectstorage",
        "designercore/imagecache",
        "../../../share/qtcreator/qml/qmlpuppet/interfaces",
        "../../../share/qtcreator/qml/qmlpuppet/container",
        "../../../share/qtcreator/qml/qmlpuppet/commands",
        "../../../share/qtcreator/qml/qmlpuppet/types",
        "components",
    ], componentDirProbe.componentDirs.map(function(dir) { return "components/"+dir; }));

    Properties {
        condition: qbs.targetOS.contains("unix") && !qbs.targetOS.contains("bsd")
        cpp.dynamicLibraries: base.concat("rt")
    }

    Export {
        Depends { name: "QmlPuppetCommunication" }
        Depends { name: "QmlDesignerBase" }

        Depends { name: "QmlJS" }
        cpp.includePaths: Array.prototype.concat(base, [
            exportingProduct.sourceDirectory,
            exportingProduct.sourceDirectory + "/components",
            exportingProduct.sourceDirectory + "/components/componentcore",
            exportingProduct.sourceDirectory + "/components/edit3d",
            exportingProduct.sourceDirectory + "/components/formeditor",
            exportingProduct.sourceDirectory + "/components/integration",
            exportingProduct.sourceDirectory + "/designercore",
            exportingProduct.sourceDirectory + "/designercore/include",
            exportingProduct.qtc.export_data_base + "/qml/qmlpuppet/interfaces",
        ])
    }

    Group {
        name: "Utils"
        prefix: "utils/"
        files: ["*.cpp", "*.h", "*.qrc", "*.ui"]
    }

    Group {
        name: "DesignerCore"
        prefix: "designercore/"
        files: ["**/*.cpp", "**/*.h", "**/*.qrc", "**/*.ui"]
    }

    Group {
        name: "Components"
        prefix: "components/"
        files: ["**/*.cpp", "**/*.h", "**/*.qrc", "**/*.ui"]
    }


    files: ["*.cpp", "*.h", "*.qrc", "*.ui"]

}
