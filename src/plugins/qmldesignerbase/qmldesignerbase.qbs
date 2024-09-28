import qbs

QtcPlugin {
    name: "QmlDesignerBase"

    Depends { name: "Core" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "QtSupport" }
    Depends { name: "Qt.quickwidgets" }
    Depends { name: "Qt.gui-private" }

    cpp.includePaths: ["settings", "studio", "utils"]
    cpp.defines: base.concat(["QMLDESIGNERSETTINGS_LIBRARY"])

    files: [
        "qmldesignerbase_global.h",
        "qmldesignerbaseplugin.cpp",
        "qmldesignerbaseplugin.h",
    ]

    Group {
        prefix: "studio/"
        files: [
            "studioquickutils.cpp",
            "studioquickutils.h",
            "studioquickwidget.cpp",
            "studioquickwidget.h",
            "studiosettingspage.cpp",
            "studiosettingspage.h",
            "studiostyle.cpp",
            "studiostyle.h",
            "studiostyle_p.cpp",
            "studiostyle_p.h",
            "studiovalidator.cpp",
            "studiovalidator.h",
        ]
    }
    Group {
        prefix: "utils/"
        files: [
            "designerpaths.cpp",
            "designerpaths.h",
            "qmlpuppetpaths.cpp",
            "qmlpuppetpaths.h",
            "windowmanager.cpp",
            "windowmanager.h",
        ]
        Group {
            prefix: "settings/"
            files: [
                "designersettings.cpp",
                "designersettings.h",
                "qmldesignersettings_global.h",
            ]
        }
    }
}
