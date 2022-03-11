# Try to locate libgit2 library

# The ``LIBGIT2_INSTALL_DIR`` (CMake or Environment) variable should be used
# to pinpoint the libgit2 installation

find_path(LIBGIT2_INCLUDE_PATH
    NAMES git2.h
    PATH_SUFFIXES include
    HINTS
        "${LIBGIT2_INSTALL_DIR}" ENV LIBGIT2_INSTALL_DIR "${CMAKE_PREFIX_PATH}"
)

find_library(LIBGIT2_LIBRARY_PATH
    NAMES git2
    PATH_SUFFIXES lib
    HINTS
        "${LIBGIT2_INSTALL_DIR}" ENV LIBGIT2_INSTALL_DIR "${CMAKE_PREFIX_PATH}"
)

find_path(LIBGIT2_DLL_DIR_PATH
    NAMES git2.dll
    PATH_SUFFIXES bin
    HINTS
        "${LIBGIT2_INSTALL_DIR}" ENV LIBGIT2_INSTALL_DIR "${CMAKE_PREFIX_PATH}"
)

file(TO_CMAKE_PATH "${LIBGIT2_DLL_DIR_PATH}/git2.dll" LIBGIT2_DLL_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libgit2
  DEFAULT_MSG
    LIBGIT2_INCLUDE_PATH LIBGIT2_LIBRARY_PATH LIBGIT2_DLL_PATH)

include(FeatureSummary)
set_package_properties(libgit2 PROPERTIES
    URL "https://libgit2.org/"
    DESCRIPTION "pure C implementation of the Git core methods provided as a re-entrant linkable library with a solid API")
