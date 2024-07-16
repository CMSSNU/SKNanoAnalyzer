#to run this script, you need to install uproot.
import ROOT
import correctionlib.convert as convert
import json
import argparse
import os
import itertools

def histParser(f):
    numerator_dicts = []
    denominator_dicts = []
    for keyObj in f.GetListOfKeys():
        key = keyObj.GetName()
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
            k, value = info.split("#")
            this_info[k] = value
        this_info['hist_key']=key
        if isNumerator:
            numerator_dicts.append(this_info)
        else:
            denominator_dicts.append(this_info)
    return numerator_dicts, denominator_dicts
        
def makeTempEffHist(f):
    numerator_dicts, denominator_dicts = histParser(f)
    ratio_hists = []
    for numerator_dict in numerator_dicts:
        #find denominator that have same flavor, tagging
        for denominator_dict in denominator_dicts:
            if numerator_dict['flavor'] == denominator_dict['flavor']:
                numerator_hist = f.Get(numerator_dict['hist_key'])
                denominator_hist = f.Get(denominator_dict['hist_key'])
                ratio_hist = numerator_hist.Clone()
                ratio_hist.Divide(denominator_hist)
                ratio_hist.SetName(numerator_dict['hist_key'].replace("num","eff"))
                ratio_hists.append(ratio_hist)
                 
    f2 = ROOT.TFile("output.root","RECREATE")
    f2.cd()
    for ratio_hist in ratio_hists:
        ratio_hist.Write()
    f2.Close()

def makingJson(era, f, taggingmode):
    categorys={
    "systematic":{"type":"string","content":["central"]},
    "working_point":{ "type":"string","content":["L","M","T","XT","XXT"],"description":"L/M/T/XT/XXT"},
    "flavor":{"type":"int","content":[4,5,0],"description":"hadron flavor definition: 5=b, 4=c, 0=udsg"},
    }
    if taggingmode == 'c':
        categorys['working_point']['content'] = ["L","M","T"]
        categorys['working_point']['description'] = "L/M/T"

    main_json = {"schema_version": 2,
               "description": "This json file contains the corrections for the deepJet, particleNet, robustParticleTransformer AK4 jet taggers for the 2022_Summer22 campaign.  Corrections are supplied for b-tag discriminator shape correction (shape) and b-tagging working point corrections (comb/mujets/light)",
             "corrections": []}
    
    taggers = ["deepJet","particleNet","robustParticleTransformer"]

    #loop that make basic struture of json. (categorys)
    for tagger in taggers:
        tagger_eff_dict={}
        tagger_eff_dict['name'] = tagger
        tagger_eff_dict['version'] = 0
        tagger_eff_dict['inputs'] = []
        for name, cat in categorys.items():
            if 'description' in cat:
                tagger_eff_dict['inputs'].append({"name":name,"type":cat['type'],"description":cat['description']})
            else:
                tagger_eff_dict['inputs'].append({"name":name,"type":cat['type']})
        tagger_eff_dict['inputs'].append({"name":'abseta',"type":"real"})
        tagger_eff_dict['inputs'].append({"name":'pt',"type":"real"})
        tagger_eff_dict['output'] = {"name":"eff","type":"real"}
        data_dict = {"nodetype":"category","input":"systematic","content":[]}
        tagger_eff_dict['data']=data_dict
        for systematic in categorys['systematic']['content']:
            data_dict['content'].append({"key":systematic,"value":{'nodetype':"category","input":"working_point","content":[]}})
            for working_point in categorys['working_point']['content']:
                data_dict['content'][-1]['value']['content'].append({"key":working_point,"value":{'nodetype':"category","input":"flavor","content":[]}})
                for flavor in categorys['flavor']['content']:
                    data_dict['content'][-1]['value']['content'][-1]['value']['content'].append({"key":flavor,"value":{}})

        main_json['corrections'].append(tagger_eff_dict)

    #below part is responsible for actual value


    for tagger, systematic, working_point, flavor in itertools.product(taggers,categorys['systematic']['content'],categorys['working_point']['content'],categorys['flavor']['content']):
        c1 = convert.from_uproot_THx(f'output.root:tagging#{taggingmode}##era#{era}##tagger#{tagger}##working_point#{working_point}##flavor#{flavor}##systematic#{systematic}##eff;1')
        this_data = c1.dict()['data']
        this_data['inputs'] = ['abseta','pt']
        this_data['flow'] = 'clamp'
        #now find the appropriate location in main_json put this data
        for tagger_dict in main_json['corrections']:
            if tagger_dict['name'] == tagger:
                for systematic_dict in tagger_dict['data']['content']:
                    if systematic_dict['key'] == systematic:
                        for working_point_dict in systematic_dict['value']['content']:
                            if working_point_dict['key'] == working_point:
                                for flavor_dict in working_point_dict['value']['content']:
                                    if flavor_dict['key'] == flavor:
                                        
                                        flavor_dict['value'] = this_data
                                        break
                                break
                        break
                break
    
    return main_json




if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Create JSON file for b-tagging efficiency')
    parser.add_argument('--input_folder', dest='input_folder',help='Input ROOT file folder',default=os.environ['SKNANO_OUTPUT']+'/MeasureJetTaggingEff')
    parser.add_argument('--input',dest='input', help='Input ROOT file name', default='TTLJ_powheg.root')
    parser.add_argument('--out_name_str',dest='out_name_str', help='Output JSON file name', default='')
    args = parser.parse_args()
    out_name_str = args.out_name_str

    totalEras = ['2022','2022EE']
    for era in totalEras:
        out_dir = os.path.join(os.environ['SKNANO_DATA'],era,'BTV')
        if not os.path.exists(out_dir):
            os.makedirs(out_dir)
        
        f = ROOT.TFile(os.path.join(args.input_folder, era, args.input))
        makeTempEffHist(f)
        main_json_btagging = makingJson(era, f, 'b')
        main_json_ctagging = makingJson(era, f, 'c')
        with open(os.path.join(out_dir,out_name_str+'btaggingEff.json'), 'w') as f:
            json.dump(main_json_btagging, f, indent=4)
        with open(os.path.join(out_dir,out_name_str+'ctaggingEff.json'), 'w') as f:
            json.dump(main_json_ctagging, f, indent=4)




