#to run this script, you need to install uproot.
import ROOT
import correctionlib.convert as convert
import json
import argparse
import os
import itertools
import copy

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
    for file in os.listdir(folder):
        if not file.endswith('.root'):
            continue
        print(f'Processing {file}')
        f = ROOT.TFile(os.path.join(folder,file),"READ")
        numerator_dicts, denominator_dicts = histParser(f)
        
        for numerator_dict in numerator_dicts:
            key = numerator_dict['hist_key']
            ratio_hist = f.Get(key).Clone()
            name = "process#" + file.replace('.root','') + "##" +key.replace("num","r")
            ratio_hist.SetNameTitle(name,name)
            ratio_hist.Divide(f.Get(key.replace("num","den")))
            ratio_hist.SetDirectory(0)
            ratio_hists.append(ratio_hist)

    f2 = ROOT.TFile("output.root","RECREATE")
    f2.cd()
    for ratio_hist in ratio_hists:
        ratio_hist.Write()
    f2.Close()

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
    
    f = ROOT.TFile("output.root","READ")
    keys = f.GetListOfKeys()
    keys = [key.GetName() for key in keys]
    for key in keys:
       infos = key.split("##")
       for info in infos:
            if 'process' in info:
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
        
    for process, tagger, systematic, category in itertools.product(process_list, taggers,categorys['systematic']['content'],categorys['category']['content']):
        exist = True
        this_data = {}
        try:
            c1 = convert.from_uproot_THx(f'output.root:process#{process}##tagging#{taggingmode}##era#{era}##systematic#{systematic}##Hist#2D##tagger#{tagger}##r##category#{category}')
            this_data = c1.dict()['data']  
        except:
            exist = False
            this_data = { "nodetype": "formula",
                                  "expression": "1.",
                                  "parser": "TFormula",
                                  "variables": [
                                ],
                                  "parameters": []
                                }
            
        
        
        this_data['inputs'] = ['nTrueInt','HT'] if taggingmode == 'c' else ['njet','HT']
        this_data['flow'] = 'clamp'
        #now find the appropriate location in main_json put this data
        for tagger_dict in main_json['corrections']:
            if tagger_dict['name'] == tagger + "_" + process:
                for systematic_dict in tagger_dict['data']['content']:
                    if systematic_dict['key'] == systematic:
                        for category_dict in systematic_dict['value']['content']:
                            if category_dict['key'] == category:
                                category_dict['value'] = this_data
                                break
                        break
                break
        
    return main_json
                    
                
        
 




if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Create JSON file for b-tagging efficiency')
    parser.add_argument('--input_folder', dest='input_folder',help='Input ROOT file folder',default=os.environ['SKNANO_OUTPUT']+'/MeasureJetTaggingR')
    parser.add_argument('--out_name_str',dest='out_name_str', help='Output JSON file name', default='')
    args = parser.parse_args()
    out_name_str = args.out_name_str

    totalEras = ['2022','2022EE']
    totalEras = ['2017','2018','2016preVFP','2016postVFP']
    totalEras = ['2016preVFP']
    for era in totalEras:
        out_dir = os.path.join(os.environ['SKNANO_DATA'],era,'BTV')
        if not os.path.exists(out_dir):
            os.makedirs(out_dir)
        
        folder = os.path.join(args.input_folder, era)
        makeTempEffHist(folder)
        main_json_btagging = makingJson(era,'b')
        main_json_ctagging = makingJson(era,'c')
        with open(os.path.join(out_dir,out_name_str+'btaggingR.json'), 'w') as f:
             json.dump(main_json_btagging, f, indent=4)
        with open(os.path.join(out_dir,out_name_str+'ctaggingR.json'), 'w') as f:
            json.dump(main_json_ctagging, f, indent=4)




