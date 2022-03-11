import qbs 1.0

Project {
    name: "LibGit2Cpp"

    QtcLibrary {
        id: myLib

        Depends { name: "Qt"; submodules: ["core", "concurrent"] }

        Depends { name: "libgit2" }

        Depends { name: "Utils" }

        files: [
            "gitrepo.cpp",
            "gitrepo.h",
            "libgit2cpp_global.h",
        ]
        condition: libgit2.found
        cpp.defines: base.concat("LIBGIT2CPP_BUILD_LIB")

        //readonly property bool debugFoo: {
        //    if (libgit2.found) {
        //        throw("utils " + myLib.builtByDefault)
        //    }
        //}
    }
}
