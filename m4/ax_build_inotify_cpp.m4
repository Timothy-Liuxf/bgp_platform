AC_DEFUN([AX_BUILD_INOTIFY_CPP], [
    AC_CHECK_FILE([third_party/inotify-cpp/build/src/libinotify-cpp.so], [], [
        AC_MSG_NOTICE([Building libinotify-cpp... ])
        libinotify_cpp_build_dir=third_party/inotify-cpp/build                        && \
        mkdir -p $libinotify_cpp_build_dir                                            && \
        pushd $libinotify_cpp_build_dir                                               && \
        cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 ..                   && \
        cmake --build .                                                               && \
        popd
        AS_IF([test $? -eq 0], [
            AC_MSG_NOTICE([Successfully to build libinotify-cpp!])
        ], [
            AC_MSG_ERROR([Failed to build libinotify-cpp!])
        ])
    ])
])
