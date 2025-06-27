SQL_FILES =                                                                    \
    %reldir%/count_tables.sql                                                  \
    %reldir%/create_prefix_outage_table.sql                                    \
    %reldir%/create_as_outage_table.sql                                        \
    %reldir%/create_country_outage_table.sql                                   \
    %reldir%/insert_prefix_outage_event.sql                                    \
    %reldir%/insert_as_outage_event.sql                                        \
    %reldir%/update_prefix_outage_end.sql                                      \
    ${NOTHING}

SQL_GENERATED_FILES = $(patsubst %,%.inc,$(SQL_FILES))
BUILT_SOURCES += ${SQL_GENERATED_FILES}

EXTRA_DIST += ${SQL_FILES}

${SQL_GENERATED_FILES}: %.inc: %
	xxd -i < $< > $@

clean-local: clean-sql-files

.PHONY: clean-sql-files
clean-sql-files:
	$(RM) ${SQL_GENERATED_FILES}
