QtcLibrary {
    name: "Tracing"

    Depends { name: "Qt"; submodules: ["qml", "quick", "gui"] }
    Depends { name: "Qt.testlib"; condition: qtc.withAutotests }
    Depends { name: "Utils" }

    Qt.qml.importName: "QtCreator.Tracing"
    Qt.qml.importVersion: "1.0"
    cpp.includePaths: ["."] // needed to allow registraction to compile

    Group {
        name: "General"
        files: [
            "README",
            "flamegraph.cpp", "flamegraph.h",
            "flamegraphattached.h",
            "safecastable.h",
            "timelineabstractrenderer.cpp", "timelineabstractrenderer.h",
            "timelineformattime.cpp", "timelineformattime.h",
            "timelineitemsrenderpass.cpp", "timelineitemsrenderpass.h",
            "timelinemodel.cpp", "timelinemodel.h", "timelinemodel_p.h",
            "timelinemodelaggregator.cpp", "timelinemodelaggregator.h",
            "timelinenotesmodel.cpp", "timelinenotesmodel.h",
            "timelinenotesrenderpass.cpp", "timelinenotesrenderpass.h",
            "timelineoverviewrenderer.cpp", "timelineoverviewrenderer.h",
            "timelinerenderer.cpp", "timelinerenderer.h",
            "timelinerenderpass.cpp", "timelinerenderpass.h",
            "timelinerenderstate.cpp", "timelinerenderstate.h",
            "timelineselectionrenderpass.cpp", "timelineselectionrenderpass.h",
            "timelinetheme.cpp", "timelinetheme.h",
            "timelinetracefile.cpp", "timelinetracefile.h",
            "timelinetracemanager.cpp", "timelinetracemanager.h",
            "timelinezoomcontrol.cpp", "timelinezoomcontrol.h",
            "traceevent.h", "traceeventtype.h", "tracestashfile.h",
            "tracingtr.h",
        ]
    }

    Depends { name: "qsb_compiler" }
    Depends { name: "qmldir" }
    Qt.core.resourcePrefix: "/qt/qml/QtCreator/Tracing"
    Group {
        name: "Qml Files"
        fileTags: ["qt.qml.qml", "qt.core.resource_data"]
        prefix: "qml/"
        files: ["**/*.qml"]
    }
    Group {
        name: "Images"
        fileTags: ["qt.core.resource_data"]
        prefix: "qml/"
        files: ["**/*.png"]
    }
    Group {
        name: "Shaders"
        qsb_compiler.extraArgs: ["--batchable", "--qt6"]
        prefix: "qml/"
        files: ["**/*.vert", "**/*.frag"]
    }

    cpp.defines: base.concat("TRACING_LIBRARY")
}
