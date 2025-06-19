#to run this script, you need to install uproot.
import ROOT
import correctionlib.convert as convert
import json
import argparse
import os
import itertools
import copy
import multiprocessing 
from tqdm import tqdm
from functools import partial
from copy import deepcopy
r_hist = None
import concurrent.futures


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
    os.system(f'hadd -f9 -j 8 temp/output.root *.root')
    
    
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
    

def key_parser(key):
    infos = key.split("##")
    this_info = {}
    for info in infos:
        if '#' not in info:
            continue
        k, value = info.split("#")
        this_info[k] = value
    this_info['hist_key'] = key
    return this_info
    
def process_correction(tagger, process, key_dicts, r_hist):
        """
        Worker function to process a single (tagger, process) pair.

        Returns:
        - dict: The correction dictionary for the given tagger and process.
        """
        this_correction_dict = {
            'name': f"{tagger}_{process}",
            'version': 0,
            'inputs': [
                {"type": "string", "name": "syst"},
                {"type": "int", "name": "flav"},
                {"type": "real", "name": "pt"},
                {"type": "real", "name": "abseta"}
            ],
            'output': {"name": "weight", "type": "real"},
            'data': {"nodetype": "category", "input": "syst", "content": []}
        }

        # Open the ROOT file inside the worker to ensure thread safety
        try:
            with ROOT.TFile(r_hist, "READ") as f:
                for key_dict in key_dicts:
                    sample = key_dict.get('sample')
                    tagger_key = key_dict.get('tagger')
                    syst = key_dict.get('variation')
                    syst_name = key_dict.get('systematic')
                    flav = key_dict.get('parton_flav')

                    if not all([sample, tagger_key, syst, syst_name, flav]):
                        continue  # Skip invalid key_dict

                    if sample != process or tagger_key != tagger:
                        continue

                    if syst == 'central' and syst_name != 'Central':
                        continue  # Not b-tag related

                    # Initialize systematic entry if necessary
                    syst_entry = next((item for item in this_correction_dict['data']['content'] if item['key'] == syst), None)
                    if not syst_entry:
                        syst_entry = {"key": syst, "value": {"nodetype": "category", "input": "flav", "content": []}}
                        this_correction_dict['data']['content'].append(syst_entry)

                    # Initialize flav entry within systematic
                    flav_entry = next((item for item in syst_entry['value']['content'] if item['key'] == flav), None)
                    if not flav_entry:
                        flav_entry = {"key": int(flav), "value": {}}
                        syst_entry['value']['content'].append(flav_entry)
                    # Load histogram data and update correction dictionary
                    try:
                        hist_path = f'output.root:{key_dict["hist_key"]}'
                        c1 = convert.from_uproot_THx(hist_path)
                        c1 = c1.dict()['data']
                        c1['inputs'] = ['pt', 'abseta']
                        c1['flow'] = 'clamp'
                        flav_entry['value'] = c1
                    except Exception as e:
                        raise RuntimeError(f"Failed to process histogram for key {key_dict}: {e}")

        except Exception as e:
            raise RuntimeError(f"Error processing tagger '{tagger}' and process '{process}': {e}")

        return this_correction_dict

def makingJson(era, taggingmode):
    main_json = {
        "schema_version": 2,
        "description": "This json file contains the R factor for the deepJet, particleNet, robustParticleTransformer AK4 jet taggers",
        "corrections": []
    }

    categorys = {
        "systematic": {"type": "string", "content": []},
        "category": {
            "type": "string",
            "description": "category of the process. TTBB, TTBJ, TTCC, TTJJ for TTbar, total for other processes.",
            "content": []
        },
        "pt": {"type": "real", "description": "pt of the jet"},
        "abseta": {"type": "real", "description": "abseta of the jet"}
    }

    taggers = ["deepJet", "particleNet", "robustParticleTransformer"]

    try:
        # Open the ROOT file once to extract key_dicts
        with ROOT.TFile(r_hist, "READ") as f:
            keys = f.GetListOfKeys()
            keys = [key.GetName() for key in keys]
            key_dicts = [key_parser(key) for key in keys]
    except Exception as e:
        raise RuntimeError(f"Failed to open or parse ROOT file {r_hist}: {e}")

    # Filtering with tagging mode
    key_dicts = [key_dict for key_dict in key_dicts if key_dict.get('tagging') == taggingmode]

    all_processes = sorted(set([key_dict['sample'] for key_dict in key_dicts]))

    # Prepare arguments for multiprocessing
    tasks = [(tagger, process, key_dicts, r_hist) for tagger in taggers for process in all_processes]

    # Determine the number of workers based on CPU cores
    num_workers = min(multiprocessing.cpu_count(), len(tasks))
    print(f"Starting multiprocessing with {num_workers} workers...")

    corrections = []
    with concurrent.futures.ProcessPoolExecutor(max_workers=num_workers) as executor:
        # Initialize tqdm progress bar
        with tqdm(total=len(tasks), desc="Processing", position=0) as pbar:
            # Start the load operations and mark each future with its (tagger, process)
            future_to_task = {executor.submit(process_correction, *task): task for task in tasks}
            for future in concurrent.futures.as_completed(future_to_task):
                task = future_to_task[future]
                try:
                    correction = future.result()
                    corrections.append(correction)
                except Exception as exc:
                    print(f"Task {task} generated an exception: {exc}")
                finally:
                    pbar.update(1)

    # Append all corrections to the main JSON
    main_json['corrections'] = corrections

    return main_json

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




