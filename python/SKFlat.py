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
import datetime
from itertools import product
import json
##############################
#Global Variables
##############################
SKNANO_RUNLOG = os.environ['SKNANO_RUNLOG']
SKNANO_OUTPUT = os.environ['SKNANO_OUTPUT']
SKNANO_DATA = os.environ['SKNANO_DATA']
SKNANO_LIB = os.environ['SKNANO_LIB']
username = os.environ['USER']
##############################
#Load commonSampleInfo.json at start
sampleInfoJsons = {}
sampleInfoJsons['2022'] = json.load(open(os.path.join(SKNANO_DATA,'2022','Sample','CommonSampleInfo.json')))
sampleInfoJsons['2022EE'] = json.load(open(os.path.join(SKNANO_DATA,'2022EE','Sample','CommonSampleInfo.json')))
##############################
def getEraList(eras):
    eras = eras.split(",")
    for era in eras:
        if era not in ["2022","2022EE"]:
            print(f"ERROR: {era} is not a valid era")
            exit()
    return eras

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
    parser.add_argument('-n', dest='NJobs', default=1, type=int)
    #parser.add_argument('-o', dest='Outputdir', default="")
    #parser.add_argument('-q', dest='Queue', default="fastq")
    parser.add_argument('-e', dest='Era', default="2022",help="2022, 2022EE. can be comma separated")
    parser.add_argument('--userflags', dest='Userflags', default="")
    parser.add_argument('--nmax', dest='NMax', default=None, type=int, help="maximum running jobs")
    parser.add_argument('--reduction', dest='Reduction', default=1, type=float)
    parser.add_argument('--memory', dest='Memory', default=2048, type=float)
    parser.add_argument('--batchname',dest='BatchName', default="")
    parser.add_argument('--telegram', dest='telegram', default=False, action='store_true')
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

def getInputSampleList(inputArguments):
    #if string
    if inputArguments.endswith(".txt"):
        with open(inputArguments, 'r') as f:
            InputSampleList = f.readlines()
    else:
        InputSampleList = inputArguments.split(",")
    InputSampleList = [x.replace("\n","").replace(" ","") for x in InputSampleList if not x.startswith("#") or not x.startswith("\n")]
    return InputSampleList
def jobFileDivider(files,ngroup):
    if ngroup > len(files):
        ngroup = len(files)
    filegroups = [[] for i in range(ngroup)]
    for i, file in enumerate(files):
        filegroups[i%ngroup].append(file)
    return filegroups
 
def pythonJobProducer(era, sample, argparse, masterJobDirectory, userflags):
    working_dir = os.path.join(masterJobDirectory,era,sample)
    os.makedirs(working_dir)
    os.makedirs(os.path.join(working_dir,"output"))
    njobs = argparse.NJobs
    reduction = argparse.Reduction
    
    try:
        sampleInfo = sampleInfoJsons[era][sample]
    except:
        print(f"Error! {sample} is not exist in era {era}")
        exit()
    samplePaths = json.load(open(os.path.join(SKNANO_DATA,'2022','Sample','ForSNU',sample+'.json')))['path']
    samplePaths = jobFileDivider(samplePaths, njobs)
    
    totalNumberOfJobs = len(samplePaths)
    for i in range(totalNumberOfJobs):
        with open(os.path.join(working_dir,f"job{i}.py"),'w') as f:
            f.writelines(f"from ROOT import {argparse.Analyzer}\n")
            f.writelines(f"module = {argparse.Analyzer}()\n")
            f.writelines(f"module.SetTreeName('Events')\n")
            
            if sampleInfo['isMC']:
                f.writelines(f"module.IsDATA = False\n")
                f.writelines(f"module.MCSample = '{sample}'\n")
                f.writelines(f"module.xsec = {sampleInfo['xsec']}\n")
                f.writelines(f"module.sumWeight = {sampleInfo['sumW']}\n")
                f.writelines(f"module.sumSign = {sampleInfo['sumsign']}\n")
            else:
                f.writelines(f"module.IsDATA = True\n")
                f.writelines(f"module.DATAStream = {sample}\n")
            f.writelines(f"module.SetEra('{era}')\n")
            
            if len(userflags) > 0:
                for flag in userflags:
                    f.writelines("module.Userflags = {\n")
                    f.writelines(f"\t'{flag}',\n")
                    f.writelines("}\n")
            for path in samplePaths[i]:
                f.writelines(f"module.AddFile('{path}')\n")
                
            f.writelines(f"module.SetOutfilePath('output/hists_{i}.root')\n")
            if reduction > 1:
                f.writelines(f"module.MaxEvent = int(int(module.fChain.GetEntries())/{int(reduction)})\n")
            f.writelines(f"module.Init()\n")
            f.writelines(f"module.initializeAnalyzer()\n")
            f.writelines(f"module.Loop()\n")
            f.writelines(f"module.WriteHist()\n")
    return working_dir, totalNumberOfJobs
            
def makeMainAnalyzerJobs(working_dir,abs_MasterDirectoryName,totalNumberOfJobs, argparse):
    nmax = argparse.NMax
    memory = argparse.Memory
    batchname = argparse.BatchName
    if batchname == "":
        batchname = "SKNano_"+argparse.Analyzer
    libpath = os.environ['LD_LIBRARY_PATH']
    libpath = libpath.split(":")
    libpath = [x for x in libpath if x != SKNANO_LIB]
    libpath.append(abs_MasterDirectoryName+'/lib')
    libpath = ":".join(libpath)
    
    with open(os.path.join(working_dir,"run.sh"),'w') as f:
        f.writelines("#!/bin/bash\n")
        f.writelines(f"cd {working_dir}\n")
        f.writelines(f"unset LD_LIBRARY_PATH\n")
        f.writelines(f"export LD_LIBRARY_PATH={libpath}\n")
        f.writelines(f"python3 job$1.py\n")
        f.writelines(f"exit 0\n")

        
    
    #submit condor jobs
    job_dict = {}
    job_dict['executable'] = os.path.join(working_dir,"run.sh")
    job_dict['jobbatchname'] = f"{batchname}_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}"
    job_dict['universe'] = "vanilla"
    job_dict['getenv'] = "True"
    job_dict['RequestMemory'] = memory
    job_dict['arguments'] = "$(Process)"
    job_dict['output'] = os.path.join(working_dir,"job$(Process).out")
    job_dict['error'] = os.path.join(working_dir,"job$(Process).err")
    job_dict['should_transfer_files'] = "YES"
    job_dict['when_to_transfer_output'] = "ON_EXIT"
    job_dict['concurrency_limits'] = f"n{nmax}.{username}"
    
    sub = htcondor.Submit(job_dict)
    return sub

def makeHaddJobs(working_dir,argparse):
    hadd_target = os.path.join(SKNANO_OUTPUT,argparse.Analyzer,working_dir.split('/')[-2],working_dir.split('/')[-1]+'.root')
    if not os.path.exists(os.path.join(SKNANO_OUTPUT,argparse.Analyzer,working_dir.split('/')[-2])):
        os.makedirs(os.path.join(SKNANO_OUTPUT,argparse.Analyzer,working_dir.split('/')[-2]))
   
    with open(os.path.join(working_dir,"hadd.sh"),'w') as f:
        f.writelines("#!/bin/bash\n")
        f.writelines(f"cd {working_dir}\n")
        f.writelines(f"mkdir -p \n")
        f.writelines(f"hadd -f {hadd_target} output/hists_*.root\n")
        
    job_dict = {}
    job_dict['executable'] = os.path.join(working_dir,"hadd.sh")
    job_dict['jobbatchname'] = f"Hadd_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}"
    job_dict['universe'] = "vanilla"
    job_dict['getenv'] = "True"
    job_dict['RequestMemory'] = 2048
    job_dict['output'] = os.path.join(working_dir,"hadd.out")
    job_dict['error'] = os.path.join(working_dir,"hadd.err")
    job_dict['should_transfer_files'] = "YES"
    job_dict['when_to_transfer_output'] = "ON_EXIT"
    sub = htcondor.Submit(job_dict)
    return sub

def getDagMan(analyzer_sub,hadd_sub, totalNumberOfJobs, argparse, working_dir):
    dag = dags.DAG()
    
    analyzer_layer = dag.layer(
        name = f"Analyzer_{argparse.Analyzer}_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}",
        submit_description = analyzer_sub,
        vars = [{"Process":str(i)} for i in range(totalNumberOfJobs)]
    )
    
    hadd_layer = analyzer_layer.child_layer(
        name = f"Hadd_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}",
        submit_description = hadd_sub
    )
    dag_file = dags.write_dag(dag,working_dir)
    return dag_file

if __name__ == '__main__':
    parser = setParser()
    args = parser.parse_args()
    eras = getEraList(args.Era)
    if args.telegram:
        print("Warning: This is EXPERIMENTAL feature. You have to install telegram python package in your environment.")
        from telegram import Bot
        exit()
        
        
    userflags = getUserFlagsList(args.Userflags)
    timestamp, string_JobStartTime = getTimeStamp()
    _, abs_MasterDirectoryName= getMasterDirectoryName(timestamp, args.Analyzer, userflags)
    InputSamplelist = getInputSampleList(args.InputSample)
    
    dag_dict = {}
    for era, sample in product(eras, InputSamplelist):
        

        working_dir, totalNumberofJobs = pythonJobProducer(era, sample, args, abs_MasterDirectoryName, userflags)
        analyzer_subs = makeMainAnalyzerJobs(working_dir,abs_MasterDirectoryName,totalNumberofJobs,args)
        hadd_sub = makeHaddJobs(working_dir,args)
        dag_file = getDagMan(analyzer_subs,hadd_sub,totalNumberofJobs,args,working_dir)
        dag_submit = htcondor.Submit.from_dag(str(dag_file),{"force":1,"include_env":','.join(list(os.environ.keys())),"batch-name":f"SKNano_{args.Analyzer}_{era}_{sample}"})
        
        print(f"Submitting DAG: {dag_submit}")

        current_dir = os.getcwd()
        os.chdir(working_dir)
        schedd = htcondor.Schedd()
        cluster_id = schedd.submit(dag_submit).cluster()
        print(f"DAGMan job cluster is {cluster_id}")
        os.chdir(current_dir)
        

    
    

 
    
