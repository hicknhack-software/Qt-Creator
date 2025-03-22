

QtcLibrary {
      type: ["staticlibrary"]
      name: "qtcBZip2"

      cpp.includePaths: [
            product.sourceDirectory,
      ]

      Export {
            Depends { name: "cpp" }
            cpp.includePaths: [
                  exportingProduct.sourceDirectory,
            ]
      }

      files: [
            "bzlib.h",
            "blocksort.c",
            "huffman.c",
            "crctable.c",
            "randtable.c",
            "compress.c",
            "decompress.c",
            "bzlib.c",
      ]
}
