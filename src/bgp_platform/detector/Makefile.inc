noinst_HEADERS +=                                                              \
    %reldir%/detector.hpp                                                      \
    %reldir%/file_watcher.hpp                                                  \
    %reldir%/outage_event.hpp                                                  \
    ${NOTHING}

noinst_LIBRARIES += build/lib/libdetector.a
build_lib_libdetector_a_SOURCES =                                              \
    %reldir%/detector.cc                                                       \
    %reldir%/file_watcher.cc                                                   \
    ${NOTHING}
