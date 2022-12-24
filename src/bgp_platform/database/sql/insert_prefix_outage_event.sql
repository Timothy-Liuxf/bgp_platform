INSERT INTO {table_name} (
    asn,
    country,
    as_name,
    org_name,
    as_type,
    s_time,
    e_time,
    duration,
    pre_vp_paths,
    eve_vp_paths,
    outage_level,
    outage_level_descr,
    prefix,
    outage_id
)
VALUES (
    '{asn}', '{country}', '{as_name}', '{org_name}', '{as_type}', '{s_time:%Y-%m-%d %H:%M:%S}', '{e_time:%Y-%m-%d %H:%M:%S}',
    '{duration}', '{pre_vp_paths}', '{eve_vp_paths}', '{outage_level}', '{outage_level_descr}', 
    '{prefix}', '{outage_id}'
);
