import qbs

QtcPlugin {
    name: "QmlDesignerBase"

    Depends { name: "Core" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "QtSupport" }
    Depends { name: "Qt.quickwidgets" }

    files: [
        "qmldesignerbase_global.h",
        "qmldesignerbaseplugin.cpp",
        "qmldesignerbaseplugin.h",
    ]

    Group {
        name: 'Studio'
        prefix: "studio/"
        files: [
            "*.cpp",
            "*.h",
        ]
    }
    Group {
        name: "Utils"
        prefix: "utils/"
        files: [
            "*.cpp",
            "*.h",
        ]
    }

    Export {
        cpp.includePaths: Array.prototype.concat(base, [
            exportingProduct.sourceDirectory,
            exportingProduct.sourceDirectory + "/studio",
            exportingProduct.sourceDirectory + "/utils",
        ])
    }
}
