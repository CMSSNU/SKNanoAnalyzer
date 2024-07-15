import os
import json
import argparse
#This script is used to generate the path information for the sample data
#Sample information is stored in the CommonSampleInfo.json
#This script will generate the path information for the sample data
basePath = '/gv0/Users/yeonjoon/DATA/SKFlat/Run3NanoAODv12/' #currently my private directory  
    
def loadCommonSampleInfo(era):
    sampleInfoJson = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','CommonSampleInfo.json')
    sampleInfos = json.load(open(sampleInfoJson))
    return sampleInfos

def fillSamplePath(era):
    sampleInfos = loadCommonSampleInfo(era)
    for alias,sampleInfo in sampleInfos.items():
        if sampleInfo['isMC']:
            path = os.path.join(basePath,era,'MC',sampleInfo['PD'])
            filePaths = []
            #Folder structure is not fixed yet, so let's do the recursive search until the .root file appears, and save all absolute paths
            for root, dirs, files in os.walk(path):
                for file in files:
                    if file.endswith('.root'):
                        filePaths.append(os.path.join(root,file))
            #sort filePaths by tree*.root
            filePaths = sorted(filePaths,key=lambda x: int(x.split('tree_')[-1].split('.root')[0])) 
            #now save the path information to another json file
            newjsondict = {}
            newjsondict['name'] = alias
            for key in sampleInfo:
                newjsondict[key] = sampleInfo[key]
            fileJsonPath = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','ForSNU',alias+'.json')
            newjsondict['path'] = filePaths
            with open(fileJsonPath,'w') as f:
                json.dump(newjsondict,f,indent=4)
        else:
            for period in sampleInfo['periods']:
                path = os.path.join(basePath,era,'DATA',alias,f"Period{period}")
                filePaths = []
                #Folder structure is not fixed yet, so let's do the recursive search until the .root file appears, and save all absolute paths
                for root, dirs, files in os.walk(path):
                    for file in files:
                        if file.endswith('.root'):
                            filePaths.append(os.path.join(root,file))
                #sort filePaths by tree*.root
                filePaths = sorted(filePaths,key=lambda x: int(x.split('tree_')[-1].split('.root')[0]))
                #now save the path information to another json file
                newjsondict = {}
                newjsondict['name'] = alias
                for key in sampleInfo:
                    if key == 'periods':
                        continue
                    newjsondict[key] = sampleInfo[key]
                fileJsonPath = os.path.join(os.environ['SKNANO_DATA'],era,'Sample','ForSNU',alias+f'_{period}.json')
                newjsondict['path'] = filePaths
                with open(fileJsonPath,'w') as f:
                    json.dump(newjsondict,f,indent=4)

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

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--fillSamplePath', action='store_true',help='Fill the path information')
    parser.add_argument('--updateXsec',action='store_true',help='update the Xsec to json from Xsec formula')
    parser.add_argument('--updateMcInfo',action='store_true',help='update the MC information to json from result of GetEffLumi(SumW, nmc)')
    args = parser.parse_args()
    eras = ['2022','2022EE']
    
    for era in eras:
        if args.fillSamplePath:
            fillSamplePath(era)
        if args.updateXsec:
            updateXsec(era)
        if args.updateMcInfo:
            updateMcInfo(era)
        
            
