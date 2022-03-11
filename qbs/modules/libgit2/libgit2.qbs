import qbs
import qbs.Probes

Module {
    id: libgit2Module

    Depends { name: "cpp" }

    property string rootPath: ""
    readonly property bool found: libgit2LibraryProbe.found && libgit2IncludeProbe.found

    Probes.IncludeProbe {
        id: libgit2IncludeProbe

        names: "git2.h"
        searchPaths: [libgit2Module.rootPath + "/include"]
    }
    Probes.LibraryProbe {
        id: libgit2LibraryProbe
    
        names: ["git2"]
        searchPaths: [libgit2Module.rootPath + "/lib"]
    }

    cpp.libraryPaths: libgit2LibraryProbe.found ? [libgit2LibraryProbe.path] : []
    cpp.dynamicLibraries: libgit2LibraryProbe.found ? [libgit2LibraryProbe.names[0]] : []
    cpp.includePaths: libgit2IncludeProbe.found ? [libgit2IncludeProbe.path] : []
    cpp.defines: libgit2Module.found ? ["LIBGIT2_FOUND"] : []
}
