import qbs

QtcAutotest {
    name: "LibGit2Cpp autotest"
    files: "testgitrepo.cpp"

    Depends { name: "LibGit2Cpp" }
    Depends { name: "Utils" }
}
