#!/usr/bin/env python3

import json, argparse

def recursive_sort_dict(d):
    if isinstance(d, dict):
        # Recursively sort dictionary values
        sorted_dict = {k: recursive_sort_dict(v) for k, v in d.items()}
        # Sort dictionary items by key
        return dict(sorted(sorted_dict.items()))
    elif isinstance(d, list):
        return sorted(d, key=lambda x: x[0])
    

    else:
        # Return the item as-is if it's neither a dict nor a list
        return d


def get_tuple(dict):
    items = []
    for key, item in dict.items():
        items.append((key,dict[key]['type']))
    return items

def compare_branches(tree1, tree2, path=""):
    """
    Compare branches between two JSON objects recursively and
    return the differences for branches present or missing.
    """
    objects = []
    for obj1 in tree1.keys():
        if obj1 not in tree2.keys():
            print(f"Object {obj1} is not in second file")
            continue
        objects.append(obj1)
    
    for obj2 in tree2.keys():
        if obj2 not in tree1.keys():
            print(f"Object {obj1} is not in first file")
            continue
        objects.append(obj2)
    objects = set(objects)
    
    branch_dict = {}

    for obj in objects:
        tree1_set = set(get_tuple(tree1[obj]))
        tree2_set = set(get_tuple(tree2[obj]))
        intersection = (tree1_set & tree2_set)
        only_tree1 = tree1_set - tree2_set
        only_tree2 = tree2_set - tree1_set
        # change them to list
        intersection = list(intersection)
        only_tree1 = list(only_tree1)
        only_tree2 = list(only_tree2)
        branch_dict[obj] = {'file_1_only':only_tree1,'file_2_only':only_tree2,'intersection':intersection}
    return branch_dict
        

            


def compare_json_files(file1, file2):
    """
    Compare the two JSON files at the object level and branch level.
    For each object, check which branches are missing or present in each file.
    """
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        data1 = json.load(f1)
        data2 = json.load(f2)
    

    # Compare Events Content
    print("\nComparing 'Events Content':")
    if 'Events Content' in data1 and 'Events Content' in data2:
        events_diff = compare_branches(data1['Events Content'], data2['Events Content'])
        events_diff = recursive_sort_dict(events_diff)
        #events_diff = sorted(events_diff, key=lambda diff: ('missing in the second file' in diff, 'missing in the first file' in diff))
    for key, item in events_diff.items():
        print(f'\n//{key}----------------------------')
        for branch_tup in item['intersection']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            print(f"RVec<{branch_type}> {branch_name};")
        print(f'//Run3')
        for branch_tup in item['file_2_only']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            print(f"RVec<{branch_type}> {branch_name};") 
        print(f'//Run2')
        for branch_tup in item['file_1_only']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            if any(t[0] == branch_name for t in item['file_2_only']): 
                print(f"RVec<{branch_type}> {branch_name}_RunII;") 
            else:
                print(f"RVec<{branch_type}> {branch_name};")

    
        print(f'\n//{key}----------------------------')
        for branch_tup in item['intersection']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            print(f'fChain->SetBranchAddress("{branch_name}", {branch_name}.data());')

        print(f'//Run3')
        for branch_tup in item['file_2_only']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            print(f'fChain->SetBranchAddress("{branch_name}", {branch_name}.data());')
        print(f'//Run2')
        for branch_tup in item['file_1_only']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            if any(t[0] == branch_name for t in item['file_2_only']): 
                print(f'fChain->SetBranchAddress("{branch_name}", {branch_name}_RunII.data());')
            else:
                print(f'fChain->SetBranchAddress("{branch_name}", {branch_name}.data());')

        print(f'\n//{key}----------------------------')
        for branch_tup in item['intersection']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            print(f'{branch_name}.resize(kMax{key});')

        print(f'//Run3')
        for branch_tup in item['file_2_only']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            print(f'{branch_name}.resize(kMax{key});')
        for branch_tup in item['file_1_only']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            if any(t[0] == branch_name for t in item['file_2_only']): 
                print(f'{branch_name}_RunII.resize(0);')
            else:
                print(f'{branch_name}.resize(0);')
        print(f'//Run2')
        for branch_tup in item['file_2_only']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            print(f'{branch_name}.resize(0);')
        for branch_tup in item['file_1_only']:
            branch_type = branch_tup[1]
            branch_name = branch_tup[0]
            if any(t[0] == branch_name for t in item['file_2_only']): 
                print(f'{branch_name}_RunII.resize(kMax{key});')
            else:
                print(f'{branch_name}.resize(kMax{key});')
    else:
        print("'Events Content' is missing in one of the files.")

            # Compare Runs Content
    print("Comparing 'Runs Content':")
    if 'Runs Content' in data1 and 'Runs Content' in data2:
        runs_diff = compare_branches(data1['Runs Content'], data2['Runs Content'])
        runs_diff = sorted(runs_diff, key=lambda diff: ('missing in the second file' in diff, 'missing in the first file' in diff))
        for diff in runs_diff:
            print(diff)
    else:
        print("'Runs Content' is missing in one of the files.")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Compare two JSON files")
    parser.add_argument("--input1", dest='file1', help="First JSON file to compare")
    parser.add_argument("--input2", dest='file2', help="Second JSON file to compare")
    args = parser.parse_args()

    file1 = args.file1
    file2 = args.file2
    compare_json_files(file1, file2)
