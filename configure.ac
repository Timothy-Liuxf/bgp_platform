#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([bgp_platform], [1.0.0])
AC_CONFIG_SRCDIR([third_party/jsoncons/third_party/catch/catch.hpp])
AC_CONFIG_MACRO_DIRS([m4 third_party/autoconf-archive/m4])
AC_PROG_RANLIB

AM_INIT_AUTOMAKE([foreign subdir-objects])

AX_CXX_COMPILE_STDCXX_17([noext], [mandatory])
AX_APPEND_COMPILE_FLAGS([-Wall -Wpedantic -Wextra -Werror -O2], [CXXFLAGS])
AX_APPEND_COMPILE_FLAGS([-Isrc -Ithird_party/jsoncons/include], [CXXFLAGS])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CPP

# Checks for libraries.

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

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
