import qbs.FileInfo

Module {
    property stringList extraArgs: []

    FileTagger {
        patterns: ["*.comp", "*.frag", "*.vert"]
        fileTags: ["qsb_src"]
    }

    Rule {
        name: "QSB"

        inputs: ["qsb_src"]
        outputFileTags: ["qsb_compiled", "qt.core.resource_data"]
        outputArtifacts: [{
            filePath: product.buildDirectory + '/qsb/' + input.fileName + ".qsb",
            fileTags: ["qsb_compiled", "qt.core.resource_data"],
            Qt: { core: { resourcePrefix: "" + input.Qt.core.resourcePrefix } }
        }]

        prepare: {
            var tool = FileInfo.joinPaths(product.Qt.core.binPath, "qsb");
            var args = input.qsb_compiler.extraArgs.concat(["-o", output.filePath, input.filePath]);
            var cmd = new Command(tool, args);
            cmd.description = "qsb " + input.fileName;
            cmd.highlight = 'codegen';
            return cmd;
        }
    }
}
