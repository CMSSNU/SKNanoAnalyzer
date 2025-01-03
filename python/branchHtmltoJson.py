#!/usr/bin/env python3
# This script parsing the HTML file of NanoAOD branch content and convert it to JSON format
# download the html file like https://cms-nanoaod-integration.web.cern.ch/autoDoc/NanoAODv12/2022/2023/doc_DYJetsToLL_M-50_TuneCP5_13p6TeV-madgraphMLM-pythia8_Run3Summer22NanoAODv12-130X_mcRun3_2022_realistic_v5-v2.html,
# and nominate this file as input to this script
# Usage: python branchHtmltoJson.py --input <input_html_file> --output <output_json_file>

import json
from bs4 import BeautifulSoup
import argparse
import re

def recursive_sort_dict(d):
    for k, v in d.items():
        if isinstance(v, dict):
            d[k] = recursive_sort_dict(v)
    return dict(sorted(d.items()))

# Function to extract content and build dictionary
def extract_content(content_type, html_file):
    # Read the HTML file
    with open(html_file, 'r') as file:
        html_content = file.read()
    soup = BeautifulSoup(html_content, 'html.parser')
    # Find the content table
    content_table = soup.find('h1', string=f'{content_type} Content').find_next('table')
    content_items = content_table.find_all('a')
    content_items = [item.text for item in content_items]

    # Create a dictionary for the content
    content_dict = {}
    h2_list = soup.find_all('h2')

    for key in content_items:
        # Find the corresponding h2 and table
        h2_for_key = next(h2 for h2 in h2_list if h2.a.get('name') == key)
        table_for_key = h2_for_key.find_next('table')

        # Extract table rows and th elements
        branch_dict = {}
        ths = table_for_key.find_all('th')[3:]
        for th in ths:
            branch_dict[th.text] = {}
            tds = th.find_next_siblings('td', limit=2)
            branch_dict[th.text]['type'] = re.sub(r"\(.*?\)", "", tds[0].text).strip()
            branch_dict[th.text]['description'] = tds[1].text

        content_dict[key] = branch_dict
    content_dict = recursive_sort_dict(content_dict)
    return content_dict

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', help='Input HTML file', required=True)
    parser.add_argument('--output', help='Output JSON file', required=True)
    args = parser.parse_args()

    final_dict = {
    'Runs Content': extract_content('Runs', args.input),
    'Events Content': extract_content('Events', args.input),
    }

    with open(args.output, 'w') as file:
        json.dump(final_dict, file, indent=4)

    

