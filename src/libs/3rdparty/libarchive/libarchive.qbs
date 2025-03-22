import qbs.Utilities

QtcLibrary {
    type: ["staticlibrary"]
    name: "qtcLibarchive"

    property string config_header: {
        if (qbs.targetPlatform.includes("macos")) {
            return "config_mac.h";
        }
        if (qbs.toolchain.includes("mingw")) {
            return "config_mingw.h";
        }
        if (qbs.targetPlatform.includes("windows")) {
            return "config_win.h";
        }
        return "config_unix.h";
    }

    Depends { name: "qtcXz" }
    Depends { name: "qtcZlib" }
    Depends { name: "qtcBZip2" }

    cpp.includePaths: [
        product.sourceDirectory + "/build/qbs",
        product.sourceDirectory + "/libarchive",
    ]
    cpp.defines: [
        "LIBARCHIVE_STATIC",
        "HAVE_CONFIG_H",
        "PLATFORM_CONFIG_H=" + Utilities.cStringQuote(product.sourceDirectory + "/" + product.config_header),
        "HAVE_ZLIB_H=1", "HAVE_LIBZ=1",
        "HAVE_BZLIB_H=1", "HAVE_LIBBZ2=1", "USE_BZIP2_STATIC",
        "HAVE_LZMA_H=1", "HAVE_LIBLZMA=1", "LZMA_API_STATIC",
    ]

    Export {
        Depends { name: "cpp" }
        Depends { name: "qtcXz" }
        Depends { name: "qtcZlib" }
        Depends { name: "qtcBZip2" }

        cpp.includePaths: [
            exportingProduct.sourceDirectory + "/libarchive",
        ]
        cpp.defines: [
            "LIBARCHIVE_STATIC",
        ]
    }

    Group {
        name: "config"
        files: [
            "config_mingw.h",
            "config_win.h",
            "config_mac.h",
            "config_unix.h",
        ]
    }

    files: [
        "libarchive/archive_acl.c",
        "libarchive/archive_acl_private.h",
        "libarchive/archive_check_magic.c",
        "libarchive/archive_cmdline.c",
        "libarchive/archive_cmdline_private.h",
        "libarchive/archive_crc32.h",
        "libarchive/archive_cryptor.c",
        "libarchive/archive_cryptor_private.h",
        "libarchive/archive_digest.c",
        "libarchive/archive_digest_private.h",
        "libarchive/archive_endian.h",
        "libarchive/archive_entry.c",
        "libarchive/archive_entry.h",
        "libarchive/archive_entry_copy_stat.c",
        "libarchive/archive_entry_link_resolver.c",
        "libarchive/archive_entry_locale.h",
        "libarchive/archive_entry_private.h",
        "libarchive/archive_entry_sparse.c",
        "libarchive/archive_entry_stat.c",
        "libarchive/archive_entry_strmode.c",
        "libarchive/archive_entry_xattr.c",
        "libarchive/archive_getdate.c",
        "libarchive/archive_getdate.h",
        "libarchive/archive_hmac.c",
        "libarchive/archive_hmac_private.h",
        "libarchive/archive_match.c",
        "libarchive/archive_openssl_evp_private.h",
        "libarchive/archive_openssl_hmac_private.h",
        "libarchive/archive_options.c",
        "libarchive/archive_options_private.h",
        "libarchive/archive_pack_dev.h",
        "libarchive/archive_pack_dev.c",
        "libarchive/archive_pathmatch.c",
        "libarchive/archive_pathmatch.h",
        "libarchive/archive_platform.h",
        "libarchive/archive_platform_acl.h",
        "libarchive/archive_platform_xattr.h",
        "libarchive/archive_ppmd_private.h",
        "libarchive/archive_ppmd8.c",
        "libarchive/archive_ppmd8_private.h",
        "libarchive/archive_ppmd7.c",
        "libarchive/archive_ppmd7_private.h",
        "libarchive/archive_private.h",
        "libarchive/archive_random.c",
        "libarchive/archive_random_private.h",
        "libarchive/archive_rb.c",
        "libarchive/archive_rb.h",
        "libarchive/archive_read.c",
        "libarchive/archive_read_add_passphrase.c",
        "libarchive/archive_read_append_filter.c",
        "libarchive/archive_read_data_into_fd.c",
        "libarchive/archive_read_disk_entry_from_file.c",
        "libarchive/archive_read_disk_posix.c",
        "libarchive/archive_read_disk_private.h",
        "libarchive/archive_read_disk_set_standard_lookup.c",
        "libarchive/archive_read_extract.c",
        "libarchive/archive_read_extract2.c",
        "libarchive/archive_read_open_fd.c",
        "libarchive/archive_read_open_file.c",
        "libarchive/archive_read_open_filename.c",
        "libarchive/archive_read_open_memory.c",
        "libarchive/archive_read_private.h",
        "libarchive/archive_read_set_format.c",
        "libarchive/archive_read_set_options.c",
        "libarchive/archive_read_support_filter_all.c",
        "libarchive/archive_read_support_filter_by_code.c",
        "libarchive/archive_read_support_filter_bzip2.c",
        "libarchive/archive_read_support_filter_compress.c",
        "libarchive/archive_read_support_filter_gzip.c",
        "libarchive/archive_read_support_filter_grzip.c",
        "libarchive/archive_read_support_filter_lrzip.c",
        "libarchive/archive_read_support_filter_lz4.c",
        "libarchive/archive_read_support_filter_lzop.c",
        "libarchive/archive_read_support_filter_none.c",
        "libarchive/archive_read_support_filter_program.c",
        "libarchive/archive_read_support_filter_rpm.c",
        "libarchive/archive_read_support_filter_uu.c",
        "libarchive/archive_read_support_filter_xz.c",
        "libarchive/archive_read_support_filter_zstd.c",
        "libarchive/archive_read_support_format_7zip.c",
        "libarchive/archive_read_support_format_all.c",
        "libarchive/archive_read_support_format_ar.c",
        "libarchive/archive_read_support_format_by_code.c",
        "libarchive/archive_read_support_format_cab.c",
        "libarchive/archive_read_support_format_cpio.c",
        "libarchive/archive_read_support_format_empty.c",
        "libarchive/archive_read_support_format_iso9660.c",
        "libarchive/archive_read_support_format_lha.c",
        "libarchive/archive_read_support_format_mtree.c",
        "libarchive/archive_read_support_format_rar5.c",
        "libarchive/archive_read_support_format_raw.c",
        "libarchive/archive_read_support_format_tar.c",
        "libarchive/archive_read_support_format_warc.c",
        "libarchive/archive_read_support_format_zip.c",
        "libarchive/archive_string.c",
        "libarchive/archive_string.h",
        "libarchive/archive_string_composition.h",
        "libarchive/archive_string_sprintf.c",
        "libarchive/archive_util.c",
        "libarchive/archive_version_details.c",
        "libarchive/archive_virtual.c",
        "libarchive/archive_write.c",
        "libarchive/archive_write_disk_posix.c",
        "libarchive/archive_write_disk_private.h",
        "libarchive/archive_write_disk_set_standard_lookup.c",
        "libarchive/archive_write_private.h",
        "libarchive/archive_write_open_fd.c",
        "libarchive/archive_write_open_file.c",
        "libarchive/archive_write_open_filename.c",
        "libarchive/archive_write_open_memory.c",
        "libarchive/archive_write_add_filter.c",
        "libarchive/archive_write_add_filter_b64encode.c",
        "libarchive/archive_write_add_filter_by_name.c",
        "libarchive/archive_write_add_filter_bzip2.c",
        "libarchive/archive_write_add_filter_compress.c",
        "libarchive/archive_write_add_filter_grzip.c",
        "libarchive/archive_write_add_filter_gzip.c",
        "libarchive/archive_write_add_filter_lrzip.c",
        "libarchive/archive_write_add_filter_lz4.c",
        "libarchive/archive_write_add_filter_lzop.c",
        "libarchive/archive_write_add_filter_none.c",
        "libarchive/archive_write_add_filter_program.c",
        "libarchive/archive_write_add_filter_uuencode.c",
        "libarchive/archive_write_add_filter_xz.c",
        "libarchive/archive_write_add_filter_zstd.c",
        "libarchive/archive_write_set_format.c",
        "libarchive/archive_write_set_format_7zip.c",
        "libarchive/archive_write_set_format_ar.c",
        "libarchive/archive_write_set_format_by_name.c",
        "libarchive/archive_write_set_format_cpio.c",
        "libarchive/archive_write_set_format_cpio_binary.c",
        "libarchive/archive_write_set_format_cpio_newc.c",
        "libarchive/archive_write_set_format_cpio_odc.c",
        "libarchive/archive_write_set_format_filter_by_ext.c",
        "libarchive/archive_write_set_format_gnutar.c",
        "libarchive/archive_write_set_format_iso9660.c",
        "libarchive/archive_write_set_format_mtree.c",
        "libarchive/archive_write_set_format_pax.c",
        "libarchive/archive_write_set_format_private.h",
        "libarchive/archive_write_set_format_raw.c",
        "libarchive/archive_write_set_format_shar.c",
        "libarchive/archive_write_set_format_ustar.c",
        "libarchive/archive_write_set_format_v7tar.c",
        "libarchive/archive_write_set_format_warc.c",
        "libarchive/archive_write_set_format_zip.c",
        "libarchive/archive_write_set_options.c",
        "libarchive/archive_write_set_passphrase.c",
        "libarchive/archive_xxhash.h",
        "libarchive/filter_fork_posix.c",
        "libarchive/filter_fork.h",
        "libarchive/xxhash.c",
    ]

    Group {
        name: "blake2"
        files: [
            "libarchive/archive_blake2sp_ref.c",
            "libarchive/archive_blake2s_ref.c",
        ]
    }

    Group {
        name: "windows"
        condition: qbs.hostOS.contains("windows")
        files: [
            "libarchive/archive_entry_copy_bhfi.c",
            "libarchive/archive_read_disk_windows.c",
            "libarchive/archive_windows.c",
            "libarchive/archive_windows.h",
            "libarchive/archive_write_disk_windows.c",
            "libarchive/filter_fork_windows.c",
        ]
    }
}
