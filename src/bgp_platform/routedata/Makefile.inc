noinst_HEADERS +=                                                              \
    %reldir%/init_info.hpp                                                     \
    %reldir%/route_info.hpp                                                    \
    %reldir%/routedata.hpp                                                     \
    ${NOTHING}

noinst_LIBRARIES += build/lib/libroutedata.a
build_lib_libroutedata_a_SOURCES =                                             \
    %reldir%/init_info.cc                                                      \
    %reldir%/routedata.cc                                                      \
    ${NOTHING}
