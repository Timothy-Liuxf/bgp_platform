CREATE TABLE if not exists {} (
    prefix varchar(60),
    outage_id int,
    asn text,
    s_time timestamp(0) without time zone not NULL,
    e_time timestamp(0) without time zone,
    duration interval DAY TO SECOND (0),
    outage_level varchar(8) not NULL,
    outage_level_descr text not NULL,
    country text,
    as_name text,
    org_name text,
    as_type text,
    pre_vp_paths jsonb,
    eve_vp_paths jsonb,
    primary key(prefix, outage_id, asn)
);
