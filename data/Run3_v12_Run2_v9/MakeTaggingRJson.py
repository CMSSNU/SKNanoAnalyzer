#to run this script, you need to install uproot.
import ROOT
import correctionlib.convert as convert
import json
import argparse
import os
import itertools
import copy
import multiprocessing as mp
from tqdm import tqdm
from functools import partial
from copy import deepcopy
r_hist = None
def histParser(f):
    numerator_dicts = []
    denominator_dicts = []
    for keyObj in f.GetListOfKeys():
        key = keyObj.GetName()
        if not '##' in key:
            continue
        infos = key.split("##")
        this_info = {}
        isNumerator = True
        for info in infos:
            if 'den' in info:
                isNumerator = False
                continue
            if 'num' in info:
                isNumerator = True
                continue
            if '#' not in info:
                continue
                
            k, value = info.split("#")
            this_info[k] = value

        this_info['hist_key']=key
        if isNumerator:
            numerator_dicts.append(this_info)
        else:
            denominator_dicts.append(this_info)
    return numerator_dicts, denominator_dicts
        
def makeTempEffHist(folder):
    dict_per_file = {}
    ratio_hists = []

    
    #hadd all the files
    os.chdir(folder)
    #save current directory
    current_dir = os.getcwd()
    if not os.path.exists('temp'):
        os.mkdir('temp')
    os.system('rm output.root')
    os.system(f'hadd -f temp/output.root *.root')
    
    
    f = ROOT.TFile("temp/output.root","READ")
    
    numerator_dicts, denominator_dicts = histParser(f)
            

    for info in numerator_dicts:
        key = info['hist_key']
        ratio_hist = f.Get(key).Clone()
        name = key.replace("num","r")
        ratio_hist.SetNameTitle(name,name)
        ratio_hist.Divide(f.Get(key.replace("num","den")))
        ratio_hist.SetDirectory(0)
        ratio_hists.append(ratio_hist.Clone())

    
    f2 = ROOT.TFile("output.root","RECREATE")
    f2.cd()
    for ratio_hist in ratio_hists:
        ratio_hist.Write()
    f2.Close()
    f.Close()
    #get absolute path of f2
    global r_hist
    r_hist = os.path.abspath("output.root")
    
    
    

def makingJson(era, taggingmode):
    main_json = {"schema_version": 2,
            "description": "This json file contains the R factor for the deepJet, particleNet, robustParticleTransformer AK4 jet taggers",
            "corrections": []}
    categorys={
    "systematic":{"type":"string", "content":[]}, 
    "category":{"type":"string", "description":"category of the process. TTBB, TTBJ, TTCC, TTJJ for TTbar, total for other processes.", "content":[]},
    }
    if taggingmode == 'c':
        categorys['nTrueInt'] = {"type":"real","description":"number of true interactions"}
        categorys['HT'] = {"type":"real","description":"HT"}
    else:
        categorys['njet'] = {"type":"int","description":"number of jets"}
        categorys['HT'] = {"type":"real","description":"HT"}
    process_list = set()
    process_category = set()
    systematics = set()
    taggers = ["deepJet","particleNet","robustParticleTransformer"]
    
    print(f"open file {r_hist}")
    f = ROOT.TFile(r_hist,"READ")
    keys = f.GetListOfKeys()
    keys = [key.GetName() for key in keys]
    keys = [key for key in keys if f'tagging#{taggingmode}' in key]
    print(f"currentely in {os.getcwd()}")
    for key in keys:
       infos = key.split("##")
       for info in infos:
            if 'sample' in info:
                process_list.add(info.split("#")[1])
            if 'category' in info:
                process_category.add(info.split("#")[1])
            if 'systematic' in info:
                systematics.add(info.split("#")[1])
    
    process_list = list(process_list)
    process_category = list(process_category)
    systematics = list(systematics)
    
    
    categorys['systematic']['content'] = systematics
    categorys['category']['content'] = process_category

    for tagger in taggers:
        for process in process_list:
            print(f'Creating json for {tagger} {process}')
            tagger_eff_dict={}
            tagger_eff_dict['name'] = tagger + "_" + process
            tagger_eff_dict['version'] = 0
            tagger_eff_dict['inputs'] = []
            tagger_eff_dict['output'] = {"name":"weight","type":"real"}
            for name, cat in categorys.items():
                tagger_eff_dict['inputs'].append(copy.deepcopy(cat))
                tagger_eff_dict['inputs'][-1]['name'] = name
                if 'content' in tagger_eff_dict['inputs'][-1].keys():
                    del tagger_eff_dict['inputs'][-1]['content']
                    
            data_dict = {"nodetype":"category","input":"systematic","content":[]}
            tagger_eff_dict['data']=data_dict
            for systematic in categorys['systematic']['content']:
                data_dict['content'].append({"key":systematic,"value":{'nodetype':"category","input":"category","content":[]}})
                for category in categorys['category']['content']:
                    data_dict['content'][-1]['value']['content'].append({"key":category,"value":{}})

            main_json['corrections'].append(tagger_eff_dict)

                #iterates over all root files and get the R factor
    f = ROOT.TFile("output.root","READ")
    keys = f.GetListOfKeys()
    keys = [key.GetName() for key in keys]
    keys = [key for key in keys if f'tagging#{taggingmode}' in key]
    keys = [key for key in keys if "2D" in key]
    f.Close()
    # Use multiprocessing pool
    with mp.Pool(32) as pool:
        process_func = partial(process_key, taggingmode=taggingmode)
        results = list(tqdm(pool.imap(process_func, keys), total=len(keys)))

    # Update main_json with results
    for process, tagger, systematic, category, this_data in results:
        for tagger_dict in main_json['corrections']:
            if tagger_dict['name'] == f"{tagger}_{process}":
                for systematic_dict in tagger_dict['data']['content']:
                    if systematic_dict['key'] == systematic:
                        for category_dict in systematic_dict['value']['content']:
                            if category_dict['key'] == category:

                                category_dict['value'] = copy.deepcopy(this_data)
                                break
                        break
                break
        
        
    # for process, tagger, systematic, category in itertools.product(process_list, taggers,categorys['systematic']['content'],categorys['category']['content']):
    #     exist = True
    #     this_data = {}
    #     try:
    #         c1 = convert.from_uproot_THx(f'output.root:tagging#{taggingmode}##era#{era}##systematic#{systematic}##Hist#2D##tagger#{tagger}##r##category#{category}##sample#{process}')
    #         this_data = c1.dict()['data']  
    #     except:
    #         exist = False
    #         this_data = { "nodetype": "formula",
    #                               "expression": "1.",
    #                               "parser": "TFormula",
    #                               "variables": [
    #                             ],
    #                               "parameters": []
    #                             }
    

    return main_json
                    
def process_key(key, taggingmode):
    # Parse key information
    info_pre = key.split("##")
    infos = {}
    for info in info_pre:
        if '#' in info:
            k, value = info.split("#")
            infos[k] = value
    
    # Process data
    process = infos['sample']
    tagger = infos['tagger']
    systematic = infos['systematic']
    category = infos['category']

    # Convert data from ROOT
    c1 = convert.from_uproot_THx(f'output.root:{key}')
    #do deep copy
    this_data = copy.deepcopy(c1.dict()['data'])
    this_data['inputs'] = ['nTrueInt', 'HT'] if taggingmode == 'c' else ['njet', 'HT']
    this_data['flow'] = 'clamp'
    
    return (process, tagger, systematic, category, this_data)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Create JSON file for b-tagging efficiency')
    parser.add_argument('--input_folder', dest='input_folder',help='Input ROOT file folder',default=os.environ['SKNANO_OUTPUT']+'/MeasureJetTaggingR')
    parser.add_argument('--out_name_str',dest='out_name_str', help='Output JSON file name', default='')
    args = parser.parse_args()
    out_name_str = args.out_name_str

    totalEras = ['2022','2022EE']
    totalEras = ['2017','2018','2016preVFP','2016postVFP']
    totalEras = ['2022EE']
    for era in totalEras:
        out_dir = os.path.join(os.environ['SKNANO_DATA'],era,'BTV')
        if not os.path.exists(out_dir):
            os.makedirs(out_dir)
        
        folder = os.path.join(args.input_folder, era)
        makeTempEffHist(folder)
        main_json_btagging = makingJson(era,'b')
        
        with open(os.path.join(out_dir,out_name_str+'_btaggingR.json'), 'w') as f:
             json.dump(main_json_btagging, f, indent=4)
        main_json_ctagging = makingJson(era,'c') 
        with open(os.path.join(out_dir,out_name_str+'_ctaggingR.json'), 'w') as f:
            json.dump(main_json_ctagging, f, indent=4)




