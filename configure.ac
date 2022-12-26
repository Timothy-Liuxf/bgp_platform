#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([bgp_platform], [1.0.0])
AC_CONFIG_SRCDIR([src/bgp_platform/main.cc])
AC_CONFIG_MACRO_DIRS([m4 third_party/autoconf-archive/m4])
AC_PROG_RANLIB

AM_INIT_AUTOMAKE([foreign subdir-objects])

AX_CXX_COMPILE_STDCXX_17([noext], [mandatory])
CXXFLAGS=""

AS_CASE([X$BUILD_CONFIG],
    [XRelease|Xrelease|X], [
        AX_APPEND_COMPILE_FLAGS(
            [-O2 -DBGP_PLATFORM_ENABLE_LOG_INFO],
            [CXXFLAGS]
        )
    ],
    [XDebug|Xdebug], [
        AX_APPEND_COMPILE_FLAGS(
            [-g -Og -DBGP_PLATFORM_ENABLE_LOG_DEBUG],
            [CXXFLAGS]
        )
    ],
    [AC_MSG_ERROR([Unknown build configuration: $BUILD_CONFIG!])])

AX_APPEND_COMPILE_FLAGS([-Wall -Wpedantic -Wextra -Werror], [CXXFLAGS])
AX_APPEND_COMPILE_FLAGS([-pthread], [CXXFLAGS])
AX_APPEND_COMPILE_FLAGS([                                                              dnl
    -Isrc                                                                              dnl
    -Ithird_party/jsoncons/include                                                     dnl
    -Ithird_party/inotify-cpp/src/include                                              dnl
    -Ithird_party/fmt/include -DFMT_HEADER_ONLY                                        dnl
], [CXXFLAGS])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CPP

# Checks for libraries.
AX_CHECK_LIBPQXX_VERSION
AC_CHECK_LIB([pq], [PQconnectdb], [], [AC_MSG_ERROR([libpq not found])])
AC_CHECK_LIB([pqxx], [PQconnectdb], [], [AC_MSG_ERROR([libpqxx not found])])

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([dup2 floor localeconv memmove memset pow select sqrt strtol strtoul])

# Build third party libraries
AX_BUILD_THIRD_PARTY

# Generate sql files
AX_GENERATE_SQL

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
