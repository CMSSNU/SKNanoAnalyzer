#!/usr/bin/env python3
#This it the preliminary version of SKFlat.py
#Using htcondor python binding and DAGMAN workflow manager
import os, shutil
import warnings
import argparse
try:
    import htcondor
    from htcondor import dags
except ModuleNotFoundError:
    import htcondor2 as htcondor
    from htcondor2 import dags
import datetime
import json
import re
import sys
import socket
import subprocess
import tarfile
import hashlib
import glob
from pathlib import Path

from tqdm.rich import tqdm
from tqdm import TqdmExperimentalWarning

try:
    from rich.console import Console
    from rich.panel import Panel
    from rich.table import Table
    from rich import box

    _RICH_AVAILABLE = True
    console = Console()
except Exception:
    _RICH_AVAILABLE = False
    console = None

HERE = Path(__file__).resolve()
REPO_ROOT = HERE.parents[1]  # .../SKNANOAnalyzer_NanoV15
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from templates.job_dict import main_job, hadd_job, partial_merge_job, final_job
from python.sample_paths import resolve_sample_paths
from python.dag_edges import RaggedGrouper, group_sizes
from python.telegram_reporter import send_telegram_message, submission_message

dag = dags.DAG()
warnings.simplefilter("ignore", TqdmExperimentalWarning)

##############################
#Global Variables
##############################
SKNANO_HOME = os.environ['SKNANO_HOME']
SKNANO_RUNLOG = os.environ['SKNANO_RUNLOG']
SKNANO_OUTPUT = os.environ['SKNANO_OUTPUT']
SKNANO_DATA = os.environ['SKNANO_DATA']
SKNANO_LIB = os.environ['SKNANO_LIB']
SKNANO_INSTALLDIR = os.environ['SKNANO_INSTALLDIR']
SKNANO_RUN3_NANOAODPATH = os.environ['SKNANO_RUN3_NANOAODPATH']
SKNANO_RUN2_NANOAODPATH = os.environ['SKNANO_RUN2_NANOAODPATH']
username = os.environ['USER']
Run = {'2016preVFP':2,'2016postVFP':2,'2017':2,'2018':2,'2022':3,'2022EE':3, '2023':3, '2023BPix':3, '2024':3}
SKIMMING_MODE = False
SOURCE_SNAPSHOT_DIRNAME = "source_snapshot"
SOURCE_ARCHIVE_DIRNAME = "code_archive"
METADATA_SNAPSHOT_DIRNAME = os.path.join("metadata", "data_snapshot")
RUN_MANIFEST_NAME = "run_manifest.json"
SOURCE_SNAPSHOT_ENTRIES = [
    "AnalyzerFramework",
    "CommonAnalyzers",
    "AnalyzerTools",
    "DataFormats",
    "PyAnalyzers",
    "SKNanoCore",
    "python",
    "templates",
    "scripts",
    "cmake",
    "examples/AnalysisModule",
    "docs",
    "ModellingPatch",
    "CMakeLists.txt",
    "README.md",
    "setup.sh",
    "AGENTS.md",
    "CLAUDE.md",
    "Nano.yml",
    ".gitignore",
    ".gitmodules",
]
SOURCE_SNAPSHOT_IGNORE_NAMES = {
    ".git",
    ".cache",
    ".venv",
    ".vscode",
    "__pycache__",
    "build",
    "install",
    "external",
    "notebooks",
    "tmp",
    "cache",
    "logs",
    "dag",
    "dags",
    "output",
    "shards",
    "bvC_Bscore_beff70",
}
SOURCE_SNAPSHOT_IGNORE_SUFFIXES = (
    ".root",
    ".tar",
    ".tar.gz",
    ".tgz",
    ".tar.xz",
    ".zip",
    ".pdf",
    ".png",
    ".jpg",
    ".jpeg",
    ".gif",
    ".so",
    ".pcm",
    ".o",
    ".pyc",
    ".pkl",
    ".npz",
    ".npy",
    ".h5",
    ".hdf5",
    ".parquet",
)
MAX_SOURCE_SNAPSHOT_FILE_BYTES = 20 * 1024 * 1024

def getAnalysisModuleDirs():
    raw = os.environ.get("SKNANO_ANALYSIS_MODULE_DIRS", "")
    entries = raw.replace(";", os.pathsep).split(os.pathsep)
    result = []
    for entry in entries:
        if not entry:
            continue
        path = os.path.realpath(os.path.expanduser(entry))
        if os.path.isdir(path) and path not in result:
            result.append(path)
    return result

def getModuleSchemaVersions(module_dir):
    versions = {}
    for version_file in Path(module_dir).glob("*/schema/VERSION"):
        versions[version_file.parent.parent.name] = version_file.read_text(
            encoding="utf-8").strip()
    return versions

##############################
#Load commonSampleInfo.json at start
sampleInfoJsons = {}
for era in Run.keys():
    try:
        sampleInfoJsons[era] = json.load(open(os.path.join(SKNANO_DATA,era,'Sample','CommonSampleInfo.json')))
    except:
        print(f"\033[93mWarning: {era} CommonSampleInfo.json is not exist\033[0m")
        sampleInfoJsons[era] = {}
moduleSampleJsons = {era: {} for era in Run.keys()}
for module_dir in getAnalysisModuleDirs():
    patterns = [
        os.path.join(module_dir, "data", "Sample", "*", "*.json"),
        os.path.join(module_dir, "*", "data", "Sample", "*", "*.json"),
    ]
    for pattern in patterns:
        for sample_path in sorted(glob.glob(pattern)):
            era = Path(sample_path).parent.name
            if era not in Run:
                continue
            with open(sample_path, encoding="utf-8") as handle:
                payload = json.load(handle)
            name = payload.get("name", Path(sample_path).stem)
            if name in moduleSampleJsons[era]:
                raise RuntimeError(
                    f"duplicate external sample '{name}' for era {era}")
            sampleInfoJsons[era][name] = {
                key: value for key, value in payload.items()
                if key not in {"name", "path", "path_glob"}
            }
            moduleSampleJsons[era][name] = sample_path

# Skims are produced by an analysis, not by the backend, so their metadata
# lives in the module that produced them -- <module>/data/Skim/<era>/, with
# skimTreeInfo.json as the registry and one json per skimmed sample beside it.
SKIM_INFO_BASENAME = "skimTreeInfo.json"
skimInfoJsons = {era: {} for era in Run.keys()}
moduleSkimJsons = {era: {} for era in Run.keys()}
for module_dir in getAnalysisModuleDirs():
    patterns = [
        os.path.join(module_dir, "data", "Skim", "*", "*.json"),
        os.path.join(module_dir, "*", "data", "Skim", "*", "*.json"),
    ]
    for pattern in patterns:
        for skim_path in sorted(glob.glob(pattern)):
            era = Path(skim_path).parent.name
            if era not in Run:
                continue
            with open(skim_path, encoding="utf-8") as handle:
                payload = json.load(handle)
            if Path(skim_path).name == SKIM_INFO_BASENAME:
                overlap = set(payload) & set(skimInfoJsons[era])
                if overlap:
                    raise RuntimeError(
                        f"duplicate skim entries for era {era}: {sorted(overlap)}")
                skimInfoJsons[era].update(payload)
                continue
            # Keyed by filename, not by the "name" field: a data skim writes
            # one file per period and they all carry the period-less name.
            name = Path(skim_path).stem
            if name in moduleSkimJsons[era]:
                raise RuntimeError(
                    f"duplicate skim sample '{name}' for era {era}")
            moduleSkimJsons[era][name] = skim_path

##############################
def isMCandGetPeriod(sample):
    #if sample is ends with _one capital letter, it is data
    #also handle 2023 format like C_v1, C_v2, etc.
    sample_parts = sample.split("_")
    if len(sample_parts) >= 2:
        # Check for 2023 format: C_v1, C_v2, etc.
        if len(sample_parts) >= 2 and sample_parts[-2].isupper() and len(sample_parts[-2]) == 1 and sample_parts[-1].startswith('v'):
            return False, f"{sample_parts[-2]}_{sample_parts[-1]}"
        # Check for older format: single capital letter
        elif sample_parts[-1].isupper() and len(sample_parts[-1]) == 1:
            return False, sample_parts[-1]
    return True, None

def getSkimmingOutBaseAndSuffix(era, sample, AnalyzerName, userflags=None):
    isMC, period = isMCandGetPeriod(sample)
    userflags = userflags or []
    suffix_tag = f"_{'_'.join(userflags)}" if len(userflags) > 0 else ""
    skim_suffix = f"{AnalyzerName.replace('Skim_','')}{suffix_tag}"
    suffix = f"Temp_Skim_{skim_suffix}_{sample if isMC else sample.replace(f'_{period}','')}"
    if Run[era] == 2:
        out_base = os.path.join(SKNANO_RUN2_NANOAODPATH ,era,'MC' if isMC else 'DATA','Skim',os.environ['USER'],suffix,'' if isMC else f'Period{period}', 'tree.root') if SKIMMING_MODE else 'output/hists.root'
    elif Run[era] == 3:
        out_base = os.path.join(SKNANO_RUN3_NANOAODPATH ,era,'MC' if isMC else 'DATA','Skim',os.environ['USER'],suffix,'' if isMC else f'Period{period}', 'tree.root') if SKIMMING_MODE else 'output/hists.root'

    return out_base, suffix

def getFinalOutputPath(era, sample, argparser, userflags):
    if SKIMMING_MODE:
        out_base, _ = getSkimmingOutBaseAndSuffix(era, sample, argparser.Analyzer, userflags)
        out_dir = os.path.dirname(out_base)
        isMC, _ = isMCandGetPeriod(sample)
        if isMC:
            return os.path.join(os.path.dirname(out_dir), out_dir.split('/')[-1].replace('Temp_', ''))
        target_dir = os.path.join(
            os.path.dirname(os.path.dirname(out_dir)),
            os.path.dirname(out_dir).split("/")[-1].replace('Temp_', '')
        )
        return target_dir

    analyzer_name = argparser.Analyzer
    if len(userflags) > 0:
        analyzer_name += f"/{'_'.join(userflags)}"
    if getattr(argparser, 'no_hadd', False):
        return os.path.join(SKNANO_OUTPUT, analyzer_name, era, sample, 'hists_*.root')
    return os.path.join(SKNANO_OUTPUT, analyzer_name, era, sample + '.root')

def sourceSnapshotIgnore(src, names):
    ignored = []
    for name in names:
        path = os.path.join(src, name)
        if name in SOURCE_SNAPSHOT_IGNORE_NAMES:
            ignored.append(name)
            continue
        if name.endswith(SOURCE_SNAPSHOT_IGNORE_SUFFIXES):
            ignored.append(name)
            continue
        if os.path.isfile(path):
            try:
                if os.path.getsize(path) > MAX_SOURCE_SNAPSHOT_FILE_BYTES:
                    ignored.append(name)
            except OSError:
                ignored.append(name)
    return ignored

def sanitizeArchiveToken(value, fallback="unknown"):
    value = value or fallback
    value = re.sub(r"[^A-Za-z0-9._-]+", "_", value).strip("._-")
    return value or fallback

def sha256File(path):
    digest = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()

def makeSourceArchive(master_dir, snapshot_dir, git_info):
    archive_dir = os.path.join(master_dir, SOURCE_ARCHIVE_DIRNAME)
    os.makedirs(archive_dir, exist_ok=True)

    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    branch = sanitizeArchiveToken(git_info.get("branch", "unknown"))
    commit = sanitizeArchiveToken((git_info.get("commit") or "unknown")[:12])
    dirty_tag = "_dirty" if git_info.get("dirty") else ""
    archive_name = f"SKNanoAnalyzer_{timestamp}_{branch}_{commit}{dirty_tag}.tar.gz"
    archive_path = os.path.join(archive_dir, archive_name)

    with tarfile.open(archive_path, "w:gz") as tar:
        tar.add(snapshot_dir, arcname=Path(SKNANO_HOME).name)

    return {
        "path": archive_path,
        "format": "tar.gz",
        "sha256": sha256File(archive_path),
        "size_bytes": os.path.getsize(archive_path),
    }

def makeSourceSnapshot(master_dir):
    snapshot_dir = os.path.join(master_dir, SOURCE_SNAPSHOT_DIRNAME)
    os.makedirs(snapshot_dir, exist_ok=True)
    copied = []
    skipped = []
    for entry in SOURCE_SNAPSHOT_ENTRIES:
        source = os.path.join(SKNANO_HOME, entry)
        target = os.path.join(snapshot_dir, entry)
        if not os.path.exists(source):
            skipped.append(entry)
            continue
        if os.path.isdir(source):
            shutil.copytree(source, target, ignore=sourceSnapshotIgnore, dirs_exist_ok=True)
        else:
            os.makedirs(os.path.dirname(target), exist_ok=True)
            shutil.copy2(source, target)
        copied.append(entry)
    modules = []
    for module_dir in getAnalysisModuleDirs():
        module_name = os.path.basename(module_dir.rstrip(os.sep))
        target = os.path.join(snapshot_dir, "analysis_modules", module_name)
        shutil.copytree(module_dir, target, ignore=sourceSnapshotIgnore,
                        dirs_exist_ok=True)
        module_git = getGitInfo(module_dir)
        modules.append({
            "name": module_name,
            "source": module_dir,
            "snapshot": target,
            "git": module_git,
            "schema_versions": getModuleSchemaVersions(module_dir),
        })
    git_info = getGitInfo()
    archive = makeSourceArchive(master_dir, snapshot_dir, git_info)
    return {
        'path': snapshot_dir,
        'copied': copied,
        'skipped': skipped,
        'max_file_bytes': MAX_SOURCE_SNAPSHOT_FILE_BYTES,
        'archive': archive,
        'git': git_info,
        'analysis_modules': modules,
    }

def getGitInfo(repository=SKNANO_HOME):
    def run_git(args):
        try:
            return subprocess.run(
                ["git"] + args,
                cwd=repository,
                check=True,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            ).stdout.strip()
        except Exception:
            return ""

    status = run_git(["status", "--short"])
    return {
        'commit': run_git(["rev-parse", "HEAD"]),
        'branch': run_git(["rev-parse", "--abbrev-ref", "HEAD"]),
        'status_short': status.splitlines(),
        'dirty': bool(status),
    }

def validateInstalledAnalyzer(analyzer, python_mode=False):
    if python_mode:
        return
    manifest = os.path.join(SKNANO_INSTALLDIR, "share", "sknano",
                            "analyzers.manifest")
    try:
        with open(manifest) as handle:
            analyzers = {line.strip() for line in handle if line.strip()}
    except OSError as exc:
        raise RuntimeError(
            f"Analyzer manifest is unavailable ({manifest}); rebuild SKNano") from exc
    if analyzer not in analyzers:
        available = ", ".join(sorted(analyzers)) or "<none>"
        raise RuntimeError(
            f"Analyzer '{analyzer}' is not installed. Available analyzers: {available}")

def copyMetadataSnapshotFile(master_dir, source_path):
    if not source_path or not os.path.exists(source_path):
        return None
    source_real = os.path.realpath(source_path)
    data_real = os.path.realpath(SKNANO_DATA)
    if os.path.commonpath([source_real, data_real]) == data_real:
        relpath = os.path.relpath(source_real, data_real)
    else:
        relpath = os.path.join("analysis_modules", Path(source_path).parent.parent.parent.name,
                               Path(source_path).parent.name, Path(source_path).name)
    target = os.path.join(master_dir, METADATA_SNAPSHOT_DIRNAME, relpath)
    os.makedirs(os.path.dirname(target), exist_ok=True)
    shutil.copy2(source_path, target)
    return target

def snapshotResolvedInputs(master_dir, era, sample, sample_paths):
    """Record the files a sample actually resolved to.

    A sample json that carries a glob says which files to look for, not which
    files were there at submission time. Without this the run is no longer
    reproducible once the production grows, which is the one thing the old
    enumerated path lists were good at.
    """
    target = os.path.join(master_dir, METADATA_SNAPSHOT_DIRNAME,
                          "resolved_inputs", era, f"{sample}.json")
    os.makedirs(os.path.dirname(target), exist_ok=True)
    with open(target, "w", encoding="utf-8") as handle:
        json.dump({"era": era, "sample": sample,
                   "input_root": os.environ.get("SKNANO_INPUT_ROOT", ""),
                   "nfiles": len(sample_paths), "files": sample_paths},
                  handle, indent=2)
    return target

def snapshotSampleMetadata(master_dir, era, sample, sample_json_path):
    copied = []
    common_info = os.path.join(SKNANO_DATA, era, 'Sample', 'CommonSampleInfo.json')
    copied_path = copyMetadataSnapshotFile(master_dir, common_info)
    if copied_path:
        copied.append(copied_path)

    copied_path = copyMetadataSnapshotFile(master_dir, sample_json_path)
    if copied_path:
        copied.append(copied_path)

    if sample.startswith("Skim_"):
        # The registry that named this skim lives with the module that made it.
        for module_dir in getAnalysisModuleDirs():
            for pattern in (os.path.join(module_dir, "data", "Skim", era, SKIM_INFO_BASENAME),
                            os.path.join(module_dir, "*", "data", "Skim", era, SKIM_INFO_BASENAME)):
                for skim_info in sorted(glob.glob(pattern)):
                    copied_path = copyMetadataSnapshotFile(master_dir, skim_info)
                    if copied_path:
                        copied.append(copied_path)
    return copied

def writeRunManifest(master_dir, argparser, userflags, dag_list, source_snapshot, submit_result=None):
    manifest_path = os.path.join(master_dir, RUN_MANIFEST_NAME)
    manifest = {
        'schema_version': 1,
        'created_at': datetime.datetime.now().isoformat(timespec='seconds'),
        'user': username,
        'host': socket.gethostname(),
        'cwd': os.getcwd(),
        'argv': sys.argv,
        'master_dir': master_dir,
        'environment': {
            'SKNANO_HOME': SKNANO_HOME,
            'SKNANO_DATA': SKNANO_DATA,
            'SKNANO_INSTALLDIR': SKNANO_INSTALLDIR,
            'SKNANO_OUTPUT': SKNANO_OUTPUT,
            'SKNANO_RUNLOG': SKNANO_RUNLOG,
            'SINGULARITY_IMAGE': os.environ.get('SINGULARITY_IMAGE', ''),
        },
        'git': getGitInfo(),
        'source_snapshot': source_snapshot,
        'analysis_modules': source_snapshot.get('analysis_modules', []),
        'options': {
            'Analyzer': argparser.Analyzer,
            'InputSample': argparser.InputSample,
            'Era': argparser.Era,
            'Run': argparser.Run,
            'Period': argparser.Period,
            'Userflags': userflags,
            'NJobs': argparser.NJobs,
            'Reduction': argparser.Reduction,
            'NMax': argparser.NMax,
            'Memory': argparser.Memory,
            'ncpu': argparser.ncpu,
            'BatchName': argparser.BatchName,
            'python': argparser.python,
            'skimming_mode': SKIMMING_MODE,
            'failure_policy': argparser.FailurePolicy,
            'max_event_errors': argparser.MaxEventErrors,
            'no_hadd': argparser.no_hadd,
            'no_exec': argparser.no_exec,
        },
        'samples': [
            {
                'era': item['era'],
                'sample': item['sample'],
                'jobs': item['totalNumberofJobs'],
                'working_dir': item['working_dir'],
                'output': getFinalOutputPath(item['era'], item['sample'], argparser, userflags),
                'metadata_snapshot_files': item.get('metadata_snapshot_files', []),
            }
            for item in dag_list
        ],
        'submit': submit_result or {},
    }
    with open(manifest_path, 'w') as f:
        json.dump(manifest, f, indent=2, sort_keys=True)
        f.write("\n")
    return manifest_path

def getEraList(eras, runs):
    if runs == 'None':
        if eras == 'All':
            return Run.keys() 
        eras = eras.split(",")
        for era in eras:
            if era not in Run.keys():
                print('\033[91m'+f"ERROR: {era} is not a valid era"+'\033[0m')
                exit()
    else:
        print('\033[93m'+"Warning: --era option will be ignored because --run option is given"+'\033[0m')
        runs = runs.split(",")
        runs = [run.replace(" ","") for run in runs]
        eras = []
        if 'Run2' in runs:
            eras += [e for e, r in Run.items() if r == 2]
        if 'Run3' in runs:
            eras += [e for e, r in Run.items() if r == 3]
    return eras

def parsePeriodFilter(period_arg):
    period_tokens = [token.strip() for token in period_arg.split(",") if token.strip() != ""]
    if len(period_tokens) == 0:
        return None
    if len(period_tokens) == 1 and period_tokens[0].lower() == "all":
        return None
    return set(period_tokens)

def filterDataPeriods(periods, period_filter, era, sample_name):
    if period_filter is None:
        return periods
    selected_periods = [period for period in periods if period in period_filter]
    if len(selected_periods) == 0:
        print('\033[93m'+f"Warning: no matching periods for {sample_name} in era {era}. requested={sorted(period_filter)}, available={periods}"+'\033[0m')
    return selected_periods

def makeSampleList(samplelist,era,period_filter=None):
    #add Period to data sample, and wildcard search
    copylist = []
    for sample in samplelist:
        if sample.startswith("Skim_"):
            if SKIMMING_MODE:
                print('\033[91m'+f"Error: Skimmed sample {sample} is not allowed in skimming mode"+'\033[0m')
                exit()
            if '*' in sample:
                #wildcard search using regex
                sample = sample.replace('*','.*')
                sample = re.compile(sample)
                for sampleInfo in skimInfoJsons[era].keys():
                    if sample.match(sampleInfo):
                        if skimInfoJsons[era][sampleInfo]['isMC']:
                            copylist.append(sampleInfo)
                        else:
                            periods = filterDataPeriods(skimInfoJsons[era][sampleInfo]['periods'], period_filter, era, sampleInfo)
                            copylist += [f"{sampleInfo}_{period}" for period in periods]
                continue
            elif sample not in skimInfoJsons[era]:
                print('\033[93m'+f"Warning: {sample} is not exist in era {era}"+'\033[0m')
                continue
            skimInfo = skimInfoJsons[era][sample]
            sampleInfo = sampleInfoJsons[era][skimInfo['PD']]
            if sampleInfo['isMC']:
                copylist.append(sample)
            if not sampleInfo['isMC']:
                periods = filterDataPeriods(sampleInfo['periods'], period_filter, era, sample)
                copylist += [f"{sample}_{period}" for period in periods]
        else:
            if '*' in sample:
                #wildcard search using regex
                sample = sample.replace('*','.*')
                sample = re.compile(sample)
                for sampleInfo in sampleInfoJsons[era].keys():
                    if sample.match(sampleInfo):
                        if sampleInfoJsons[era][sampleInfo]['isMC']:
                            copylist.append(sampleInfo)
                        else:
                            periods = filterDataPeriods(sampleInfoJsons[era][sampleInfo]['periods'], period_filter, era, sampleInfo)
                            copylist += [f"{sampleInfo}_{period}" for period in periods]
                continue
            elif sample not in sampleInfoJsons[era]:
                print('\033[93m'+f"Warning: {sample} is not exist in era {era}"+'\033[0m')
                continue
            sampleInfo = sampleInfoJsons[era][sample]
            if sampleInfo['isMC']:
                copylist.append(sample)
            if not sampleInfo['isMC']:
                periods = filterDataPeriods(sampleInfo['periods'], period_filter, era, sample)
                copylist += [f"{sample}_{period}" for period in periods]

    return copylist

def getUserFlagsList(Userflags):
    UserflagsList = Userflags.split(",")
    UserflagsList = [x for x in UserflagsList if x != ""]
    return UserflagsList

def getExcludeRegexList(exclude_samples):
    if not exclude_samples:
        return []
    exclude_list = [x for x in exclude_samples.split(",") if x != ""]
    regex_list = []
    for pattern in exclude_list:
        pattern = pattern.replace('*','.*')
        try:
            regex_list.append(re.compile(pattern))
        except re.error as exc:
            print('\033[91m'+f"ERROR: invalid exclude regex '{pattern}': {exc}"+'\033[0m')
            exit()
    return regex_list

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
    parser.add_argument('-r', dest='Run', default="None",help="Run2, Run3. can be comma separated. override era option")
    parser.add_argument('-p', dest='Period', default="All",help="Data period filter for data samples (e.g. I or I,I_v2). Default: All")
    parser.add_argument('--userflags', dest='Userflags', default="")
    parser.add_argument('--exclude', dest='ExcludeSample', default="",
    help="Exclude samples by regex (comma-separated, supports * wildcard)")
    parser.add_argument('--nmax', dest='NMax', default=500, type=int, help="maximum running jobs")
    parser.add_argument('--reduction', dest='Reduction', default=1, type=float)
    parser.add_argument('--python', action="store_true", default=False,
    help="Use python analyzer")
    parser.add_argument('--memory', dest='Memory', default=2048, type=float)
    parser.add_argument('--ncpu', dest='ncpu', default=1, type=int) 
    parser.add_argument('--batchname', dest='BatchName', default="")
    parser.add_argument('--skimming_mode', action='store_true', default=False, help="Enable this option when anlyzer is skimmer.")
    parser.add_argument('--failure-policy', dest='FailurePolicy', default='fail-fast',
    choices=['fail-fast', 'skip-event'], help="Event failure handling policy. Default: fail-fast")
    parser.add_argument('--max-event-errors', dest='MaxEventErrors', default=1, type=int,
    help="Maximum event-local errors before failing a job. Use -1 for unlimited. Default: 1")
    parser.add_argument('--no-hadd', dest='no_hadd', action='store_true', default=False,
    help="Skip hadd and move the per-job ROOT outputs to a sample directory under SKNANO_OUTPUT")
    parser.add_argument('--merge-mode', dest='MergeMode', default='single', choices=['single','index'],
    help="single: concatenate each sample into one ROOT file. index: merge histograms only and publish <sample>.root.chain.json over the RNTuple shards, which skips the bulk copy entirely")
    parser.add_argument('--merge-group-size', dest='MergeGroupSize', default=0, type=int,
    help="Merge analyzer outputs in groups of this many jobs as soon as each group finishes, then merge the group results. 0 keeps the single merge job that waits for the whole sample. 100 is a good starting point")
    parser.add_argument('--merge-jobs', dest='MergeJobs', default=8, type=int,
    help="Independent hadd processes the final merge may run over disjoint batches. Never becomes hadd's own -j, which corrupts output")
    parser.add_argument('--merge-cache-size', dest='MergeCacheSize', default='2g',
    help="hadd -cachesize for the final merge stage (0 disables). hadd clamps this at 2 GiB")
    parser.add_argument('--merge-batch-cache-size', dest='MergeBatchCacheSize', default='512m',
    help="hadd -cachesize for each concurrent merge process, both group merges and batch merges; budget --merge-jobs times this against the merge job's memory request")
    parser.add_argument('--no_exec', action='store_true', default=False, help="only produce working area, not submitting to the condor pool")
    
    #Note: this option will change the behavior of the script. output directory will be changed to Your GV0, hadd will be disabled, and will create the info json of skimmed tree   
    return parser

def getMasterDirectoryName(timeStamp, Analyzer, Userflags):
    MasterDirectoryName = f"{timeStamp}_{Analyzer}"
    if len(Userflags) > 0:
        for flag in Userflags:
            MasterDirectoryName += f"_{flag}"
    abs_MasterDirectoryName = os.path.join(SKNANO_RUNLOG,MasterDirectoryName)
    print(f"Creating Master Working Directory: {abs_MasterDirectoryName}")
    #print(f"Copy library files to {MasterDirectoryName.split('/')}")
    #os.makedirs(abs_MasterDirectoryName)
    shutil.copytree(SKNANO_INSTALLDIR, abs_MasterDirectoryName) 
    #os.system(f"cp -r {SKNANO_INSTALLDIR} {abs_MasterDirectoryName}")
    #print("...Done")
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
 
def jobProducer(era, sample, argparse, masterJobDirectory, userflags, isample, totsamples):
    isMC, period = isMCandGetPeriod(sample)
    AnalyzerName = argparse.Analyzer

        
    working_dir = os.path.join(masterJobDirectory,era,sample)
        
    os.makedirs(working_dir)
    if SKIMMING_MODE:
        out_base, suffix = getSkimmingOutBaseAndSuffix(era, sample, AnalyzerName, userflags)
        if not os.path.exists(os.path.dirname(out_base)):
            os.makedirs(os.path.dirname(out_base))
    else:
        os.makedirs(os.path.join(working_dir,"output"))
        out_base = os.path.join(working_dir,"output","hists.root")
    njobs = argparse.NJobs
    reduction = argparse.Reduction
    
    if sample.startswith("Skim_"):
        SkimInfo = skimInfoJsons[era][sample if isMC else re.sub(f"_{re.escape(period)}$", "", sample)]
        sampleInfo = sampleInfoJsons[era][SkimInfo['PD']]
        sample_json_path = moduleSkimJsons[era].get(sample)
        if not sample_json_path:
            raise RuntimeError(
                f"no skim metadata for '{sample}' in era {era}; it belongs in "
                "<module>/data/Skim/<era>/ of the analysis that produced it")
        samplePaths = resolve_sample_paths(json.load(open(sample_json_path)), era, period)
        metadata_snapshot_files = snapshotSampleMetadata(masterJobDirectory, era, sample, sample_json_path)
        metadata_snapshot_files.append(
            snapshotResolvedInputs(masterJobDirectory, era, sample, samplePaths))
        sample = SkimInfo['PD']
    else:
        sample_key = sample if isMC else re.sub(f"_{re.escape(period)}$", "", sample)
        sampleInfo = sampleInfoJsons[era][sample_key]
        # A module may ship its own sample json. Otherwise the registry entry is
        # the whole description: the inputs derive from era plus PD (MC) or
        # name and period (DATA), so there is no per-sample file to read.
        sample_json_path = moduleSampleJsons[era].get(sample)
        if sample_json_path:
            sample_spec = json.load(open(sample_json_path))
        else:
            sample_spec = dict(sampleInfo, name=sample_key)
            sample_json_path = os.path.join(
                SKNANO_DATA, era, 'Sample', 'CommonSampleInfo.json')
        samplePaths = resolve_sample_paths(sample_spec, era, period)
        metadata_snapshot_files = snapshotSampleMetadata(masterJobDirectory, era, sample, sample_json_path)
        metadata_snapshot_files.append(
            snapshotResolvedInputs(masterJobDirectory, era, sample, samplePaths))

    samplePaths = jobFileDivider(samplePaths, njobs)

    
    totalNumberOfJobs = len(samplePaths)

    for i in tqdm(range(totalNumberOfJobs), position=1, leave=False, desc=f"Creating Jobs for {sample}, ({isample}/{totsamples})", smoothing=1.):
        output = out_base.replace('.root',f'_{i}.root')
        # Read the template file
        if argparse.python:
            template_path = os.path.join(SKNANO_HOME, "templates", "job.py")
            with open(template_path, 'r') as f:
                job_content = f.read()

            # Replace template variations
            job_content = job_content.replace("[Analyzer]", argparse.Analyzer)
            job_content = job_content.replace("[ncpu]", str(argparse.ncpu))
            job_content = job_content.replace("[era]", era)
            job_content = job_content.replace("[period]", period if period else "")

            if isMC:
                job_content = job_content.replace("[sample]", sample)
                job_content = job_content.replace("[xsec]", str(sampleInfo["xsec"]))
                job_content = job_content.replace("[sumW]", str(sampleInfo["sumW"]))
                job_content = job_content.replace("[sumSign]", str(sampleInfo["sumsign"]))
            else:
                job_content = job_content.replace("module.IsDATA = False", "module.IsDATA = True")
                job_content = job_content.replace('module.MCSample = "[sample]"',
                                                  f'module.DataStream = "{sample.split("_")[0]}"')
                job_content = job_content.replace("    module.xsec = [xsec]\n", "")
                job_content = job_content.replace("    module.sumW = [sumW]\n", "")
                job_content = job_content.replace("    module.sumSign = [sumSign]\n", "")
            
            # Handle userflags
            if userflags:
                userflags_str = "    module.Userflags = RVec(TString)(["
                userflags_str += ", ".join([f'"{flag}"' for flag in userflags])
                userflags_str += "])"
            else:
                userflags_str = ""
            job_content = job_content.replace("[USERFLAGS]", userflags_str)
            
            # Handle sample paths
            samplepaths_str = "\n".join([f'    module.AddFile("{path}")' for path in samplePaths[i]])
            job_content = job_content.replace("[SAMPLEPATHS]", samplepaths_str)

            # Handle reduction/max events
            maxevent_str = f'    module.MaxEvent = max(1, int(module.GetInputEntries()/{int(reduction)}))'
            job_content = job_content.replace("[MAXEVENT]", maxevent_str)

            # Set output path
            job_content = job_content.replace("[output]", output)
            job_content = job_content.replace("[failure_policy]", argparse.FailurePolicy)
            job_content = job_content.replace("[max_event_errors]", str(argparse.MaxEventErrors))
            job_filename = os.path.join(working_dir, f"job_{i+1}.py")
            with open(job_filename, 'w') as f:
                f.write(job_content)
        else:
            template_path = os.path.join(SKNANO_HOME, "templates", "job.cc")
            with open(template_path, 'r') as f:
                job_content = f.read()
            
            # Replace template variables
            job_content = job_content.replace("[jobname]", f"job_{i+1}")
            job_content = job_content.replace("[ncpu]", str(argparse.ncpu))
            job_content = job_content.replace("[analyzer]", argparse.Analyzer)
            job_content = job_content.replace("[era]", era)
            job_content = job_content.replace("[period]", period if period else "")

            if isMC:
                job_content = job_content.replace("[sample]", sample)
                job_content = job_content.replace("[xsec]", str(sampleInfo["xsec"]))
                job_content = job_content.replace("[sumW]", str(sampleInfo["sumW"]))
                job_content = job_content.replace("[sumSign]", str(sampleInfo["sumsign"]))
            else:
                # For data, remove MC-specific lines
                job_content = job_content.replace("module.IsDATA = false;", "module.IsDATA = true;")
                job_content = job_content.replace('module.MCSample = "[sample]";', f'module.DataStream = "{sample.split("_")[0]}";')
                job_content = job_content.replace("module.xsec = [xsec];", "")
                job_content = job_content.replace("module.sumW = [sumW];", "") 
                job_content = job_content.replace("module.sumSign = [sumSign];", "")

            # Handle userflags
            if userflags:
                userflags_str = "module.Userflags = {\n"
                userflags_str += "".join([f'\t"{flag}",\n' for flag in userflags])
                userflags_str += "    };"
            else:
                userflags_str = ""
            job_content = job_content.replace("[USERFLAGS]", userflags_str)

            # Handle sample paths
            samplepaths_str = "\n".join([f'\tmodule.AddFile("{path}");' for path in samplePaths[i]])
            job_content = job_content.replace("[SAMPLEPATHS]", samplepaths_str)

            # Handle reduction/max events
            maxevent_str = f'\tmodule.MaxEvent = std::max<Long64_t>(1, module.GetInputEntries()/{int(reduction)});'
            job_content = job_content.replace("[MAXEVENT]", maxevent_str)

            # Set output path
            job_content = job_content.replace("[output]", output)
            job_content = job_content.replace("[failure_policy]", argparse.FailurePolicy)
            job_content = job_content.replace("[max_event_errors]", str(argparse.MaxEventErrors))
            job_filename = os.path.join(working_dir, f"job_{i+1}.cc")
            with open(job_filename, 'w') as f:
                f.write(job_content)
            
    return working_dir, totalNumberOfJobs, metadata_snapshot_files
            
def makeMainAnalyzerJobs(working_dir,abs_MasterDirectoryName,totalNumberOfJobs, argparse):
    nmax = argparse.NMax
    memory = argparse.Memory
    batchname = argparse.BatchName
    userflags = getUserFlagsList(argparse.Userflags)
    ncpu = argparse.ncpu
    if batchname == "":
        batchname = argparse.Analyzer
        if len(userflags) > 0:
            for flag in userflags:
                batchname += f"_{flag}"
    libpath = os.environ['LD_LIBRARY_PATH']
    libpath = libpath.split(":")
    libpath = [x for x in libpath if x != SKNANO_LIB]
    libpath = [os.path.join(abs_MasterDirectoryName,'lib')]+libpath
    libpath = ":".join(libpath)
    install_include = os.path.join(abs_MasterDirectoryName, 'include')
    original_home = os.path.abspath(SKNANO_HOME)
    original_install = os.path.abspath(SKNANO_INSTALLDIR)
    incl_entries = [install_include]
    for entry in os.environ.get('ROOT_INCLUDE_PATH', '').split(":"):
        if not entry:
            continue
        abs_entry = os.path.abspath(entry)
        if abs_entry.startswith(original_home) or abs_entry.startswith(original_install):
            continue
        if entry not in incl_entries:
            incl_entries.append(entry)
    inclpath = ":".join(incl_entries)
    
    run_template = "run.python.sh" if argparse.python else "run.sh"
    template_path = os.path.join(SKNANO_HOME, "templates", run_template)
    with open(template_path, 'r') as f:
        run_content = f.read()
    #mamba_bin_path = os.environ['MAMBA_EXE']
    #mamba_bin_path = os.path.dirname(mamba_bin_path)
    # We need to fetch the MAMBA_ROOT_PREFIX from the singularity image if running in a singularity container
    # Currently, we assume that /opt/conda is the MAMBA_ROOT_PREFIX
    singularity_image = os.environ["SINGULARITY_IMAGE"]
    mamba_root_prefix = "/opt/conda" if singularity_image else os.environ['MAMBA_ROOT_PREFIX']
    run_content = run_content.replace("[MAMBA_BIN_PATH]", os.path.join(mamba_root_prefix, "bin"))
    run_content = run_content.replace("[MAMBA_ROOT_PREFIX]", mamba_root_prefix)
    snapshot_home = os.path.join(abs_MasterDirectoryName, SOURCE_SNAPSHOT_DIRNAME)
    lhapdf_include_dir = os.environ.get('LHAPDF_INCLUDE_DIR', os.path.join(SKNANO_HOME, 'external', 'lhapdf', 'redhat', 'include'))
    lhapdf_lib_dir = os.environ.get('LHAPDF_LIB_DIR', os.path.join(SKNANO_HOME, 'external', 'lhapdf', 'redhat', 'lib'))
    lhapdf_bin_dir = os.path.join(os.path.dirname(os.path.dirname(lhapdf_lib_dir)), 'bin')
    run_content = run_content.replace("[SKNANO_HOME]", snapshot_home)
    run_content = run_content.replace("[SKNANO_DATA]", SKNANO_DATA)
    run_content = run_content.replace("[LHAPDF_BIN_DIR]", lhapdf_bin_dir)
    run_content = run_content.replace("[LHAPDF_INCLUDE_DIR]", lhapdf_include_dir)
    run_content = run_content.replace("[LHAPDF_LIB_DIR]", lhapdf_lib_dir)
    run_content = run_content.replace("[JSONPOG_REPO_PATH]", os.environ.get('JSONPOG_REPO_PATH', os.path.join(SKNANO_HOME, 'CMS_corrections')))
    run_content = run_content.replace("[ROCCOR_PATH]", os.environ.get('ROCCOR_PATH', os.path.join(SKNANO_HOME, 'external', 'RoccoR')))
    run_content = run_content.replace("[WORKDIR]", working_dir)
    run_content = run_content.replace("[SKNANO_RUNLOG_LIB]", os.path.join(abs_MasterDirectoryName, 'lib'))
    run_content = run_content.replace("[ROOT_INCLUDE_PATH]", inclpath)
    with open(os.path.join(working_dir,"run.sh"),'w') as f:
        f.write(run_content)

    #submit condor jobs
    job_dict = main_job.copy()
    job_dict['JobBatchName'] = f"{batchname}_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}"
    job_dict['executable'] = os.path.join(working_dir,"run.sh")
    job_dict['RequestMemory'] = f'ifthenelse(isUndefined(MemoryUsage),{memory},(MemoryUsage * 2))' # 2 times of memory usage
    job_dict['RequestCpus'] = ncpu
    job_dict['output'] = os.path.join(working_dir,"job_$(Process).out")
    job_dict['error'] = os.path.join(working_dir,"job_$(Process).err")
    job_dict['concurrency_limits'] = f"n{nmax}.{username}"
    
    return job_dict

def makeHaddJobs(working_dir,argparser,sample,totalNumberofJobs):
    """Build a sample's merge jobs.

    Returns ``(final_job, partial_job, sizes)``.  ``partial_job`` is None unless
    --merge-group-size asks for group merges; those run as soon as their own
    analyzer jobs finish instead of waiting for the whole sample, which is what
    keeps the merge off the critical path.
    """
    AnalyzerName = argparser.Analyzer
    userflags = getUserFlagsList(argparser.Userflags)
    if len(userflags) > 0:
        AnalyzerName += f"/{'_'.join(userflags)}"
    era = working_dir.split('/')[-2]
    hadd_target = os.path.join(SKNANO_OUTPUT,AnalyzerName,era,sample+'.root')
    if not os.path.exists(os.path.dirname(hadd_target)):
        os.makedirs(os.path.dirname(hadd_target))

    sizes = group_sizes(totalNumberofJobs, argparser.MergeGroupSize)
    grouped = argparser.MergeGroupSize > 0 and len(sizes) > 1

    partial_job = None
    if grouped:
        template_path = os.path.join(SKNANO_HOME, "templates", "merge_partial.sh")
        with open(template_path, 'r') as f:
            partial_content = f.read()
        partial_content = partial_content.replace("[WORKDIR]", working_dir)
        partial_content = partial_content.replace("[SKNANO_HOME]", SKNANO_HOME)
        partial_content = partial_content.replace("[GROUP_SIZE]", str(argparser.MergeGroupSize))
        partial_content = partial_content.replace("[NJOBS]", str(totalNumberofJobs))
        partial_content = partial_content.replace("[CACHE_SIZE]", argparser.MergeBatchCacheSize)
        partial_script = os.path.join(working_dir, "merge_partial.sh")
        with open(partial_script, 'w') as f:
            f.write(partial_content)

        partial_job = partial_merge_job.copy()
        partial_job['executable'] = partial_script
        partial_job['JobBatchName'] = f"Merge_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}"
        partial_job['output'] = os.path.join(working_dir,"merge_$(Group).out")
        partial_job['error'] = os.path.join(working_dir,"merge_$(Group).err")

    shard_dir = os.path.join(SKNANO_OUTPUT,AnalyzerName,era,sample)
    if argparser.MergeMode == 'index':
        os.makedirs(shard_dir, exist_ok=True)
        mode_args = f"--shard-dir {shard_dir}"
        # index mode republishes the shards rather than consuming them.
        delete_flag = ""
    else:
        mode_args = ""
        delete_flag = "--delete-inputs"

    template_path = os.path.join(SKNANO_HOME, "templates", "hadd.sh")
    with open(template_path, 'r') as f:
        hadd_content = f.read()
    hadd_content = hadd_content.replace("[WORKDIR]", working_dir)
    hadd_content = hadd_content.replace("[SKNANO_HOME]", SKNANO_HOME)
    hadd_content = hadd_content.replace("[TARGET]", hadd_target)
    hadd_content = hadd_content.replace(
        "[INPUT_GLOB]", "output/partial_*.root" if grouped else "output/hists_*.root")
    hadd_content = hadd_content.replace("[MERGE_MODE]", argparser.MergeMode)
    hadd_content = hadd_content.replace("[MODE_ARGS]", mode_args)
    hadd_content = hadd_content.replace("[MERGE_JOBS]", str(argparser.MergeJobs))
    hadd_content = hadd_content.replace("[CACHE_SIZE]", argparser.MergeCacheSize)
    hadd_content = hadd_content.replace("[BATCH_CACHE_SIZE]", argparser.MergeBatchCacheSize)
    hadd_content = hadd_content.replace("[DELETE_FLAG]", delete_flag)
    hadd_content = hadd_content.replace("[PROVENANCE]", os.path.join(os.path.dirname(os.path.dirname(working_dir)), RUN_MANIFEST_NAME))
    hadd_content = hadd_content.replace("[TARGET_PROVENANCE]", hadd_target + ".provenance.json")
    with open(os.path.join(working_dir,"hadd.sh"),'w') as f:
        f.write(hadd_content)

    job_dict = hadd_job.copy()
    job_dict['executable'] = os.path.join(working_dir,"hadd.sh")
    job_dict['JobBatchName'] = f"Hadd_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}"
    job_dict['output'] = os.path.join(working_dir,"hadd.out")
    job_dict['error'] = os.path.join(working_dir,"hadd.err")

    return job_dict, partial_job, (sizes if grouped else None)

def makeMoveJobs(working_dir,argparser,sample):
    AnalyzerName = argparser.Analyzer
    userflags = getUserFlagsList(argparser.Userflags)
    if len(userflags) > 0:
        AnalyzerName += f"/{'_'.join(userflags)}"
    era = working_dir.split('/')[-2]
    target_dir = os.path.join(SKNANO_OUTPUT, AnalyzerName, era, sample)
    os.makedirs(target_dir, exist_ok=True)

    template_path = os.path.join(SKNANO_HOME, "templates", "move.sh")
    with open(template_path, 'r') as f:
        move_content = f.read()
    move_content = move_content.replace("[WORKDIR]", working_dir)
    move_content = move_content.replace("[TARGETDIR]", target_dir)
    move_content = move_content.replace(
        "[PROVENANCE]",
        os.path.join(os.path.dirname(os.path.dirname(working_dir)), RUN_MANIFEST_NAME),
    )
    with open(os.path.join(working_dir,"move.sh"),'w') as f:
        f.write(move_content)

    job_dict = hadd_job.copy()
    job_dict['executable'] = os.path.join(working_dir,"move.sh")
    job_dict['JobBatchName'] = f"Move_{working_dir.split('/')[-1]}_{working_dir.split('/')[-2]}"
    job_dict['RequestCpus'] = 1
    job_dict['RequestMemory'] = '1024 MB'
    job_dict['output'] = os.path.join(working_dir,"move.out")
    job_dict['error'] = os.path.join(working_dir,"move.err")

    return job_dict

def makeSkimPostProcsJobs(working_dir,sample, argparser,era):
    AnalyzerName = argparser.Analyzer
    userflags = getUserFlagsList(argparser.Userflags)
    skim_suffix = AnalyzerName.replace('Skim_','')
    if len(userflags) > 0:
        skim_suffix += f"_{'_'.join(userflags)}"
    isMC, period = isMCandGetPeriod(sample)
    out_base, suffix = getSkimmingOutBaseAndSuffix(era, sample, AnalyzerName, userflags) 
    out_base = os.path.dirname(out_base)
    manifest_path = os.path.join(os.path.dirname(os.path.dirname(working_dir)), RUN_MANIFEST_NAME)
    if isMC:
        target_dir = os.path.join(os.path.dirname(out_base),out_base.split('/')[-1].replace('Temp_',''))
        with open(os.path.join(working_dir,"postproc.sh"),'w') as f:
            f.writelines("#!/bin/bash\n")
            f.writelines(f"mv {out_base} {target_dir}\n")
            f.writelines(f"cp {manifest_path} {os.path.join(target_dir, 'provenance.json')}\n")
            f.writelines(f"cd $SKNANO_PYTHON\n")
            f.writelines(f"python3 sampleManager.py --era {era} --makeSkimTreeInfo --skimTreeFolder {os.path.dirname(out_base)} --skimTreeSuffix {skim_suffix} --skimTreeOrigPD {sample}\n")
    else:
        target_dir = os.path.join(os.path.dirname(os.path.dirname(out_base)),os.path.dirname(out_base).split("/")[-1].replace('Temp_',''))
        if not os.path.exists(target_dir):
            os.makedirs(target_dir)
        with open(os.path.join(working_dir,"postproc.sh"),'w') as f:
            f.writelines("#!/bin/bash\n")
            f.writelines(f"mv {out_base} {target_dir}\n")
            f.writelines(f"cp {manifest_path} {os.path.join(target_dir, 'provenance.json')}\n")
            f.writelines(f"cd $SKNANO_PYTHON\n")
            f.writelines(f"python3 sampleManager.py --era {era} --makeSkimTreeInfo --skimTreeFolder {os.path.dirname(target_dir)} --skimTreeSuffix {skim_suffix} --skimTreeOrigPD {sample}\n")
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
    analyzer_sub_dict = kwarg['analyzer_sub_dict']
    hadd_sub_dict = kwarg['hadd_sub_dict']
    partial_sub_dict = kwarg.get('partial_sub_dict')
    merge_group_sizes = kwarg.get('merge_group_sizes')
    totalNumberOfJobs = kwarg['totalNumberofJobs']
    batchname = kwarg['batchname']
    postproc_label = kwarg.get('postproc_label', 'Hadd')
    
    if totalNumberOfJobs == 0:
        return
    
    analyzer_layer = {
        'name' : f"Analyzer_{batchname}",
        'submit_description': htcondor.Submit(analyzer_sub_dict),
        'vars' : [{"Process":str(i)} for i in range(1,totalNumberOfJobs+1)]
    }

    # Analyzer node index j wrote output/hists_j.root, so consecutive chunks of
    # that layer are exactly the shards one group merge owns.
    partial_layer = None
    if partial_sub_dict is not None and merge_group_sizes:
        partial_layer = {
            'name' : f"Merge_{batchname}",
            'submit_description' : htcondor.Submit(partial_sub_dict),
            'vars' : [{"Group":str(group)} for group in range(len(merge_group_sizes))],
            'group_sizes' : merge_group_sizes,
        }
    
    hadd_layer = {
        'name' :  f"Postproc_{batchname}" if SKIMMING_MODE else f"{postproc_label}_{batchname}",
        'submit_description' : htcondor.Submit(hadd_sub_dict)
    }
    
    return (analyzer_layer,partial_layer,hadd_layer)

def renderSubmissionSummary(dag_list, master_dir, argparser, userflags, submit_result):
    total_samples = len(dag_list)
    total_jobs = sum(item['totalNumberofJobs'] for item in dag_list)
    eras = sorted({item['era'] for item in dag_list})
    cluster_id = submit_result.get('cluster_id') if submit_result else None
    dag_file = submit_result.get('dag_file') if submit_result else None
    status = "Prepared only (--no_exec)" if argparser.no_exec else f"Submitted cluster {cluster_id}"
    if SKIMMING_MODE:
        hadd_status = "not applicable (skimming)"
    elif argparser.no_hadd:
        hadd_status = "disabled"
    else:
        hadd_status = f"{argparser.MergeMode}, {argparser.MergeJobs} processes"
        groups = sum(
            len(group_sizes(item['totalNumberofJobs'], argparser.MergeGroupSize))
            for item in dag_list
            if item.get('merge_group_sizes')
        )
        hadd_status += (
            f", pipelined in {groups} groups of {argparser.MergeGroupSize}"
            if groups else ", single merge after all jobs"
        )

    if not _RICH_AVAILABLE:
        print("\nSKNano submission summary")
        print(f"  Status: {status}")
        print(f"  Analyzer: {argparser.Analyzer}")
        print(f"  Eras: {', '.join(eras) if eras else '-'}")
        print(f"  Samples: {total_samples}")
        print(f"  Analyzer jobs: {total_jobs}")
        print(f"  Hadd: {hadd_status}")
        print(f"  Master directory: {master_dir}")
        if dag_file:
            print(f"  Final DAG: {dag_file}")
        if submit_result and submit_result.get('manifest_path'):
            print(f"  Manifest: {submit_result['manifest_path']}")
        if submit_result and submit_result.get('source_snapshot'):
            source_snapshot = submit_result['source_snapshot']
            print(f"  Source snapshot: {source_snapshot.get('path', '')}")
            if source_snapshot.get('archive'):
                print(f"  Source archive: {source_snapshot['archive'].get('path', '')}")
        return

    overview = Table.grid(padding=(0, 2))
    overview.add_column(style="bold cyan", no_wrap=True)
    overview.add_column()
    overview.add_row("Status", status)
    overview.add_row("Analyzer", argparser.Analyzer)
    overview.add_row("Mode", "python" if argparser.python else "C++ ROOT macro")
    overview.add_row("Skimming", str(SKIMMING_MODE))
    overview.add_row("Hadd", hadd_status)
    overview.add_row("Eras", ", ".join(eras) if eras else "-")
    overview.add_row("Samples", str(total_samples))
    overview.add_row("Analyzer jobs", str(total_jobs))
    overview.add_row("Concurrency", f"{argparser.NMax} per user")
    overview.add_row("Memory", f"{argparser.Memory} MB base")
    overview.add_row("CPUs/job", str(argparser.ncpu))
    overview.add_row("Failure policy", f"{argparser.FailurePolicy}, max errors={argparser.MaxEventErrors}")
    overview.add_row("Master dir", master_dir)
    if dag_file:
        overview.add_row("Final DAG", dag_file)
    if submit_result and submit_result.get('manifest_path'):
        overview.add_row("Manifest", submit_result['manifest_path'])
    if submit_result and submit_result.get('source_snapshot'):
        source_snapshot = submit_result['source_snapshot']
        overview.add_row("Source snapshot", source_snapshot.get('path', ''))
        if source_snapshot.get('archive'):
            overview.add_row("Source archive", source_snapshot['archive'].get('path', ''))

    console.print()
    console.print(Panel(overview, title="SKNano Submission Summary", border_style="green"))

    sample_table = Table(box=box.SIMPLE_HEAVY, show_lines=False)
    sample_table.add_column("Era", style="cyan", no_wrap=True)
    sample_table.add_column("Sample", style="bold")
    sample_table.add_column("Jobs", justify="right", style="magenta")
    sample_table.add_column("Output", overflow="fold")
    sample_table.add_column("Workdir", overflow="fold", style="dim")

    max_rows = 30
    for item in dag_list[:max_rows]:
        sample_table.add_row(
            item['era'],
            item['sample'],
            str(item['totalNumberofJobs']),
            getFinalOutputPath(item['era'], item['sample'], argparser, userflags),
            item['working_dir'],
        )
    if len(dag_list) > max_rows:
        sample_table.add_row(
            "...",
            f"{len(dag_list) - max_rows} more samples",
            "",
            "",
            "",
        )
    console.print(sample_table)
        
def getFinalDag(hadd_layer_dicts,skim_postproc_layers,master_dir,argparser):
    batchname = argparser.BatchName
    userflags = getUserFlagsList(argparser.Userflags)
    if batchname == "":
        batchname = argparser.Analyzer
        for flag in userflags:
            batchname += f"_{flag}"
                
    dag_dir = os.path.join(master_dir,"dags")
    os.makedirs(dag_dir)

    job_dict = final_job.copy()
    job_dict['executable'] = os.path.join(dag_dir,"final.sh")
    job_dict['output'] = os.path.join(dag_dir,"final.out")
    job_dict['error'] = os.path.join(dag_dir,"final.err")

    final_content = os.path.join(SKNANO_HOME, "templates", "final.sh")  
    with open(final_content, 'r') as f:
        final_content = f.read()
    final_content = final_content.replace("[DAGDIR]", dag_dir)
    final_content = final_content.replace("[SKNANO_PYTHON]", os.environ['SKNANO_PYTHON'])
    final_content = final_content.replace("[MASTERDIR]", master_dir)
    with open(os.path.join(dag_dir,"final.sh"),'w') as f:
        f.write(final_content)
    
    dag = dags.DAG()
    finaldag = dags.DAG()
    hadd_layers = []
    postproc_layers = []
    if SKIMMING_MODE:
        for layer_dict in skim_postproc_layers:
            if layer_dict is None:
                continue
            analyzer_dict, _, postproc_dict = layer_dict
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
            analyzer_dict, partial_dict, hadd_dict = layer_dict
            if analyzer_dict is None or hadd_dict is None:
                continue
            analyzer_layer = dag.layer(
                name = analyzer_dict['name'],
                submit_description = analyzer_dict['submit_description'],
                vars = analyzer_dict['vars']
            )
            merge_parent = analyzer_layer
            if partial_dict is not None:
                merge_parent = analyzer_layer.child_layer(
                    name = partial_dict['name'],
                    submit_description = partial_dict['submit_description'],
                    vars = partial_dict['vars'],
                    edge = RaggedGrouper(partial_dict['group_sizes'])
                )
            hadd_layer = merge_parent.child_layer(
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

    cluster_id = None
    if not argparser.no_exec:
        submit_cwd = os.getcwd()
        try:
            os.chdir(dag_dir)
            secret_names = {"TOKEN_TELEGRAMBOT", "USER_CHATID"}
            included_environment = ','.join(
                name for name in os.environ.keys() if name not in secret_names
            )
            finalDag_submit = htcondor.Submit.from_dag(
                str(finalDag_file),
                {"force": 1, "include_env": included_environment, "batch-name": batchname},
            )
            cluster_id = htcondor.Schedd().submit(finalDag_submit).cluster()
            print(f"DAGMan job cluster is {cluster_id}")
        finally:
            os.chdir(submit_cwd)

    return {
        'cluster_id': cluster_id,
        'dag_dir': dag_dir,
        'dag_file': str(finalDag_file),
    }

    
if __name__ == '__main__':
    parser = setParser()
    args = parser.parse_args()
    validateInstalledAnalyzer(args.Analyzer, args.python)
    eras = getEraList(args.Era, args.Run)
    SKIMMING_MODE = args.skimming_mode
    if args.Analyzer.startswith("Skim_") and not SKIMMING_MODE:
        print('\033[93m'+'''It seems like you want to skim the samples. If so, you need to enable skimming mode by passing the 
--skimming_mode arguments to write the output file to gv0 and generate the *.json file of the skimmed sample.\n'''+'\033[0m')
        #ask Y/N to continue, print in yellow color
        while True:
            answer = input('\033[93m'+"Do you want to enable the skimming mode? (Y/N): "+'\033[0m')
            if answer == 'Y' or answer == 'y':
                SKIMMING_MODE = True
                break
            elif answer == 'N' or answer == 'n':
                SKIMMING_MODE = False
                break
            else:
                print('\033[93m'+"Please enter Y or N"+'\033[0m')

    
    userflags = getUserFlagsList(args.Userflags)
    timestamp, string_JobStartTime = getTimeStamp()
    _, abs_MasterDirectoryName= getMasterDirectoryName(timestamp, args.Analyzer, userflags)
    source_snapshot = makeSourceSnapshot(abs_MasterDirectoryName)
    InputSamplelist = getInputSampleList(args.InputSample)
    period_filter = parsePeriodFilter(args.Period)
    exclude_regexes = getExcludeRegexList(args.ExcludeSample)
    
    dag_list = []
    hadd_layers = []
    postproc_layers = []

    for era in eras:
        print(f"Working on {era}")
        InputSamplelist_era = makeSampleList(InputSamplelist, era, period_filter)
        if exclude_regexes:
            InputSamplelist_era = [
                sample for sample in InputSamplelist_era
                if not any(regex.search(sample) for regex in exclude_regexes)
            ]
        for isample, sample in enumerate(InputSamplelist_era):
            working_dir, totalNumberofJobs, metadata_snapshot_files = jobProducer(era, sample, args, abs_MasterDirectoryName, userflags, isample, len(InputSamplelist_era))
            if totalNumberofJobs == None:
                continue
            analyzer_sub_dict = makeMainAnalyzerJobs(working_dir,abs_MasterDirectoryName,totalNumberofJobs,args)
            partial_sub_dict = None
            merge_group_sizes = None
            if SKIMMING_MODE:
                postproc_sub_dict = makeSkimPostProcsJobs(working_dir,sample,args,era)
            elif args.no_hadd:
                hadd_sub_dict = makeMoveJobs(working_dir,args,sample)
            else:
                hadd_sub_dict, partial_sub_dict, merge_group_sizes = makeHaddJobs(working_dir,args,sample,totalNumberofJobs)
            
            if SKIMMING_MODE:
                dag_list.append({'era':era,'sample':sample,'analyzer_sub_dict':analyzer_sub_dict,'hadd_sub_dict':postproc_sub_dict,'totalNumberofJobs':totalNumberofJobs,'working_dir':working_dir,'batchname':f"{args.Analyzer}_{era}_{sample}",'metadata_snapshot_files':metadata_snapshot_files})
            else:
                dag_list.append({'era':era,'sample':sample,'analyzer_sub_dict':analyzer_sub_dict,'hadd_sub_dict':hadd_sub_dict,'partial_sub_dict':partial_sub_dict,'merge_group_sizes':merge_group_sizes,'postproc_label':'Move' if args.no_hadd else 'Hadd','totalNumberofJobs':totalNumberofJobs,'working_dir':working_dir,'batchname':f"{args.Analyzer}_{era}_{sample}",'metadata_snapshot_files':metadata_snapshot_files})
            if dag_list is not None:
                if SKIMMING_MODE:
                    postproc_layers.append(getEachAnalyzerToPostDag(dag_list[-1]))
                else:
                    hadd_layers.append(getEachAnalyzerToPostDag(dag_list[-1]))
            
    if len(dag_list) == 0:
        print('\033[91m'+"ERROR: no samples matched the requested selection"+'\033[0m')
        sys.exit(1)

    writeRunManifest(abs_MasterDirectoryName, args, userflags, dag_list, source_snapshot)
    submit_result = getFinalDag(hadd_layers, postproc_layers, abs_MasterDirectoryName, args)
    submit_result['source_snapshot'] = source_snapshot
    submit_result['manifest_path'] = writeRunManifest(abs_MasterDirectoryName, args, userflags, dag_list, source_snapshot, submit_result)
    renderSubmissionSummary(dag_list, abs_MasterDirectoryName, args, userflags, submit_result)
    if not args.no_exec:
        with open(submit_result['manifest_path'], encoding='utf-8') as manifest_file:
            manifest = json.load(manifest_file)
        send_telegram_message(submission_message(manifest))
