#!/usr/bin/env python3
###########################################################
###########This is Developement version####################
###########################################################

#This it the preliminary version of SKFlat.py
#Using htcondor python binding and DAGMAN workflow manager
import os
import argparse
import htcondor
from htcondor import dags
dag = dags.DAG()
import datetime
from itertools import product
import json
from time import sleep
from multiprocessing import Pool
import requests

##############################
#Global Variables
##############################
SKNANO_RUNLOG = os.environ['SKNANO_RUNLOG']
SKNANO_OUTPUT = os.environ['SKNANO_OUTPUT']
SKNANO_DATA = os.environ['SKNANO_DATA']
SKNANO_LIB = os.environ['SKNANO_LIB']
username = os.environ['USER']
totalEras = ['2022','2022EE']
TOKEN = os.environ['TOKEN_TELEGRAMBOT']
chat_id = os.environ['USER_CHATID']
url = f"https://api.telegram.org/bot{TOKEN}/sendMessage?chat_id={chat_id}"
SKIMMING_MODE = False
##############################
#Load commonSampleInfo.json at start
sampleInfoJsons = {}
sampleInfoJsons['2022'] = json.load(open(os.path.join(SKNANO_DATA,'2022','Sample','CommonSampleInfo.json')))
sampleInfoJsons['2022EE'] = json.load(open(os.path.join(SKNANO_DATA,'2022EE','Sample','CommonSampleInfo.json')))
skimInfoJsons = {}
try:
    skimInfoJsons['2022'] = json.load(open(os.path.join(SKNANO_DATA,'2022','Sample','Skim','skimTreeInfo.json')))
    skimInfoJsons['2022EE'] = json.load(open(os.path.join(SKNANO_DATA,'2022EE','Sample','Skim','skimTreeInfo.json')))
except:
    print('\033[93m'+"Warning: Skimmed tree info is not exist"+'\033[0m')
##############################
def isMCandGetPeriod(sample):
    #if sample is ends with _one capital letter, it is data
    #hope there will be no exception(please)
    if sample.split("_")[-1].isupper() and len(sample.split("_")[-1]) == 1:
        return False, sample.split("_")[-1]
    else:
        return True, None
    
def getSkimmingOutBaseAndSuffix(sample, AnalyzerName):
    isMC, period = isMCandGetPeriod(sample)
    suffix = f"Temp_Skim_{AnalyzerName.replace('Skim_','')}_{sample if isMC else sample.replace(f'_{period}','')}"
    out_base = os.path.join('/gv0/Users',username,'DATA/SKFlat/Run3NanoAODv12/',era,'MC' if isMC else 'DATA','Skim',suffix,'' if isMC else f'Period{period}', 'tree.root') if SKIMMING_MODE else 'output/hists.root'
    return out_base, suffix

def getEraList(eras):
    if eras == 'All':
        return totalEras
    eras = eras.split(",")
    for era in eras:
        if era not in totalEras:
            print('\033[91m'+f"ERROR: {era} is not a valid era"+'\033[0m')
            exit()
    return eras

def addPeriodToSampleList(samplelist,era):
    toBeAdded = []
    copylist = []
    for sample in samplelist:
        if sample.startswith("Skim_"):
            if SKIMMING_MODE:
                print('\033[91m'+f"Error: Skimmed sample {sample} is not allowed in skimming mode"+'\033[0m')
                exit()
            if sample not in skimInfoJsons[era]:
                print('\033[93m'+f"Warning: {sample} is not exist in era {era}"+'\033[0m')
                continue
            skimInfo = skimInfoJsons[era][sample]
            sampleInfo = sampleInfoJsons[era][skimInfo['PD']]
            if sampleInfo['isMC']:
                copylist.append(sample)
            if not sampleInfo['isMC']:
                copylist += [f"{sample}_{period}" for period in sampleInfo['periods']]
        else:
            if sample not in sampleInfoJsons[era]:
                print('\033[93m'+f"Warning: {sample} is not exist in era {era}"+'\033[0m')
                continue
            sampleInfo = sampleInfoJsons[era][sample]
            if sampleInfo['isMC']:
                copylist.append(sample)
            if not sampleInfo['isMC']:
                copylist += [f"{sample}_{period}" for period in sampleInfo['periods']]

    return copylist

def getUserFlagsList(Userflags):
    UserflagsList = Userflags.split(",")
    UserflagsList = [x for x in UserflagsList if x != ""]
    return UserflagsList

def getTimeStamp():
    ## TimeStamp

    # 1) dir/file name style
    JobStartTime = datetime.datetime.now()
    timestamp =  JobStartTime.strftime('%Y_%m_%d_%H%M%S')
    timestring =  JobStartTime.strftime('%Y-%m-%d %H:%M:%S')
    return timestamp, timestring

def setParser():
    parser = argparse.ArgumentParser(description='SKNano Command')
    parser.add_argument('-a', dest='Analyzer', default="")
    parser.add_argument('-i', dest='InputSample', default="", help="Input sample list, can be txt file, or comma separated")
    #parser.add_argument('-p', dest='DataPeriod', default="ALL")
    #parser.add_argument('-l', dest='InputSampleList', default="")
    parser.add_argument('-n', dest='NJobs', default=1, type=int, help="Number of jobs, If Negative, it will means number of files per job")
    #parser.add_argument('-o', dest='Outputdir', default="")
    #parser.add_argument('-q', dest='Queue', default="fastq")
    parser.add_argument('-e', dest='Era', default="All",help="2022, 2022EE. can be comma separated")
    parser.add_argument('--userflags', dest='Userflags', default="")
    parser.add_argument('--nmax', dest='NMax', default=300, type=int, help="maximum running jobs")
    parser.add_argument('--reduction', dest='Reduction', default=1, type=float)
    parser.add_argument('--memory', dest='Memory', default=2048, type=float)
    parser.add_argument('--batchname',dest='BatchName', default="")
    parser.add_argument('--skimming_mode',action='store_true',default=False,help="Enable this option when anlyzer is skimmer.")
    #Note: this option will change the behavior of the script. output directory will be changed to Your GV0, hadd will be disabled, and will creat the info json of skimmed tree   
    return parser

def getMasterDirectoryName(timeStamp, Analyzer, Userflags):
    MasterDirectoryName = f"{timeStamp}_{Analyzer}"
    if len(Userflags) > 0:
        for flag in Userflags:
            MasterDirectoryName += f"_{flag}"
    print(f"Creating Master Working Directory: {MasterDirectoryName}")
    abs_MasterDirectoryName = os.path.join(SKNANO_RUNLOG,MasterDirectoryName)
    print(f"Copy library files to {MasterDirectoryName.split('/')}")
    os.makedirs(abs_MasterDirectoryName)
    os.system(f"cp -r {SKNANO_LIB} {abs_MasterDirectoryName}")
    print("...Done")
    return MasterDirectoryName, abs_MasterDirectoryName

def getInputSampleList(inputArguments, eras):
    #if string
    if inputArguments.endswith(".txt"):
        with open(inputArguments, 'r') as f:
            InputSampleList = f.readlines()
    else:
        InputSampleList = inputArguments.split(",")
    InputSampleList = [x.replace("\n","").replace(" ","") for x in InputSampleList if not x.startswith("#") or not x.startswith("\n")]
    return InputSampleList
            
def jobFileDivider(files,ngroup):
    if ngroup > 0:
        if ngroup > len(files):
            ngroup = len(files)
        filegroups = [[] for i in range(ngroup)]
        for i, file in enumerate(files):
            filegroups[i%ngroup].append(file)
    elif ngroup < 0:
        #if ngroup is negative, it will be number of files per job
        nfile = abs(ngroup)
        if nfile > len(files):
            nfile = len(files)
        if nfile == 0:
            return []
        filegroups = [files[i:i + nfile] for i in range(0, len(files), nfile)]
        
    else:
        print('\033[91m'+"ERROR: ngroup should be positive or negative integer"+'\033[0m')
    return filegroups
 
def pythonJobProducer(era, sample, argparse, masterJobDirectory, userflags):
    isMC, period = isMCandGetPeriod(sample)
    AnalyzerName = argparse.Analyzer

        
    working_dir = os.path.join(masterJobDirectory,era,sample)
    os.makedirs(working_dir)
    if SKIMMING_MODE:
        out_base, suffix = getSkimmingOutBaseAndSuffix(sample, AnalyzerName)
        if not os.path.exists(os.path.dirname(out_base)):
            os.makedirs(os.path.dirname(out_base))
    else:
        os.makedirs(os.path.join(working_dir,"output"))
        out_base = os.path.join(working_dir,"output","hists.root")
    njobs = argparse.NJobs
    reduction = argparse.Reduction
    
    if sample.startswith("Skim_"):
        SkimInfo = skimInfoJsons[era][sample if isMC else sample.replace(f"_{period}","")]
        sampleInfo = sampleInfoJsons[era][SkimInfo['PD']]
        samplePaths = json.load(open(os.path.join(SKNANO_DATA,era,'Sample','Skim',sample+'.json')))['path']
        
    else:
        sampleInfo = sampleInfoJsons[era][sample if isMC else sample.replace(f"_{period}","")]
        samplePaths = json.load(open(os.path.join(SKNANO_DATA,era,'Sample','ForSNU',sample+'.json')))['path']
        
    samplePaths = jobFileDivider(samplePaths, njobs)

    
    totalNumberOfJobs = len(samplePaths)
    
    for i in range(totalNumberOfJobs):
        output = out_base.replace('.root',f'_{i}.root')
        #python job is too hard to debug. I will use c++ job instead
        # with open(os.path.join(working_dir,f"job{i}.py"),'w') as f:
        #     f.writelines(f"from ROOT import {argparse.Analyzer}\n")
        #     f.writelines(f"module = {argparse.Analyzer}()\n")
        #     f.writelines(f"module.SetTreeName('Events')\n")
            
        #     if isMC:
        #         f.writelines(f"module.IsDATA = False\n")
        #         f.writelines(f"module.MCSample = '{sample}'\n")
        #         f.writelines(f"module.xsec = {sampleInfo['xsec']}\n")
        #         f.writelines(f"module.sumW = {sampleInfo['sumW']}\n")
        #         f.writelines(f"module.sumSign = {sampleInfo['sumsign']}\n")
        #     else:
        #         f.writelines(f"module.IsDATA = True\n")
        #         f.writelines(f"module.DataStream = '{sample.split('_')[0]}'\n")
        #     f.writelines(f"module.SetEra('{era}')\n")
            
        #     if len(userflags) > 0:
        #         for flag in userflags:
        #             f.writelines("module.Userflags = {\n")
        #             f.writelines(f"\t'{flag}',\n")
        #             f.writelines("}\n")
        #     for path in samplePaths[i]:
        #         f.writelines(f"module.AddFile('{path}')\n")
                
        #     f.writelines(f"module.SetOutfilePath('output/hists_{i}.root')\n")
        #     if reduction > 1:
        #         f.writelines(f"from math import ceil\n")
        #         f.writelines(f"module.MaxEvent = ceil(int(module.fChain.GetEntries())/{int(reduction)})\n")
        #     f.writelines(f"module.Init()\n")
        #     f.writelines(f"module.initializeAnalyzer()\n")
        #     f.writelines(f"module.Loop()\n")
        #     f.writelines(f"module.WriteHist()\n")
        
        with open(os.path.join(working_dir,f"job{i}.cpp"),'w') as f:
            f.writelines(f"#include <algorithm>\n")
            f.writelines(f"void job{i}"+"(){\n")
            f.writelines(f"\t{argparse.Analyzer} module;\n")
            f.writelines(f"\tmodule.SetTreeName(\"Events\");\n")
            f.writelines(f"\tmodule.LogEvery = 1000;\n")
            if isMC:
                f.writelines(f"\tmodule.IsDATA = false;\n")
                f.writelines(f"\tmodule.MCSample = \"{sample}\";\n")
                f.writelines(f"\tmodule.xsec = {sampleInfo['xsec']};\n")
                f.writelines(f"\tmodule.sumW = {sampleInfo['sumW']};\n")
                f.writelines(f"\tmodule.sumSign = {sampleInfo['sumsign']};\n")
            else:
                f.writelines(f"\tmodule.IsDATA = true;\n")
                f.writelines(f"\tmodule.DataStream = \"{sample.split('_')[0]}\";\n")
            f.writelines(f"\tmodule.SetEra(\"{era}\");\n")
            if len(userflags) > 0:
                for flag in userflags:
                    f.writelines("\tmodule.Userflags = {\n")
                    f.writelines(f"\t\t\"{flag}\",\n")
                    f.writelines("\t};\n")
            for path in samplePaths[i]:
                f.writelines(f"\tmodule.AddFile(\"{path}\");\n")
                
            f.writelines(f"\tmodule.SetOutfilePath(\"{output}\");\n")
            if reduction > 1:
                f.writelines(f"\tmodule.MaxEvent = std::max(1,static_cast<int>(module.fChain->GetEntries()/{int(reduction)}));\n")
            f.writelines(f"\tmodule.Init();\n")
            f.writelines(f"\tmodule.initializeAnalyzer();\n")
            f.writelines(f"\tmodule.Loop();\n")
            f.writelines(f"\tmodule.WriteHist();\n")
            f.writelines("}")
            
    return working_dir, totalNumberOfJobs
            
def makeMainAnalyzerJobs(working_dir,abs_MasterDirectoryName,totalNumberOfJobs, argparse):
    nmax = argparse.NMax
    memory = argparse.Memory
    batchname = argparse.BatchName
    if batchname == "":
        batchname = argparse.Analyzer
    libpath = os.environ['LD_LIBRARY_PATH']
    libpath = libpath.split(":")
    libpath = [x for x in libpath if x != SKNANO_LIB]
    libpath.append(abs_MasterDirectoryName+'/lib')
    libpath = ":".join(libpath)
    
    with open(os.path.join(working_dir,"run.sh"),'w') as f:
        f.writelines("#!/bin/sh\n")
        f.writelines(f"cd {working_dir}\n") 
        f.writelines(f"export SKNANO_LIB=""\n")
        
        f.writelines(f"export LD_LIBRARY_PATH=""\n")
        f.writelines(f"export LD_LIBRARY_PATH={libpath}\n")
        #f.writelines(f"python3 job$1.py\n")
        f.writelines(f"root -l -b -q job$1.cpp\n")
        f.writelines(f"exit $?\n")

    #submit condor jobs
    job_dict = {}
    job_dict['executable'] = os.path.join(working_dir,"run.sh")
    job_dict['JobBatchName'] = f"{batchname}_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}"
    job_dict['universe'] = "vanilla"
    job_dict['getenv'] = "True"
    job_dict['RequestMemory'] = f'ifthenelse(isUndefined(MemoryUsage),{memory},(MemoryUsage * 2))' # 2 times of memory usage
    job_dict['arguments'] = "$(Process)"
    job_dict['output'] = os.path.join(working_dir,"job$(Process).out")
    job_dict['error'] = os.path.join(working_dir,"job$(Process).err")
    job_dict['should_transfer_files'] = "YES"
    job_dict['when_to_transfer_output'] = "ON_EXIT"
    job_dict['concurrency_limits'] = f"n{nmax}.{username}"
    job_dict['periodic_release'] = '(NumJobStarts < 3) && (HoldReasonCode == 34) && (JobStatus == 5)' # periodic release for 3 times // Held reason is lack of memeory // JobStatus is Hold // https://research.cs.wisc.edu/htcondor/manual/v8.5/12_Appendix_A.html

    
    return job_dict

def makeHaddJobs(working_dir,argparser):
    AnalyzerName = argparser.Analyzer
    if len(userflags) > 0:
        for flag in userflags:
            AnalyzerName += f"_{flag}"
    hadd_target = os.path.join(SKNANO_OUTPUT,AnalyzerName,working_dir.split('/')[-2],working_dir.split('/')[-1]+'.root')
    if not os.path.exists(os.path.join(SKNANO_OUTPUT,argparser.Analyzer,working_dir.split('/')[-2])):
        os.makedirs(os.path.join(SKNANO_OUTPUT,argparser.Analyzer,working_dir.split('/')[-2]))
    
    with open(os.path.join(working_dir,"hadd.sh"),'w') as f:
        f.writelines("#!/bin/bash\n")
        f.writelines(f"cd {working_dir}\n")
        f.writelines(f"hadd -f {hadd_target} output/hists_*.root\n")
        
    job_dict = {}
    job_dict['executable'] = os.path.join(working_dir,"hadd.sh")
    job_dict['JobBatchName'] = f"Hadd_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}"
    job_dict['universe'] = "vanilla"
    job_dict['getenv'] = "True"
    job_dict['RequestMemory'] = 2048
    job_dict['output'] = os.path.join(working_dir,"hadd.out")
    job_dict['error'] = os.path.join(working_dir,"hadd.err")
    job_dict['should_transfer_files'] = "YES"
    job_dict['when_to_transfer_output'] = "ON_EXIT"

    return job_dict

def makeSkimPostProcsJobs(working_dir,sample, argparser):
    AnalyzerName = argparser.Analyzer
    isMC, period = isMCandGetPeriod(sample)
    out_base, suffix = getSkimmingOutBaseAndSuffix(sample, AnalyzerName) 
    out_base = os.path.dirname(out_base)
    if isMC:
        with open(os.path.join(working_dir,"postproc.sh"),'w') as f:
            f.writelines("#!/bin/bash\n")
            f.writelines(f"mv {out_base} {os.path.join(os.path.dirname(out_base),out_base.split('/')[-1].replace('Temp_',''))}\n")
            f.writelines(f"cd $SKNANO_PYTHON\n")
            f.writelines(f"python3 sampleManager.py --era {era} --makeSkimTreeInfo --skimTreeFolder {os.path.dirname(out_base)} --skimTreeSuffix {AnalyzerName.replace('Skim_','')} --skimTreeOrigPD {sample}\n")
    else:
        target_dir = os.path.join(os.path.dirname(os.path.dirname(out_base)),os.path.dirname(out_base).split("/")[-1].replace('Temp_',''))
        if not os.path.exists(target_dir):
            os.makedirs(target_dir)
        with open(os.path.join(working_dir,"postproc.sh"),'w') as f:
            f.writelines("#!/bin/bash\n")
            f.writelines(f"mv {out_base} {target_dir}\n")
            f.writelines(f"cd $SKNANO_PYTHON\n")
            f.writelines(f"python3 sampleManager.py --era {era} --makeSkimTreeInfo --skimTreeFolder {os.path.dirname(target_dir)} --skimTreeSuffix {AnalyzerName.replace('Skim_','')} --skimTreeOrigPD {sample}\n")
            f.writelines(f"""if [ -z "$(ls -A {os.path.dirname(out_base)})" ]; then\n""")
            f.writelines(f"\trmdir {os.path.dirname(out_base)}\n") 
            f.writelines(f"fi")
     
    job_dict = {}
    job_dict['executable'] = os.path.join(working_dir,"postproc.sh")
    job_dict['JobBatchName'] = f"PostProc_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}"
    job_dict['universe'] = "vanilla"
    job_dict['getenv'] = "True"
    job_dict['RequestMemory'] = 1024
    job_dict['output'] = os.path.join(working_dir,"postproc.out")
    job_dict['error'] = os.path.join(working_dir,"postproc.err")
    job_dict['should_transfer_files'] = "YES"
    job_dict['when_to_transfer_output'] = "ON_EXIT"

    return job_dict
    


def getEachAnalyzerToPostDag(kwarg):
    era = kwarg['era']
    sample = kwarg['sample']
    analyzer_sub_dict = kwarg['analyzer_sub_dict']
    hadd_sub_dict = kwarg['hadd_sub_dict']
    totalNumberOfJobs = kwarg['totalNumberofJobs']

    working_dir = kwarg['working_dir']
    batchname = kwarg['batchname']
    
    if totalNumberOfJobs == 0:
        return
    
    analyzer_layer = {
        'name' : f"Analyzer_{batchname}",
        'submit_description': htcondor.Submit(analyzer_sub_dict),
        'vars' : [{"Process":str(i)} for i in range(totalNumberOfJobs)]
    }
    
    hadd_layer = {
        'name' :  f"Postproc_{batchname}" if SKIMMING_MODE else f"Hadd_{batchname}",
        'submit_description' : htcondor.Submit(hadd_sub_dict)
    }
    
    return (analyzer_layer,hadd_layer)
        
def getFinalDag(hadd_layer_dicts,skim_postproc_layers,master_dir,argparser):
    batchname = argparser.BatchName
    if batchname == "":
        batchname = argparser.Analyzer
    dag_dir = os.path.join(master_dir,"dags")
    os.makedirs(dag_dir)

    job_dict = {}
    job_dict['executable'] = os.path.join(dag_dir,"final.sh")
    job_dict['JobBatchName'] = f"Summarize"
    job_dict['universe'] = "vanilla"
    job_dict['getenv'] = "True"
    job_dict['RequestMemory'] = 1024
    job_dict['output'] = os.path.join(dag_dir,"final.out")
    job_dict['error'] = os.path.join(dag_dir,"final.err")
    job_dict['should_transfer_files'] = "YES"
    job_dict['when_to_transfer_output'] = "ON_EXIT"
    
    with open(os.path.join(dag_dir,"final.sh"),'w') as f:
        f.writelines("#!/bin/bash\n")
        f.writelines(f"cd {dag_dir}\n")
        f.writelines(f"python3 {os.environ['SKNANO_PYTHON']}/JobReporter.py --TOKEN {TOKEN} --chatID {chat_id} --master_dir {master_dir}\n")
    
    dag = dags.DAG()
    finaldag = dags.DAG()
    hadd_layers = []
    postproc_layers = []
    if SKIMMING_MODE:
        for layer_dict in skim_postproc_layers:
            if layer_dict is None:
                continue
            analyzer_dict, postproc_dict = layer_dict
            if analyzer_dict is None or postproc_dict is None:
                continue
            analyzer_layer = dag.layer(
                name = analyzer_dict['name'],
                submit_description = analyzer_dict['submit_description'],
                vars = analyzer_dict['vars']
            )
            postproc_layer = analyzer_layer.child_layer(
                name = postproc_dict['name'],
                submit_description = postproc_dict['submit_description']
            )
            postproc_layers.append(postproc_layer)
        #if Skimming, each postproc layer will not run concurrently b/c io
        for i, layer in enumerate(postproc_layers):
            if i == 0:
                continue
            postproc_layers[i-1].add_parents(postproc_layers[i])

    else:
        for layer_dict in hadd_layer_dicts:
            if layer_dict is None:
                continue
            analyzer_dict, hadd_dict = layer_dict
            if analyzer_dict is None or hadd_dict is None:
                continue
            analyzer_layer = dag.layer(
                name = analyzer_dict['name'],
                submit_description = analyzer_dict['submit_description'],
                vars = analyzer_dict['vars']
            )
            hadd_layer = analyzer_layer.child_layer(
                name = hadd_dict['name'],
                submit_description = hadd_dict['submit_description']
            )
            hadd_layers.append(hadd_layer)
    


    
    dag_file = dags.write_dag(dag,dag_dir)
    subdag = finaldag.subdag(name='main_Dag',dag_file=str(dag_file))
    finalNode = finaldag.final(name=job_dict['JobBatchName'],submit_description=job_dict)
    with open(os.path.join(dag_dir,job_dict['JobBatchName']+'.sub'),'w') as f:
        for key, value in job_dict.items():
            f.writelines(f"{key} = {value}\n")
        f.writelines(f"queue\n")

    

    print(dag.describe())
    print(finaldag.describe())
        
    
    finalDag_file = dags.write_dag(finaldag,dag_dir,'finaldag.dag')
    
    os.chdir(dag_dir)
    finalDag_submit = htcondor.Submit.from_dag(str(finalDag_file),{"force":1,"include_env":','.join(list(os.environ.keys())),"batch-name":batchname}) 
    cluster_id = htcondor.Schedd().submit(finalDag_submit).cluster()
    print(f"DAGMan job cluster is {cluster_id}")

    
    
    
if __name__ == '__main__':
    parser = setParser()
    args = parser.parse_args()
    eras = getEraList(args.Era)
    SKIMMING_MODE = args.skimming_mode
    userflags = getUserFlagsList(args.Userflags)
    timestamp, string_JobStartTime = getTimeStamp()
    _, abs_MasterDirectoryName= getMasterDirectoryName(timestamp, args.Analyzer, userflags)
    InputSamplelist = getInputSampleList(args.InputSample, eras)
    
    dag_list = []
    hadd_layers = []
    postproc_layers = []

    for era in eras:
        InputSamplelist_era = addPeriodToSampleList(InputSamplelist, era)
        for sample in InputSamplelist_era:
            working_dir, totalNumberofJobs = pythonJobProducer(era, sample, args, abs_MasterDirectoryName, userflags)
            if totalNumberofJobs == None:
                continue
            analyzer_sub_dict = makeMainAnalyzerJobs(working_dir,abs_MasterDirectoryName,totalNumberofJobs,args)
            if SKIMMING_MODE:
                postproc_sub_dict = makeSkimPostProcsJobs(working_dir,sample,args)
            else:
                hadd_sub_dict = makeHaddJobs(working_dir,args)
            
            if SKIMMING_MODE:
                dag_list.append({'era':era,'sample':sample,'analyzer_sub_dict':analyzer_sub_dict,'hadd_sub_dict':postproc_sub_dict,'totalNumberofJobs':totalNumberofJobs,'working_dir':working_dir,'batchname':f"{args.Analyzer}_{era}_{sample}"})
            else:
                dag_list.append({'era':era,'sample':sample,'analyzer_sub_dict':analyzer_sub_dict,'hadd_sub_dict':hadd_sub_dict,'totalNumberofJobs':totalNumberofJobs,'working_dir':working_dir,'batchname':f"{args.Analyzer}_{era}_{sample}"})
            if dag_list is not None:
                if SKIMMING_MODE:
                    postproc_layers.append(getEachAnalyzerToPostDag(dag_list[-1]))
                else:
                    hadd_layers.append(getEachAnalyzerToPostDag(dag_list[-1]))
            
    getFinalDag(hadd_layers, postproc_layers,abs_MasterDirectoryName, args)