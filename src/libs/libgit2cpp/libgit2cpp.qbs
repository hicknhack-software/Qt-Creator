import qbs
import qbs.Probes

Project {
    name: "LibGit2Cpp"

    Probes.IncludeProbe {
        id: libgit2IncludeProbe

        names: ["git2.h"]
        pathSuffixes: ["include"]
        environmentPaths: ["LIBGIT2_INSTALL_DIR"]
    }
    Probes.LibraryProbe {
        id: libgit2LibraryProbe

        names: ["git2"]
        pathSuffixes: ["lib"]
        environmentPaths: ["LIBGIT2_INSTALL_DIR"]
    }

    QtcLibrary {
        condition: libgit2LibraryProbe.found && libgit2IncludeProbe.found

        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["core", "concurrent"] }
        Depends { name: "Utils" }

        files: [
            "gitrepo.cpp",
            "gitrepo.h",
            "libgit2cpp_global.h",
        ]
        cpp.defines: base.concat("LIBGIT2CPP_BUILD_LIB")
        cpp.libraryPaths: libgit2LibraryProbe.found ? [libgit2LibraryProbe.path] : []
        cpp.dynamicLibraries: libgit2LibraryProbe.found ? [libgit2LibraryProbe.names[0]] : []
        cpp.includePaths: libgit2IncludeProbe.found ? [libgit2IncludeProbe.path] : []
    }
}
