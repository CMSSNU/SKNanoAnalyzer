#!/usr/bin/env python3
import os
import sys
import re
from SKFlat import setParser, getEraList, getInputSampleList, jobFileDivider
from SKFlat import sampleInfoJsons, SKNANO_DATA
import json
import itertools
import ROOT
GetEffLumi_Foler = ""

def splitDict(file_tuples, target_entry):
    split_dict = {}
    for tuple in file_tuples:
        file = tuple[1]
        entry = tuple[2]
        split_dict[file] = {}
        split_dict[file]['entry'] = entry
        nSplit = entry // target_entry + 1
        split_dict[file]['nSplit'] = nSplit
    total_nSplit = 0
    
 
    for file in split_dict.keys():
        split_dict[file]['target_split'] = []
        for i in range(split_dict[file]['nSplit']):
            total_nSplit += 1
            remaining_entry = split_dict[file]['entry']
            new_name = f"tree_{total_nSplit}.root"
            this_entry = target_entry if remaining_entry >= target_entry else remaining_entry
            remaining_entry -= this_entry
            new_tuple = (new_name, this_entry)
            split_dict[file]['target_split'].append(new_tuple)
            
    return split_dict
            
        
        

def GenEntryDirect(era, sample):
    sampleInfo = sampleInfoJsons[era]
    samplePaths = json.load(open(os.path.join(SKNANO_DATA,era,'Sample','ForSNU',sample+'.json')))['path']
    this_folder = os.path.join(GetEffLumi_Foler, era, sample, 'output')
    if not os.path.exists:
        print(f"\033[91mFolder does not exist for {sample}, {era}\033[00m")
        return
    histlist = os.listdir(this_folder)
    histlist = [x for x in histlist if x.endswith('.root')]
    histlist = [os.path.join(this_folder, x) for x in histlist]
    
    if len(histlist) != len(samplePaths):
        print(f"\033[91mNumber of files in {this_folder} does not match with the number of files in {samplePaths}\033[00m")
        print(f"\033[91m You have to Run SKFlat.py -a GetEffLumi -n -1 first. \033[00m")
        return
    file_tuples = []
    for file in samplePaths:
        fileName = file.split('/')[-1]
        fileNum = re.search(r'\d+', fileName).group()
        fileNum = int(fileNum) 
        f = ROOT.TFile(file,"READ")
        entry = f.Get("Events").GetEntries()
        file_tuples.append((fileNum, file, entry))
        f.Close()
    return file_tuples
        
def makeNEntryPerFileDict(era, sample):
    sampleInfo = sampleInfoJsons[era]
    samplePaths = json.load(open(os.path.join(SKNANO_DATA,era,'Sample','ForSNU',sample+'.json')))['path']
    this_folder = os.path.join(GetEffLumi_Foler, era, sample, 'output')
    if not os.path.exists:
        print(f"\033[91mFolder does not exist for {sample}, {era}\033[00m")
        return
    histlist = os.listdir(this_folder)
    histlist = [x for x in histlist if x.endswith('.root')]
    histlist = [os.path.join(this_folder, x) for x in histlist]
    
    if len(histlist) != len(samplePaths):
        print(f"\033[91mNumber of files in {this_folder} does not match with the number of files in {samplePaths}\033[00m")
        print(f"\033[91m You have to Run SKFlat.py -a GetEffLumi -n -1 first. \033[00m")
        return
    file_tuples = []
    hist_tuples = []
    tuples = []
    for file in samplePaths:
        fileName = file.split('/')[-1]
        fileNum = re.search(r'\d+', fileName).group()
        fileNum = int(fileNum) 
        file_tuples.append((fileNum, file))
    for hist in histlist:
        histName = hist.split('/')[-1]
        histNum = re.search(r'\d+', histName).group()
        histNum = int(histNum)
        hist_tuples.append((histNum, hist))
    file_tuples.sort()
    hist_tuples.sort()
    for file, hist in zip(file_tuples, hist_tuples):
        if file[0]-1 != hist[0]:
            print(f"\033[91mFile number does not match between {file[1]} and {hist[1]}\033[00m")
            return
        tuples.append((file[1], hist[1]))
    return tuples
    
        

if __name__ == '__main__':
    parser = setParser()
    parser.add_argument('--EffLumiLogFolder', dest='EffLumiLogFolder', default='GetEffLumi', help='Folder name of GetEffLumi log files')
    args = parser.parse_args()
    GetEffLumi_Foler = args.EffLumiLogFolder
    GetEffLumi_Foler = '/data6/Users/yeonjoon/SKNanoRunlog/2024_09_23_004321_GetEffLumi'
    eras = getEraList(args.Era, args.Run)
    samples = getInputSampleList(args.InputSample, eras)
    for era, sample in itertools.product(eras, samples):
        isDATA = not sampleInfoJsons[era][sample]['isMC']
        #makeNEntryPerFileDict(era, sample, isDATA)
        file_tuples = GenEntryDirect(era, sample)
        split_dict = splitDict(file_tuples, 100000)
        print(split_dict)
        
        
