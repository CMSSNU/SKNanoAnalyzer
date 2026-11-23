#!/usr/bin/env python3
"""Build and validate friend RNTuples for a production declared in a module's
friends.yml (docs/FriendInput.md).

    sknano_friend.py build    -f hadron_av6 [-e 2024] [-i TTLJ_powheg,Muon0] [-p C] [--local]
    sknano_friend.py validate -f hadron_av6 [-e 2024] [-i ...] [-p ...]
    sknano_friend.py status   -f hadron_av6 [-e 2024] [-i ...] [-p ...]

-e/-i/-p mean what they mean for SKNano.py; without them every era and
sample the production declares in friends.yml is covered.

`build` resolves the sample's central files exactly like SKNano.py does, finds
the private-production files covering each one through a cached lumi index,
and runs `sknano_friend build` per central file (one condor job each, or
locally with --local).  `validate` checks every central file has its friend and
writes the per-sample friend_summary.json the submission reads for its
normalisation correction.
"""

from __future__ import annotations

import argparse
import datetime
import glob
import json
import math
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

SKNANO_HOME = os.environ.get("SKNANO_HOME") or str(Path(__file__).resolve().parents[1])
sys.path.insert(0, SKNANO_HOME)

from python import friend_config as fc  # noqa: E402
from python.sample_paths import resolve_sample_paths  # noqa: E402

SKNANO_DATA = os.environ.get("SKNANO_DATA", "")
BINARY = os.path.join(os.environ.get("SKNANO_INSTALLDIR", ""), "bin", "sknano_friend")
INDEX_DIRNAME = "_index"
JOBS_DIRNAME = "_jobs"


def module_dirs() -> list[str]:
    raw = os.environ.get("SKNANO_ANALYSIS_MODULE_DIRS", "")
    dirs = []
    for entry in raw.replace(";", os.pathsep).split(os.pathsep):
        if entry and os.path.isdir(entry):
            dirs.append(os.path.realpath(os.path.expanduser(entry)))
    return dirs


def sample_registry(era: str) -> dict:
    """CommonSampleInfo plus the module sample jsons, as SKNano.py merges them."""
    path = os.path.join(SKNANO_DATA, era, "Sample", "CommonSampleInfo.json")
    with open(path, encoding="utf-8") as handle:
        registry = json.load(handle)
    for module_dir in module_dirs():
        for pattern in (os.path.join(module_dir, "data", "Sample", era, "*.json"),
                        os.path.join(module_dir, "*", "data", "Sample", era, "*.json")):
            for sample_path in sorted(glob.glob(pattern)):
                with open(sample_path, encoding="utf-8") as handle:
                    payload = json.load(handle)
                registry[payload.get("name", Path(sample_path).stem)] = payload
    return registry


def production_by_name(name: str) -> fc.FriendProduction:
    for production in fc.load_friends(module_dirs()):
        if production.name == name:
            return production
    raise SystemExit(f"no friends.yml in {module_dirs()} declares production '{name}'")


def split_sample(sample: str, registry: dict) -> tuple[str, str | None]:
    """'Muon0_C' -> ('Muon0', 'C') for data; MC samples carry no period."""
    if sample in registry and registry[sample].get("isMC"):
        return sample, None
    match = re.match(r"^(.*)_([A-Z](?:_v\d+)?)$", sample)
    if match and match.group(1) in registry:
        return match.group(1), match.group(2)
    if sample in registry:
        return sample, None
    raise SystemExit(f"sample '{sample}' is not in the sample registry")


def lumi_table(path: str) -> set[tuple[int, int]]:
    # uproot rather than PyROOT: an RNTupleView outliving its reader at
    # function exit segfaults, and this runs over thousands of files.
    import uproot  # noqa: PLC0415  (slow import, only when needed)
    table = uproot.open(path)["LuminosityBlocks"].arrays(
        ["run", "luminosityBlock"], library="np")
    return set(zip(table["run"].tolist(), table["luminosityBlock"].tolist()))


INDEX_VERSION = 2


def aux_index(production: fc.FriendProduction, era: str, pd: str) -> tuple[list[str], dict]:
    """lumi -> every aux file holding part of it, cached per (production, era,
    PD) until the pool changes.  A lumi split over several aux files (common
    for data, where lumis cross MiniAOD file boundaries) sends all of them to
    the builder, which joins on the event key and rejects duplicate events."""
    files = production.source_files(era, pd)
    if not files:
        raise SystemExit(f"no aux files for {era}/{pd} under {production.source_globs(era, pd)}")
    path = os.path.join(production.production_dir, INDEX_DIRNAME, era, f"{pd}.json")
    if os.path.isfile(path):
        with open(path, encoding="utf-8") as handle:
            cached = json.load(handle)
        if cached.get("version") == INDEX_VERSION and cached.get("files") == files:
            return files, {tuple(map(int, k.split(":"))): v for k, v in cached["lumis"].items()}
    print(f"indexing {len(files)} aux files for {era}/{pd} ...", flush=True)
    lumis: dict[tuple[int, int], list[int]] = {}
    for index, file in enumerate(files):
        for key in lumi_table(file):
            lumis.setdefault(key, []).append(index)
    split = sum(1 for owners in lumis.values() if len(owners) > 1)
    if split:
        print(f"  {split}/{len(lumis)} lumis are spread over more than one aux file")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path + ".tmp", "w", encoding="utf-8") as handle:
        json.dump({"version": INDEX_VERSION, "files": files,
                   "lumis": {f"{r}:{l}": v for (r, l), v in lumis.items()}}, handle)
    os.replace(path + ".tmp", path)
    return files, lumis


def build_command(production, central, aux, out, is_mc, args) -> list[str]:
    command = [BINARY, "build", "--central", central, "--out", out,
               "--production", production.name, "--report", out + ".build.json",
               "--chunk", str(args.chunk)]
    if args.local:  # condor jobs stage under <production>/_scratch, see run_condor
        command += ["--scratch", args.scratch]
    for file in aux:
        command += ["--aux", file]
    for collection in production.overrides:
        command += ["--override", collection]
    for collection in production.exclude_prefixes:
        command += ["--exclude-prefix", collection]
    if is_mc:
        command.append("--mc")
    if production.filtered:
        command.append("--filtered")
    if production.filter_note:
        command += ["--filter-note", production.filter_note]
    return command


def plan_sample(production, era, sample, registry, args):
    name, period = split_sample(sample, registry)
    spec = dict(registry[name], name=name)
    is_mc = bool(spec.get("isMC"))
    central_files = resolve_sample_paths(spec, era, period)
    pd = spec.get("PD") or name
    aux_files, index = aux_index(production, era, pd)
    jobs, done = [], []
    for central in central_files:
        out = production.friend_path(central)
        if os.path.isfile(out) and not args.force:
            done.append(out)
            continue
        aux = sorted({aux_files[i] for key in lumi_table(central) for i in index.get(key, ())})
        if not aux:
            if production.filtered:
                # A filtered production is assumed lossless and normalised to
                # its own Runs sums; a whole central file it never saw breaks that.
                raise SystemExit(f"no aux file covers any lumi of {central}, but "
                                 f"'{production.name}' is filtered and assumed lossless")
            print(f"\033[93mWarning: no aux file covers any lumi of {central}; "
                  "every event will carry status 2\033[0m")
            aux = [aux_files[0]]  # still build: the friend then records the missing lumis
        jobs.append(build_command(production, central, aux, out, is_mc, args))
    return central_files, jobs, done, is_mc


def run_condor(production, era, sample, jobs, args):
    stamp = datetime.datetime.now().strftime("%Y_%m_%d_%H%M%S")
    job_dir = os.path.join(production.production_dir, JOBS_DIRNAME, era, sample, stamp)
    os.makedirs(os.path.join(job_dir, "logs"))
    # condor's "new" argument syntax: the whole list is one double-quoted
    # string (in the .sub), arguments with spaces are single-quoted inside it.
    def condor_arg(part: str) -> str:
        if not re.search(r"[\s'\"]", part):
            return part
        return "'" + part.replace("'", "''").replace('"', '""') + "'"
    with open(os.path.join(job_dir, "jobs.txt"), "w", encoding="utf-8") as handle:
        for command in jobs:
            handle.write(" ".join(condor_arg(part) for part in command[1:]) + "\n")
    runner = os.path.join(job_dir, "run.sh")
    # The stage files go to the shared friend root, not the worker: on the
    # SNU workers /tmp and the condor scratch are tmpfs, so a stage written
    # there is charged to the job's memory (a 243k-entry file was held at
    # 7 GB).  The builder removes its stage directory when it finishes.
    scratch = os.path.join(production.production_dir, "_scratch")
    os.makedirs(scratch, exist_ok=True)
    with open(runner, "w", encoding="utf-8") as handle:
        handle.write(f"""#!/bin/bash
source "{SKNANO_HOME}/setup.sh" > /dev/null 2>&1
exec "{BINARY}" "$@" --scratch "{scratch}"
""")
    os.chmod(runner, 0o755)
    submit = os.path.join(job_dir, "submit.sub")
    with open(submit, "w", encoding="utf-8") as handle:
        handle.write(f"""executable = {runner}
arguments = "$(args)"
getenv = True
request_memory = {args.memory} MB
request_cpus = 1
max_materialize = {args.nmax}
log = {job_dir}/logs/job.$(Process).log
output = {job_dir}/logs/job.$(Process).out
error = {job_dir}/logs/job.$(Process).err
batch_name = sknano_friend_{production.name}_{sample}
queue args from {job_dir}/jobs.txt
""")
    condor_submit = shutil.which("condor_submit")
    if args.no_exec:
        print(f"  job area written, not submitted (--no_exec): condor_submit {submit}")
        return
    if not condor_submit:
        print(f"condor_submit is not on PATH; submit on the schedd host with:\n  condor_submit {submit}")
        return
    subprocess.run([condor_submit, submit], check=True)
    print(f"submitted {len(jobs)} jobs, logs in {job_dir}/logs")


def cmd_build(args):
    production = production_by_name(args.friend)
    for era, registry, sample in targets(args, production):
        central_files, jobs, done, _ = plan_sample(production, era, sample, registry, args)
        print(f"{era}/{sample}: {len(central_files)} central files, "
              f"{len(done)} friends present, {len(jobs)} to build")
        if not jobs:
            continue
        if not args.local:
            run_condor(production, era, sample, jobs, args)
            continue
        if args.no_exec:
            print("  " + " ".join(jobs[0]))
            continue
        for i, command in enumerate(jobs, 1):
            print(f"  [{i}/{len(jobs)}] {command[3]}", flush=True)
            subprocess.run(command, check=True, stdout=subprocess.DEVNULL)


def brilcalc_mask(missing: list[tuple[int, int]]) -> dict:
    """Run -> [[first, last], ...] as brilcalc -i / the golden json expect."""
    mask: dict[str, list[list[int]]] = {}
    for run, lumi in sorted(missing):
        ranges = mask.setdefault(str(run), [])
        if ranges and ranges[-1][1] == lumi - 1:
            ranges[-1][1] = lumi
        else:
            ranges.append([lumi, lumi])
    return mask


def cmd_validate(args):
    production = production_by_name(args.friend)
    failed = False
    for era, registry, sample in targets(args, production):
        name, period = split_sample(sample, registry)
        spec = dict(registry[name], name=name)
        is_mc = bool(spec.get("isMC"))
        central_files = resolve_sample_paths(spec, era, period)
        files, missing_files, missing = {}, [], set()
        aux_runs: dict[str, dict] = {}  # each aux file once, however many friends read it
        totals = {"n_status": [0, 0, 0], "missing_sumw": 0., "missing_sumsign": 0.,
                  "missing_count": 0, "central_entries": 0, "aux_only": 0}
        for central in central_files:
            out = production.friend_path(central)
            report_path = out + ".build.json"
            if not os.path.isfile(out) or not os.path.isfile(report_path):
                missing_files.append(out)
                continue
            with open(report_path, encoding="utf-8") as handle:
                report = json.load(handle)
            if bool(report.get("filtered", False)) != production.filtered:
                failed = True
                print(f"\033[91m{out} was built with filtered={report.get('filtered', False)}, "
                      f"friends.yml says {production.filtered}; rebuild with --force\033[0m")
            aux_runs.update(report.get("aux_runs", {}))
            files[out] = {"central": central, "n_status": report["n_status"],
                          "missing_lumis": report["missing_lumis"],
                          "missing_sumw": report["missing_sumw"],
                          "central_entries": report["central_entries"]}
            for i in range(3):
                totals["n_status"][i] += report["n_status"][i]
            for key in ("missing_sumw", "missing_sumsign", "missing_count", "central_entries", "aux_only"):
                totals[key] += report.get(key, 0)
            missing.update(tuple(x) for x in report["missing_lumis"])
        status = f"{era}/{sample}: {len(files)}/{len(central_files)} friends"
        if missing_files:
            failed = True
            print(f"\033[91m{status}, {len(missing_files)} missing, e.g. {missing_files[0]}\033[0m")
            continue
        summary = dict(totals, era=era, sample=sample, production=production.name,
                       is_mc=is_mc, filtered=production.filtered, files=files,
                       missing_lumis=[list(x) for x in sorted(missing)],
                       validated_at=datetime.datetime.now().isoformat(timespec="seconds"))
        if is_mc and production.filtered:
            # Lossless by assumption, so the sample is what the aux files read:
            # their pre-filter Runs sums.  sumsign is not stored; with weights of
            # constant magnitude +-w0, sumw=(N+ - N-)w0 and sumw2=N w0^2 give
            # sumsign = sumw*sqrt(N/sumw2) (Hadron docs/CustomBPHInput.md).
            nmc = sum(r["count"] for r in aux_runs.values())
            sumw = sum(r["sumw"] for r in aux_runs.values())
            sumw2 = sum(r["sumw2"] for r in aux_runs.values())
            summary["norm"] = {"nmc": nmc, "sumw": sumw,
                               "sumsign": sumw * math.sqrt(nmc / sumw2) if sumw2 > 0 else 0.,
                               "source": f"Runs of {len(aux_runs)} aux files"}
        sample_dir = production.sample_dir(era, sample)
        os.makedirs(sample_dir, exist_ok=True)
        if missing and not is_mc:
            mask_path = os.path.join(sample_dir, "missing_lumis.json")
            with open(mask_path, "w", encoding="utf-8") as handle:
                json.dump(brilcalc_mask(sorted(missing)), handle, indent=1)
            summary["missing_lumis_json"] = mask_path
            summary["brilcalc"] = (f"brilcalc lumi -u /pb --normtag <normtag> -i {mask_path}  "
                                   "# recorded lumi to subtract from the trigger lumi")
        with open(production.summary_path(era, sample), "w", encoding="utf-8") as handle:
            json.dump(summary, handle, indent=2)
        note = f"{status}, status counts {totals['n_status']}"
        if summary.get("norm"):
            note += f", normalised to {summary['norm']['sumw']} ({summary['norm']['source']})"
        elif missing:
            note += f", events lost in {len(missing)} lumis"
            note += (f", missing sumW {totals['missing_sumw']}" if is_mc else
                     f", mask {summary['missing_lumis_json']}")
        print(note)
        update_production_json(production, era, sample)
    if failed:
        raise SystemExit(1)


def update_production_json(production, era, sample):
    path = production.production_json_path()
    payload = {}
    if os.path.isfile(path):
        with open(path, encoding="utf-8") as handle:
            payload = json.load(handle)
    payload.update({"production": production.name, "format_version": 1,
                    "module_dir": production.module_dir,
                    "overrides": production.overrides,
                    "exclude_prefixes": production.exclude_prefixes,
                    "filter_note": production.filter_note, "sources": production.sources,
                    "updated_at": datetime.datetime.now().isoformat(timespec="seconds")})
    payload.setdefault("samples", {}).setdefault(era, {})[sample] = production.summary_path(era, sample)
    with open(path, "w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2, sort_keys=True)


def cmd_status(args):
    production = production_by_name(args.friend)
    for era, registry, sample in targets(args, production):
        name, period = split_sample(sample, registry)
        spec = dict(registry[name], name=name)
        central_files = resolve_sample_paths(spec, era, period)
        present = sum(os.path.isfile(production.friend_path(c)) for c in central_files)
        summary = production.summary_path(era, sample)
        print(f"{era}/{sample}: {present}/{len(central_files)} friends, "
              f"summary {'present' if os.path.isfile(summary) else 'missing'}")


def targets(args, production):
    """(era, registry, sample) to work on, selected the way SKNano.py selects
    them: -e/-i/-p with comma lists or a .txt, data samples expanded into
    their periods.  Without -i, every sample the production lists."""
    eras = production.eras if args.era == "All" else [e for e in args.era.split(",") if e]
    if not eras:
        raise SystemExit(f"'{production.name}' declares no eras; give -e")
    if args.samples:
        if args.samples.endswith(".txt"):
            with open(args.samples, encoding="utf-8") as handle:
                names = [line.strip() for line in handle]
        else:
            names = args.samples.split(",")
        names = [n.strip() for n in names if n.strip() and not n.strip().startswith("#")]
    else:
        names = list(production.samples)
    if not names:
        raise SystemExit(f"'{production.name}' lists no samples; give -i")
    tokens = [t.strip() for t in args.period.split(",") if t.strip()]
    period_filter = None if not tokens or tokens == ["All"] else set(tokens)
    for era in eras:
        registry = sample_registry(era)
        for name in names:
            info = registry.get(name)
            if info is None or info.get("isMC"):
                yield era, registry, name  # MC, or data given with its period
                continue
            periods = [p for p in info.get("periods", [])
                       if period_filter is None or p in period_filter]
            if not periods:
                print(f"\033[93mWarning: no matching periods for {name} in era {era}\033[0m")
            for period in periods:
                yield era, registry, f"{name}_{period}"


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["build", "validate", "status"])
    parser.add_argument("-f", "--friend", required=True,
                        help="friend production, as named in a module's friends.yml")
    # Same meaning as in SKNano.py.
    parser.add_argument("-e", dest="era", default="All",
                        help="eras, comma separated; All = the eras the production declares")
    parser.add_argument("-i", dest="samples", default="",
                        help="samples, comma separated or a .txt; default: the production's samples")
    parser.add_argument("-p", dest="period", default="All",
                        help="data periods, e.g. C or I,I_v2; default All")
    parser.add_argument("--nmax", type=int, default=100,
                        help="maximum running jobs (condor max_materialize); bounds the "
                             "concurrent readers on the input filesystem")
    parser.add_argument("--memory", type=int, default=6000, help="condor request_memory in MB")
    parser.add_argument("--no_exec", action="store_true",
                        help="only produce the job area (with --local: print the command), run nothing")
    # Builder only.
    parser.add_argument("--local", action="store_true",
                        help="build on this machine instead of one condor job per central file")
    parser.add_argument("--scratch", default=os.environ.get("TMPDIR", "/tmp"),
                        help="stage directory for --local (condor jobs stage under <production>/_scratch)")
    parser.add_argument("--chunk", type=int, default=5000, help="aux entries buffered per stage file")
    parser.add_argument("--force", action="store_true", help="rebuild friends that already exist")
    args = parser.parse_args()
    if args.command == "build" and not os.path.isfile(BINARY):
        raise SystemExit(f"{BINARY} not found; build the framework first")
    {"build": cmd_build, "validate": cmd_validate, "status": cmd_status}[args.command](args)


if __name__ == "__main__":
    main()
