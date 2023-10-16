noinst_HEADERS +=                                                              \
    %reldir%/connector.hpp                                                     \
    %reldir%/database.hpp                                                      \
    ${NOTHING}

noinst_LIBRARIES += build/lib/libdatabase.a
build_lib_libdatabase_a_SOURCES =                                              \
    %reldir%/connector.cc                                                      \
    %reldir%/database.cc                                                       \
    ${NOTHING}

include %reldir%/models/Makefile.inc
include %reldir%/sql/Makefile.inc
