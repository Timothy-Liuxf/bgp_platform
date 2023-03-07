UPDATE {table_name} SET
    e_time      = {e_time},
    duration    = {duration}
where prefix = {prefix} and outage_id = {outage_id} and asn={asn};
