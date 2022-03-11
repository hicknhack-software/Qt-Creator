find_package(libgit2)

if (libgit2_FOUND)
  add_qtc_library(LibGit2Cpp
    INCLUDES ${LIBGIT2_INCLUDE_PATH}
    PUBLIC_DEPENDS ${LIBGIT2_LIBRARY_PATH}
    DEPENDS Qt5::Core Qt5::Concurrent Utils
    DEFINES LIBGIT2CPP_BUILD_LIB
    SOURCES
      gitrepo.cpp gitrepo.h libgit2cpp_global.h
  )
endif()
