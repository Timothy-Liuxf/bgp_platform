import requests
import re
import time
import csv
from bs4 import BeautifulSoup

def download_outage_data(out_file_path: str):
    DATA_URL = 'https://bgpstream.crosswork.cisco.com/'
    html = requests.get(DATA_URL).text
    soup = BeautifulSoup(html, 'html.parser')
    table = soup.select('#all_events > tbody')
    if len(table) == 0:
        print('No data found')
        return
    # select tr
    rows = table[0].find_all('tr')
    asn_pattern = r'AS (\d+)'
    outage_info = dict()
    for row in rows:
        # select td
        cols = row.find_all('td')
        if len(cols) != 6:
            continue
        event_type = cols[0].text.strip()
        if event_type == 'Outage':
            asn_col = cols[2]
            start_time_col = cols[3]
            end_time_col = cols[4]
            as_search = re.search(asn_pattern, asn_col.text)
            if as_search is None:
                continue
            asn = as_search.group(1)
            start_time = start_time_col.text.strip()
            end_time = end_time_col.text.strip()
            # convert 'yyyy-mm-dd HH:MM:SS' to timestamp with strptime
            try:
                start_time = time.strptime(start_time, '%Y-%m-%d %H:%M:%S')
                end_time = time.strptime(end_time, '%Y-%m-%d %H:%M:%S')
            except:
                continue
            start_time = time.mktime(start_time) - time.timezone # timestamp in second
            end_time = time.mktime(end_time) - time.timezone
            if asn not in outage_info:
                outage_info[asn] = list()
            outage_info[asn].append((start_time, end_time))
    
    # merge time range
    for (asn, time_list) in outage_info.items():
        time_list.sort(key=lambda x: x[0])
        merged_time_list = list()
        merged_time_list.append(time_list[0])
        for time_range in time_list[1:]:
            if time_range[0] <= merged_time_list[-1][1]:
                merged_time_list[-1] = (merged_time_list[-1][0], max(merged_time_list[-1][1], time_range[1]))
            else:
                merged_time_list.append(time_range)
        outage_info[asn] = merged_time_list

    # write to csv
    with open(out_file_path, 'w') as f:
        writer = csv.writer(f)
        writer.writerow(['asn', 'start_time', 'end_time'])
        for (asn, time_list) in outage_info.items():
            for time_range in time_list:
                writer.writerow([asn, time_range[0], time_range[1]])
