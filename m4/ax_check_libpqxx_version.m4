AC_DEFUN([AX_CHECK_LIBPQXX_VERSION], [
    AC_MSG_CHECKING([for libpqxx version])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([
            AC_LANG_PROGRAM([[
                #include <pqxx/version.hxx>
            ]], [[
                #if (PQXX_VERSION_MAJOR != 6) || (PQXX_VERSION_MINOR != 4)
                #error "libpqxx version 6.4 is required"
                #endif
            ]])
        ],
        [AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])
        AC_MSG_ERROR([libpqxx version 6.4 is required])])
    AC_LANG_POP([C++])
])
