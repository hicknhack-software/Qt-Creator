import qbs
import qbs.Environment
import qbs.File
import qbs.FileInfo
import qbs.Process
import qbs.TextFile
import qbs.Utilities

QtcLibrary {
    condition: qbs.toolchain.contains("msvc")
    name: "qtcreatorcdbext"
    targetName: name

    property string pythonInstallDir: Environment.getEnv("PYTHON_INSTALL_DIR")

    Probe {
        id: pythonDllProbe
        condition: product.condition
        property string pythonDir: pythonInstallDir // Input
        property string buildVariant: qbs.buildVariant // Input
        property string fileNamePrefix // Output
        configure: {
            function printWarning(msg) {
                console.warn(msg + " The python dumpers for cdb will not be available.");
            }

            if (!pythonDir) {
                printWarning("PYTHON_INSTALL_DIR not set.");
                return;
            }
            if (!File.exists(pythonDir)) {
                printWarning("The provided python installation directory '" + pythonDir
                             + "' does not exist.");
                return;
            }
            var p = new Process();
            try {
                var pythonFilePath = FileInfo.joinPaths(pythonDir, "python.exe");
                p.exec(pythonFilePath, ["--version"], true);
                var output = p.readStdOut().trim();
                var magicPrefix = "Python ";
                if (!output.startsWith(magicPrefix)) {
                    printWarning("Unexpected python output when checking for version: '"
                                 + output + "'");
                    return;
                }
                var versionNumberString = output.slice(magicPrefix.length);
                var versionNumbers = versionNumberString.split('.');
                if (versionNumbers.length < 2) {
                    printWarning("Unexpected python output when checking for version: '"
                            + output + "'");
                    return;
                }
                var pythonMinVersion = "3.8";
                if (Utilities.versionCompare(versionNumberString, pythonMinVersion) < 0) {
                    printWarning("The python installation at '" + pythonDir
                                 + "' has version " + versionNumberString + ", but "
                                 + pythonMinVersion + " or higher is required.");
                    return;
                }
                found = true;
                fileNamePrefix = "python" + versionNumbers[0] + versionNumbers[1];
                if (buildVariant === "debug")
                    fileNamePrefix += "_d"
            } finally {
                p.close();
            }

        }
    }

    Group {
        name: "pythonDumper"
        condition: pythonDllProbe.found
        files: [
            "pycdbextmodule.cpp",
            "pycdbextmodule.h",
            "pyfield.cpp",
            "pyfield.h",
            "pystdoutredirect.cpp",
            "pystdoutredirect.h",
            "pytype.cpp",
            "pytype.h",
            "pyvalue.cpp",
            "pyvalue.h",
        ]
    }

    Properties {
        condition: pythonDllProbe.found
        cpp.defines: ["WITH_PYTHON=1", "PY_SSIZE_T_CLEAN"]
    }
    cpp.includePaths: {
        if (pythonDllProbe.found)
            return [ FileInfo.joinPaths(pythonInstallDir, "include") ];
        return [ ];
    }
    cpp.dynamicLibraries: {
        var libs = [ "user32.lib", "dbgeng.lib" ];
        if (pythonDllProbe.found)
            libs.push(FileInfo.joinPaths(pythonInstallDir, "libs",
                                        pythonDllProbe.fileNamePrefix + ".lib"));
        return libs;
    }
    cpp.linkerFlags: ["/DEF:" + FileInfo.toWindowsSeparators(
                                    FileInfo.joinPaths(product.sourceDirectory,
                                                    "qtcreatorcdbext.def"))]
    installDir: {
        var dirName = name;
        if (qbs.architecture.contains("x86_64"))
            dirName += "64";
        else
            dirName += "32";
        return FileInfo.joinPaths(qtc.libDirName, dirName);
    }

    Group {
        condition: pythonDllProbe.found
        files: [FileInfo.joinPaths(pythonInstallDir, pythonDllProbe.fileNamePrefix + ".dll")]
        qbs.install: true
        qbs.installDir: installDir
    }

    useNonGuiPchFile: false

    files: [
        "common.cpp",
        "common.h",
        "containers.cpp",
        "containers.h",
        "eventcallback.cpp",
        "eventcallback.h",
        "extensioncontext.cpp",
        "extensioncontext.h",
        "gdbmihelpers.cpp",
        "gdbmihelpers.h",
        "iinterfacepointer.h",
        "knowntype.h",
        "outputcallback.cpp",
        "outputcallback.h",
        "qtcreatorcdbextension.cpp",
        "stringutils.cpp",
        "stringutils.h",
        "symbolgroup.cpp",
        "symbolgroup.h",
        "symbolgroupnode.cpp",
        "symbolgroupnode.h",
        "symbolgroupvalue.cpp",
        "symbolgroupvalue.h",
    ]

    Group {
        condition: pythonDllProbe.found
        prefix: FileInfo.joinPaths(pythonInstallDir, "Lib") + '/'
        files: [
            "*.py",
            "collections/*.py",
            "encodings/*.py",
            "importlib/*.py",
            "json/*.py",
            "urllib/*.py",
        ]
        excludeFiles: [
            "aifc.py",              "imghdr.py",        "socket.py",
            "antigravity.py",       "imp.py",           "socketserver.py",
            "argparse.py",          "ipaddress.py",     "ssl.py",
            "asynchat.py",          "locale.py",        "statistics.py",
            "asyncore.py",          "lzma.py",          "string.py",
            "bdb.py",               "mailbox.py",       "stringprep.py",
            "binhex.py",            "mailcap.py",       "sunau.py",
            "bisect.py",            "mimetypes.py",     "symbol.py",
            "bz2.py",               "modulefinder.py",  "symtable.py",
            "calendar.py",          "netrc.py",         "tabnanny.py",
            "cgi.py",               "nntplib.py",       "tarfile.py",
            "cgitb.py",             "nturl2path.py",    "telnetlib.py",
            "chunk.py",             "numbers.py",       "tempfile.py",
            "cmd.py",               "optparse.py",      "textwrap.py",
            "code.py",              "pathlib.py",       "this.py",
            "codeop.py",            "pdb.py",           "timeit.py",
            "colorsys.py",          "pickle.py",        "trace.py",
            "compileall.py",        "pickletools.py",   "tracemalloc.py",
            "configparser.py",      "pipes.py",         "tty.py",
            "contextvars.py",       "plistlib.py",      "turtle.py",
            "cProfile.py",          "poplib.py",        "typing.py",
            "crypt.py",             "pprint.py",        "uu.py",
            "csv.py",               "profile.py",       "uuid.py",
            "dataclasses.py",       "pstats.py",        "wave.py",
            "datetime.py",          "pty.py",           "webbrowser.py",
            "decimal.py",           "pyclbr.py",        "xdrlib.py",
            "difflib.py",           "py_compile.py",    "zipapp.py",
            "doctest.py",           "queue.py",         "zipfile.py",
            "dummy_threading.py",   "quopri.py",        "zipimport.py",
            "filecmp.py",           "random.py",        "_compat_pickle.py",
            "fileinput.py",         "rlcompleter.py",   "_compression.py",
            "formatter.py",         "runpy.py",         "_dummy_thread.py",
            "fractions.py",         "sched.py",         "_markupbase.py",
            "ftplib.py",            "secrets.py",       "_osx_support.py",
            "getopt.py",            "selectors.py",     "_pydecimal.py",
            "getpass.py",           "shelve.py",        "_pyio.py",
            "gettext.py",           "shlex.py",         "_py_abc.py",
            "gzip.py",              "shutil.py",        "_strptime.py",
            "hashlib.py",           "smtpd.py",         "_threading_local.py",
            "hmac.py",              "smtplib.py",       "__future__.py",
            "imaplib.py",           "sndhdr.py",        "__phello__.foo.py",
        ]
        fileTags: ["python-lib-files"]
    }

    Rule {
        inputs: ["python-lib-files"]
        Artifact {
            filePath: {
                var rel = FileInfo.relativePath(FileInfo.joinPaths(product.pythonInstallDir, "Lib"), input.filePath);
                return FileInfo.joinPaths("python-lib", rel);
            }
            fileTags: ["python-uncompiled"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = input.filePath + "->" + output.filePath;
            cmd.highlight = "codegen";
            cmd.sourceCode = function() {
                File.copy(input.filePath, output.filePath);
            }
            return [cmd];
        }
    }

    Rule {
        multiplex: true
        inputs: ["python-uncompiled"]
        outputFileTags: ["python-compiled"]
        outputArtifacts: {
            var r = [];
            for (var i = 0; i < inputs["python-uncompiled"].length; ++i) {
                var input = inputs["python-uncompiled"][i];
                r.push({
                    filePath: input.filePath + 'c',
                    fileTags: ["python-compiled"],
                });
            }
            return r;
        }
        prepare: {
            var args = ["-OO", "-m", "compileall", "-b", FileInfo.joinPaths(product.buildDirectory, "python-lib")];
            var cmd = new Command(FileInfo.joinPaths(product.pythonInstallDir, "python.exe"), args);
            cmd.description = "Compile Python modules";
            return [cmd];
        }
    }

    Rule {
        multiplex: true
        inputs: ["python-compiled"]
        Artifact {
            filePath: "python-libs.txt"
            fileTags: ["archiver.input-list"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "generate";
            cmd.highlight = "codegen";
            cmd.sourceCode = function() {
                var content = "";
                for (var i = 0; i < inputs["python-compiled"].length; ++i) {
                    var input = inputs["python-compiled"][i];
                    var rel = FileInfo.relativePath(FileInfo.joinPaths(product.buildDirectory, "python-lib"), input.filePath);
                    content += rel + '\n';
                }
                file = new TextFile(output.filePath, TextFile.WriteOnly);
                file.write(content);
                file.close();
            }
            return [cmd];
        }
    }

    Depends { name: "archiver" }
    type: base.concat("archiver.archive")
    installTags: base.concat("archiver.archive")
    archiver.type: "zip"
    archiver.workingDirectory: FileInfo.joinPaths(buildDirectory, "python-lib")
    archiver.archiveBaseName: pythonDllProbe.fileNamePrefix
}
