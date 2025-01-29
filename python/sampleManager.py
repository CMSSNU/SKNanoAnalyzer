#!/usr/bin/env python3
import os
import json
import argparse
#This script is used to generate the path information for the sample data
#Sample information is stored in the CommonSampleInfo.json
#This script will generate the path information for the sample data
basePath = os.environ['SKNANO_RUN3_NANOAODPATH'] 
    
def loadCommonSampleInfo(era):
    sampleInfoJson = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','CommonSampleInfo.json')
    sampleInfos = {} 
    try:
        sampleInfos = json.load(open(sampleInfoJson))
    except Exception as e:
        print(f'Error: on {era}')
        print(e)
    return sampleInfos

# def fillSamplePath(era):
#     sampleInfos = loadCommonSampleInfo(era)
#     for alias,sampleInfo in sampleInfos.items():
        # if sampleInfo['isMC']:
        #     path = os.path.join(basePath,era,'MC',sampleInfo['PD'])
        #     filePaths = []
        #     #Folder structure is not fixed yet, so let's do the recursive search until the .root file appears, and save all absolute paths
        #     for root, dirs, files in os.walk(path):
        #         for file in files:
        #             if file.endswith('.root'):
        #                 filePaths.append(os.path.join(root,file))
        #     #sort filePaths by tree*.root
        #     filePaths = sorted(filePaths,key=lambda x: int(x.split('tree_')[-1].split('.root')[0])) 
        #     #now save the path information to another json file
        #     newjsondict = {}
        #     newjsondict['name'] = alias
        #     for key in sampleInfo:
        #         newjsondict[key] = sampleInfo[key]
        #     fileJsonPath = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','ForSNU',alias+'.json')
        #     newjsondict['path'] = filePaths
        #     with open(fileJsonPath,'w') as f:
        #         json.dump(newjsondict,f,indent=4)
        # else:
        #     for period in sampleInfo['periods']:
        #         path = os.path.join(basePath,era,'DATA',alias,f"Period{period}")
        #         filePaths = []
        #         #Folder structure is not fixed yet, so let's do the recursive search until the .root file appears, and save all absolute paths
        #         for root, dirs, files in os.walk(path):
        #             for file in files:
        #                 if file.endswith('.root'):
        #                     filePaths.append(os.path.join(root,file))
        #         #sort filePaths by tree*.root
        #         filePaths = sorted(filePaths,key=lambda x: int(x.split('tree_')[-1].split('.root')[0]))
        #         #now save the path information to another json file
        #         newjsondict = {}
        #         newjsondict['name'] = alias
        #         for key in sampleInfo:
        #             if key == 'periods':
        #                 continue
        #             newjsondict[key] = sampleInfo[key]
        #         fileJsonPath = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','ForSNU',alias+f'_{period}.json')
        #         newjsondict['path'] = filePaths
        #         with open(fileJsonPath,'w') as f:
        #             json.dump(newjsondict,f,indent=4)

import os
import json
from multiprocessing import Pool

# Top-level function to call either process_mc_sample or process_data_sample
def process_sample(fn, alias, sampleInfo, era, basePath):
    fn(alias, sampleInfo, era, basePath)

def process_mc_sample(alias, sampleInfo, era, basePath):
    path = os.path.join(basePath, era, 'MC', sampleInfo['PD'])
    filePaths = []
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith('.root'):
                filePaths.append(os.path.join(root, file))
    filePaths = sorted(filePaths, key=lambda x: int(x.split('tree_')[-1].split('.root')[0]))

    newjsondict = {'name': alias}
    newjsondict.update(sampleInfo)
    fileJsonPath = os.path.join(os.environ['SKNANO_DATA'], era, 'Sample', 'ForSNU', alias + '.json')
    newjsondict['path'] = filePaths
    with open(fileJsonPath, 'w') as f:
        json.dump(newjsondict, f, indent=4)

def process_data_sample(alias, sampleInfo, era, basePath):
    for period in sampleInfo['periods']:
        path = os.path.join(basePath, era, 'DATA', alias, f"Period{period}")
        filePaths = []
        for root, dirs, files in os.walk(path):
            for file in files:
                if file.endswith('.root'):
                    filePaths.append(os.path.join(root, file))
        filePaths = sorted(filePaths, key=lambda x: int(x.split('tree_')[-1].split('.root')[0]))

        newjsondict = {'name': alias}
        for key in sampleInfo:
            if key == 'periods':
                continue
            newjsondict[key] = sampleInfo[key]
        fileJsonPath = os.path.join(os.environ['SKNANO_DATA'], era, 'Sample', 'ForSNU', alias + f'_{period}.json')
        newjsondict['path'] = filePaths
        with open(fileJsonPath, 'w') as f:
            json.dump(newjsondict, f, indent=4)

def fillSamplePath(era):
    sampleInfos = loadCommonSampleInfo(era)
    
    # Create a list of tasks for multiprocessing
    tasks = []
    for alias, sampleInfo in sampleInfos.items():
        if sampleInfo['isMC']:
            tasks.append((process_mc_sample, alias, sampleInfo, era, basePath))
        else:
            tasks.append((process_data_sample, alias, sampleInfo, era, basePath))

    # Use multiprocessing to parallelize
    with Pool(processes=16) as pool:
        pool.starmap(process_sample, tasks)



def updateXsec(era):
    sampleInfos = loadCommonSampleInfo(era)
    for alias, sampleInfo in sampleInfos.items():
        if sampleInfo['isMC'] and 'xsec_formula' in sampleInfo:
            xsec = eval(sampleInfo['xsec_formula'])
            sampleInfo['xsec'] = xsec
    sampleInfoJson = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','CommonSampleInfo.json')
    with open(sampleInfoJson,'w') as f:
        json.dump(sampleInfos,f,indent=4)
        
def updateMcInfo(era):
    import ROOT
    sampleInfos = loadCommonSampleInfo(era)
    for alias, sampleInfo in sampleInfos.items():
        if sampleInfo['isMC']:
            try:
                f = ROOT.TFile.Open(os.path.join(os.environ['SKNANO_OUTPUT'],'GetEffLumi',era,alias+'.root'))
            except:
                print(f'File {alias}.root not found')
                print('############################\n')
                continue
            h_sumW = f.Get('sumW')
            h_sumSign = f.Get('sumSign')
            nmc = h_sumW.GetEntries()
            sumW = h_sumW.GetBinContent(1)
            sumSign = h_sumSign.GetBinContent(1)
            sampleInfo['nmc'] = nmc
            sampleInfo['sumW'] = sumW
            sampleInfo['sumsign'] = sumSign
            print('Will update the MC information for',alias,'from')
            print(f'nmc:{sampleInfo["nmc"]}, sumW:{sampleInfo["sumW"]}, sumSign:{sampleInfo["sumsign"]} to')
            print(f'nmc:{nmc}, sumW:{sumW}, sumSign:{sumSign}')
            print('############################\n')
        else:
            nevt = []
            for period in sampleInfo['periods']:
                try:
                    f = ROOT.TFile.Open(os.path.join(os.environ['SKNANO_OUTPUT'],'GetEffLumi',era,alias+f'_{period}.root'))
                except:
                    print(f'File {alias}_{period}.root not found')
                    print('############################\n')
                    continue
                h = f.Get('NEvents')
                NEvents = h.GetBinContent(1)
                nevt.append(NEvents)
            print('Will update the DATA information for',alias,'from')
            print(f'nmc:{sampleInfo["NEvents"]} to')
            print(f'nmc:{nevt}')
            print('############################\n')
            sampleInfo['NEvents'] = nevt
    sampleInfoJson = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','CommonSampleInfo.json')
    with open(sampleInfoJson,'w') as f:
        json.dump(sampleInfos,f,indent=4)

def makeSkimTreeInfo(era,skimTreeFolder,skimTreeSuffix,skimTreeOrigPD):
    from copy import deepcopy
    isMC = True
    if skimTreeOrigPD.split("_")[-1].isupper() and len(skimTreeOrigPD.split("_")[-1]) == 1:
        #if sample is ends with _one capital letter, it is data
        #hope there will be no exception(please)
        isMC = False
        period = skimTreeOrigPD.split("_")[-1]
        skimTreeOrigPD = skimTreeOrigPD[:-2]
        

    sampleInfos = loadCommonSampleInfo(era)
    skimJsonFolderPath = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','Skim')
    if not os.path.exists(skimJsonFolderPath):
        os.makedirs(skimJsonFolderPath)
    skimTreeSummaryJsonPath = os.path.join(skimJsonFolderPath,'skimTreeInfo.json')
    if os.path.exists(skimTreeSummaryJsonPath):
        skimTreeSummary = json.load(open(skimTreeSummaryJsonPath))
    else:
        skimTreeSummary = {}
    #check already exist
    if f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}' in skimTreeSummary:
        skimTreeSummary[f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}']['suffix'] = skimTreeSuffix
        skimTreeSummary[f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}']['PD'] = skimTreeOrigPD
        skimTreeSummary[f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}']['isMC'] = int(isMC)
        if not isMC:
            if period not in skimTreeSummary[f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}']['periods']:
                skimTreeSummary[f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}']['periods'].append(period)
    else:
        skimTreeSummary[f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}'] = {'suffix':skimTreeSuffix,'PD':skimTreeOrigPD,'isMC':int(isMC)}
        if not isMC:
            skimTreeSummary[f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}']['periods'] = [period]


    #just for redundancy
    if isMC != sampleInfos[skimTreeOrigPD]['isMC']:
        print('Error: The sample type is not matched')
        return
    
    filePaths = []
    if isMC:
        path = os.path.join(skimTreeFolder,f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}')
    else:
        path = os.path.join(skimTreeFolder,f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}',f'Period{period}')
    print(path)
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith('.root'):
                filePaths.append(os.path.join(root,file))

    filePaths = sorted(filePaths,key=lambda x: int(x.split('tree_')[-1].split('.root')[0]))
    if isMC:
        skimPathInfoJson = os.path.join(skimJsonFolderPath,f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}.json')
    else:
        skimPathInfoJson = os.path.join(skimJsonFolderPath,f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}_{period}.json')
    skimTreePath = deepcopy(skimTreeSummary[f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}'])
    skimTreePath['name']= f'Skim_{skimTreeSuffix}_{skimTreeOrigPD}'
    skimTreePath.pop('periods',None)
    skimTreePath['path'] = filePaths
    with open(skimPathInfoJson,'w') as f:
        json.dump(skimTreePath,f,indent=4)
    with open(skimTreeSummaryJsonPath,'w') as f:
        json.dump(skimTreeSummary,f,indent=4)
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--fillSamplePath', action='store_true',help='Fill the path information')
    parser.add_argument('--updateXsec',action='store_true',help='update the Xsec to json from Xsec formula')
    parser.add_argument('--updateMcInfo',action='store_true',help='update the MC information to json from result of GetEffLumi(SumW, nmc)')
    ###below arguments are used for SKFlat.py for automatic update of the skim information
    parser.add_argument('--makeSkimTreeInfo',action='store_true',help='Make the SkimTreeInfo')
    parser.add_argument('--skimTreeFolder',dest='skimTreeFolder',default='',help='Folder where the skim tree is stored')
    parser.add_argument('--skimTreeSuffix',dest='skimTreeSuffix',default='',help='Suffix of the skim tree')
    parser.add_argument('--skimTreeOrigPD', dest='skimTreeOrigPD',default='',help='Original PD of the skim tree')
    parser.add_argument('--era',dest='era',default='',help='Era of the sample')
    args = parser.parse_args()
    run3eras = ['2022','2022EE', '2023', '2023BPix']
    run2eras = ['2016preVFP','2016postVFP','2017','2018']
    eras = run3eras + run2eras
    if args.era == '':
        for era in eras:
            if era in run3eras:
                basePath = os.environ['SKNANO_RUN3_NANOAODPATH']
            elif era in run2eras:
                basePath = os.environ['SKNANO_RUN2_NANOAODPATH']
            if args.fillSamplePath:
                fillSamplePath(era)
            if args.updateXsec:
                updateXsec(era)
            if args.updateMcInfo:
                updateMcInfo(era)
            


    else:
        era = args.era
        if era in run3eras:
            basePath = os.environ['SKNANO_RUN3_NANOAODPATH']
        elif era in run2eras:
            basePath = os.environ['SKNANO_RUN2_NANOAODPATH']
        if args.fillSamplePath:
            fillSamplePath(era)
        if args.updateXsec:
            updateXsec(era)
        if args.updateMcInfo:
            updateMcInfo(era)
        if args.makeSkimTreeInfo:
            makeSkimTreeInfo(era,args.skimTreeFolder,args.skimTreeSuffix, args.skimTreeOrigPD)
                
