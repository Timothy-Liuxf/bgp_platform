noinst_HEADERS +=                                                              \
    %reldir%/outage_data_generator.hpp                                         \
    ${NOTHING}

noinst_LIBRARIES += build/lib/libdatacvt.a
build_lib_libdatacvt_a_SOURCES =                                               \
    %reldir%/outage_data_generator.cc                                          \
    ${NOTHING}
