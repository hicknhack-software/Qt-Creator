import qbs.File
import qbs.FileInfo
import qbs.TextFile
import qbs.Utilities

Module {
    Rule {
        name: "QmldirBuilder"

        multiplex: true
        inputs: ["qt.qml.qml"]
        outputFileTags: ["qmldir_generated", "qt.core.resource_data"]
        outputArtifacts: [{
            filePath: product.buildDirectory + "/qmldir",
            fileTags: ["qmldir_generated", "qt.core.resource_data"],
            Qt: { core: { resourcePrefix: "" + inputs["qt.qml.qml"][0].Qt.core.resourcePrefix } }
        }]

        prepare: {
            var content = [
              ["module", product.Qt.qml.importName].join(" "),
              ["prefer", ":" + inputs["qt.qml.qml"][0].Qt.core.resourcePrefix.replace(/^\/?/, "/").replace(/\/?$/, "/")].join(" "),
            ];
            for (var i = 0; i < inputs["qt.qml.qml"].length; ++i) {
                var input = inputs["qt.qml.qml"][i];
                content.push([input.baseName, product.Qt.qml.importVersion, input.fileName].join(" "));
            }

            var cmd = new JavaScriptCommand();
            cmd.description = "generating qmldir for " + product.Qt.qml.importName;
            cmd.highlight = 'codegen';
            cmd.content = content.join("\n");
            cmd.sourceCode = function() {
              var file = new TextFile(output.filePath, TextFile.WriteOnly);
                file.truncate();
                file.write(content);
                file.close();
            }
            return cmd;
        }
    }
}
