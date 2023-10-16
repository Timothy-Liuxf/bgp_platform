include %reldir%/utils/Makefile.inc
include %reldir%/common/Makefile.inc
include %reldir%/datacvt/Makefile.inc
include %reldir%/routedata/Makefile.inc
include %reldir%/detector/Makefile.inc
include %reldir%/database/Makefile.inc

bin_PROGRAMS += build/bin/bgp_platform
build_bin_bgp_platform_SOURCES = %reldir%/main.cc
build_bin_bgp_platform_LDADD =                                                 \
    build/lib/libdatacvt.a                                                     \
    build/lib/libdetector.a                                                    \
    build/lib/libdatabase.a                                                    \
    build/lib/libroutedata.a                                                   \
    -lstdc++fs                                                                 \
    -Lthird_party/inotify-cpp/build/src -linotify-cpp                          \
    ${NOTHING}

build_bin_bgp_platform_LDFLAGS =                                               \
    -Wl,-Rthird_party/inotify-cpp/build/src                                    \
    ${NOTHING}
