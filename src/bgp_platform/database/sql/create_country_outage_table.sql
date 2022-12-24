CREATE TABLE if not exists {} (
    country               varchar(100),
    outage_id             int,
    country_chinese_name  text,
    s_time                timestamp(0) without time zone  not NULL,
    e_time                timestamp(0) without time zone,
    duration              interval DAY TO SECOND (0),
    outage_level          varchar(8)  not NULL,
    outage_level_descr    text        not NULL,
    max_outage_as_ratio   decimal(4, 3) not NULL,
    max_outage_as_num     int not NULL,
    total_as_num          int not NULL,
    outage_ases           jsonb,
    primary key(country, outage_id)
);
