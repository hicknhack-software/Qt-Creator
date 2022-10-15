import qbs 1.0

QtcLibrary {
    name: "QmlPuppetCommunication"
    type: "staticlibrary"

    Depends { name: "Qt"; submodules: ["core", "gui", "core-private"] }
    Depends { name: "Nanotrace" }

    cpp.optimization: "fast"
    cpp.includePaths: base.concat([
        ".",
        "types",
        "interfaces",
        "container",
        "commands",
    ])

    Export {
        Depends { name: "Nanotrace" }
        cpp.includePaths: Array.prototype.concat(base, [
            ".",
            "types",
            "interfaces",
            "container",
            "commands",
        ])
    }

    Group {
        name: "Types"
        prefix: "types/"
        files: [
            "enumeration.h"
        ]
    }
    Group {
        name: "Interfaces"
        prefix: "interfaces/"
        files: [
            "commondefines.h",
            "nodeinstanceclientinterface.h",
            "nodeinstanceserverinterface.cpp", "nodeinstanceserverinterface.h",
            "nodeinstanceglobal.h",
        ]
    }
    Group {
        name: "Container"
        prefix: "container/"
        files: [
            "addimportcontainer.cpp", "addimportcontainer.h",
            "idcontainer.cpp", "idcontainer.h",
            "imagecontainer.cpp", "imagecontainer.h",
            "informationcontainer.cpp", "informationcontainer.h",
            "instancecontainer.cpp", "instancecontainer.h",
            "mockuptypecontainer.cpp", "mockuptypecontainer.h",
            "propertyabstractcontainer.cpp", "propertyabstractcontainer.h",
            "propertybindingcontainer.cpp", "propertybindingcontainer.h",
            "propertyvaluecontainer.cpp", "propertyvaluecontainer.h",
            "reparentcontainer.cpp", "reparentcontainer.h",
            "sharedmemory.h",
            (qbs.targetOS.contains("unix") ? "sharedmemory_unix.cpp" : "sharedmemory_qt.cpp"),
        ]
    }
    Group {
        name: "Commands"
        prefix: "commands/"
        files: [
            "captureddatacommand.h",
            "changeauxiliarycommand.cpp", "changeauxiliarycommand.h",
            "changebindingscommand.cpp", "changebindingscommand.h",
            "changefileurlcommand.cpp", "changefileurlcommand.h",
            "changeidscommand.cpp", "changeidscommand.h",
            "changelanguagecommand.cpp", "changelanguagecommand.h",
            "changenodesourcecommand.cpp", "changenodesourcecommand.h",
            "changepreviewimagesizecommand.cpp", "changepreviewimagesizecommand.h",
            "changeselectioncommand.cpp", "changeselectioncommand.h",
            "changestatecommand.cpp", "changestatecommand.h",
            "changevaluescommand.cpp", "changevaluescommand.h",
            "childrenchangedcommand.cpp", "childrenchangedcommand.h",
            "clearscenecommand.cpp", "clearscenecommand.h",
            "completecomponentcommand.cpp", "completecomponentcommand.h",
            "componentcompletedcommand.cpp", "componentcompletedcommand.h",
            "createinstancescommand.cpp", "createinstancescommand.h",
            "createscenecommand.cpp", "createscenecommand.h",
            "debugoutputcommand.cpp", "debugoutputcommand.h",
            "endpuppetcommand.cpp", "endpuppetcommand.h",
            "informationchangedcommand.cpp", "informationchangedcommand.h",
            "inputeventcommand.cpp", "inputeventcommand.h",
            "nanotracecommand.cpp", "nanotracecommand.h",
            "pixmapchangedcommand.cpp", "pixmapchangedcommand.h",
            "puppetalivecommand.cpp", "puppetalivecommand.h",
            "puppettocreatorcommand.cpp", "puppettocreatorcommand.h",
            "removeinstancescommand.cpp", "removeinstancescommand.h",
            "removepropertiescommand.cpp", "removepropertiescommand.h",
            "removesharedmemorycommand.cpp", "removesharedmemorycommand.h",
            "reparentinstancescommand.cpp", "reparentinstancescommand.h",
            "requestmodelnodepreviewimagecommand.cpp", "requestmodelnodepreviewimagecommand.h",
            "scenecreatedcommand.h",
            "statepreviewimagechangedcommand.cpp", "statepreviewimagechangedcommand.h",
            "synchronizecommand.h",
            "tokencommand.cpp", "tokencommand.h",
            "update3dviewstatecommand.cpp", "update3dviewstatecommand.h",
            "valueschangedcommand.cpp", "valueschangedcommand.h",
            "view3dactioncommand.cpp", "view3dactioncommand.h",
        ]
    }
}
