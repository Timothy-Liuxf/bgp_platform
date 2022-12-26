AC_DEFUN([AX_GENERATE_SQL], [
    AC_MSG_NOTICE([Generating SQL files...])
    sql_dirs="src/bgp_platform/database/sql"
    for sql_dir in $sql_dirs
    do
        pushd $sql_dir
        sql_files=`find . -iname "*.sql"`
        for sql_file in $sql_files
        do
            AC_CHECK_FILE([$sql_file.inc], [], [
                AC_MSG_NOTICE([Generating $sql_file...])
                AS_IF([xxd -i < $sql_file > $sql_file.inc], [], [
                    AC_MSG_ERROR([Failed to generate $sql_file.inc!])
                ])
            ])
        done
        popd
    done
])
