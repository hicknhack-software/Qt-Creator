#pragma once

#include <qglobal.h>

#if defined(LIBGIT2CPP_BUILD_LIB)
#define LIBGIT2CPP_EXPORT Q_DECL_EXPORT
#elif defined(LIBGIT2CPP_BUILD_STATIC_LIB)
#define LIBGIT2CPP_EXPORT
#else
#define LIBGIT2CPP_EXPORT Q_DECL_IMPORT
#endif
