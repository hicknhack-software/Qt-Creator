import qbs

QtcProduct {
    name: "assetexporterplugin"
    condition: QmlDesigner.present
    type: ["dynamiclibrary"]
    installDir: qtc.ide_plugin_path + '/' + installDirName
    property string installDirName: qbs.targetOS.contains("macos") ? "QmlDesigner" : "qmldesigner"

    Depends { name: "Core" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "QmlDesigner"; required: false }
    Depends { name: "Utils" }
    Depends {
        name: "Qt"
        submodules: [
            "quick-private"
        ]
    }

    cpp.includePaths: base.concat([
        "./",
        "../utils",
    ])

    Properties {
        condition: qbs.targetOS.contains("unix")
        cpp.internalVersion: ""
    }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: base.concat([
            "./",
        ])
    }

    Group {
        name: "plugin metadata"
        files: ["assetexporterplugin.json"]
        fileTags: ["qt_plugin_metadata"]
    }

    files: [
        "assetexportdialog.h",
        "assetexportdialog.cpp",
        "assetexporter.h",
        "assetexporter.cpp",
        "assetexporterplugin.h",
        "assetexporterplugin.cpp",
        "assetexporterview.h",
        "assetexporterview.cpp",
        "assetexportpluginconstants.h",
        "componentexporter.h",
        "componentexporter.cpp",
        "exportnotification.h",
        "exportnotification.cpp",
        "filepathmodel.h",
        "filepathmodel.cpp",
        "dumpers/assetnodedumper.h",
        "dumpers/assetnodedumper.cpp",
        "dumpers/itemnodedumper.h",
        "dumpers/itemnodedumper.cpp",
        "dumpers/nodedumper.h",
        "dumpers/nodedumper.cpp",
        "dumpers/textnodedumper.h",
        "dumpers/textnodedumper.cpp",
        "assetexporterplugin.qrc",
    ]
}
