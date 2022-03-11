import qbs 1.0

QtcPlugin {
    name: "GitVcsChanges"

    Depends { name: "Core" }
    Depends { name: "ProjectExplorer" }
    Depends { name: "TextEditor" }
    Depends { name: "Qt"; submodules: ["widgets", "network"] }

    files: [
        "gitvcschangesplugin.cpp",
        "gitvcschangesplugin.h",
        "gitscrollbarhighlighter.cpp",
        "gitscrollbarhighlighter.h",
    ]
}
