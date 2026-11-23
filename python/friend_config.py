"""Friend productions declared by analysis modules (docs/FriendInput.md).

A module that needs custom columns next to the central NanoAOD ships one
``<module>/friends.yml``::

    hadron_av6:                       # friend production name
      analyzers: [HadronAnalyzer]     # which analyzers attach it
      eras: [2024]                    # optional, every era when omitted
      overrides: [GenPart]            # central collections replaced by the friend
      exclude_prefixes: [D0ToKPiLoose]
      filtered: false                 # does the producer apply an event filter?
      filter_note: "HLT prefilter"    # what that filter was
      sources:                        # builder only: aux file globs, {PD}/{era}
        - /gv0/DATA/SKNano/CustomNanoAODv15/hadron_av6_rntuple640/{era}/{PD}/**/*.root
      samples: [TTLJ_powheg, Muon0]   # optional allowlist of registry names
      on_missing: error               # or run_without, when a sample has no friend
      root: null                      # optional, else $SKNANO_FRIEND_ROOT

The submission and the builder read the same file, so the production is
described once.  Friend files mirror the central file's path below
``<root>/<production>/`` and are paired by that path, never by position.

Normalisation: an unfiltered production's lost events are skipped and their
genWeight leaves the central sumW; a filtered production is assumed lossless,
its rejected events are skipped, and the sample is normalised to the sumW its
Runs tables record before the filter.
"""

import glob
import json
import os
from dataclasses import dataclass, field
from typing import Any, Mapping, Optional

import yaml

FRIEND_ROOT_ENV = "SKNANO_FRIEND_ROOT"
INPUT_ROOT_ENV = "SKNANO_INPUT_ROOT"
FRIENDS_FILENAME = "friends.yml"
SUMMARY_FILENAME = "friend_summary.json"
PRODUCTION_FILENAME = "friend_production.json"

_KNOWN_KEYS = {"analyzers", "eras", "overrides", "exclude_prefixes", "filtered",
               "filter_note", "sources", "samples", "on_missing", "root"}


def friend_root() -> str:
    root = os.environ.get(FRIEND_ROOT_ENV)
    if not root:
        raise ValueError(
            f"{FRIEND_ROOT_ENV} is not set; source setup.sh, or set it in "
            "config/config.$USER")
    return root.rstrip("/")


def _as_list(value: Any, key: str, production: str) -> list[str]:
    if value is None:
        return []
    if isinstance(value, str):
        return [value]
    if not isinstance(value, list) or not all(isinstance(v, str) for v in value):
        raise ValueError(f"friend '{production}': '{key}' must be a list of strings")
    return list(value)


def _as_bool(value: Any, key: str, production: str) -> bool:
    if not isinstance(value, bool):
        raise ValueError(f"friend '{production}': '{key}' must be true or false")
    return value


@dataclass
class FriendProduction:
    name: str
    module_dir: str
    analyzers: list[str]
    eras: list[str] = field(default_factory=list)
    overrides: list[str] = field(default_factory=list)
    exclude_prefixes: list[str] = field(default_factory=list)
    filter_note: str = ""
    sources: list[str] = field(default_factory=list)
    samples: list[str] = field(default_factory=list)
    on_missing: str = "error"
    filtered: bool = False
    root_override: Optional[str] = None

    @property
    def root(self) -> str:
        return (self.root_override or friend_root()).rstrip("/")

    @property
    def production_dir(self) -> str:
        return os.path.join(self.root, self.name)

    def applies(self, analyzer: str, era: str, sample: Optional[str] = None) -> bool:
        if analyzer not in self.analyzers:
            return False
        if self.eras and era not in self.eras:
            return False
        if sample is not None and self.samples and sample not in self.samples:
            return False
        return True

    def friend_path(self, central_path: str) -> str:
        """The friend file for one central file: the same path below the
        production directory.  Files outside the input root keep their
        absolute path minus the leading slash, so the pairing stays unique."""
        input_root = os.environ.get(INPUT_ROOT_ENV, "").rstrip("/")
        central_path = os.path.abspath(central_path)
        if input_root and central_path.startswith(input_root + "/"):
            relative = central_path[len(input_root) + 1:]
        else:
            relative = central_path.lstrip("/")
        return os.path.join(self.production_dir, relative)

    def sample_dir(self, era: str, sample: str) -> str:
        return os.path.join(self.production_dir, era, sample)

    def summary_path(self, era: str, sample: str) -> str:
        return os.path.join(self.sample_dir(era, sample), SUMMARY_FILENAME)

    def production_json_path(self) -> str:
        return os.path.join(self.production_dir, PRODUCTION_FILENAME)

    def source_globs(self, era: str, pd: str) -> list[str]:
        return [pattern.format(era=era, PD=pd) for pattern in self.sources]

    def source_files(self, era: str, pd: str) -> list[str]:
        files: list[str] = []
        for pattern in self.source_globs(era, pd):
            files.extend(p for p in glob.glob(pattern, recursive=True)
                         if os.path.isfile(p) and p.endswith(".root")
                         and not os.path.basename(p).startswith("."))
        return sorted(set(files))


def load_module_friends(module_dir: str) -> list[FriendProduction]:
    path = os.path.join(module_dir, FRIENDS_FILENAME)
    if not os.path.isfile(path):
        return []
    with open(path, encoding="utf-8") as handle:
        payload = yaml.safe_load(handle) or {}
    if not isinstance(payload, Mapping):
        raise ValueError(f"{path}: top level must map production names to settings")
    productions = []
    for name, spec in payload.items():
        spec = spec or {}
        unknown = set(spec) - _KNOWN_KEYS
        if unknown:
            raise ValueError(f"{path}: friend '{name}' has unknown keys {sorted(unknown)}")
        analyzers = _as_list(spec.get("analyzers"), "analyzers", name)
        if not analyzers:
            raise ValueError(f"{path}: friend '{name}' needs 'analyzers'")
        on_missing = spec.get("on_missing", "error")
        if on_missing not in ("error", "run_without"):
            raise ValueError(f"{path}: friend '{name}': on_missing must be error or run_without")
        productions.append(FriendProduction(
            name=str(name), module_dir=module_dir, analyzers=analyzers,
            eras=[str(e) for e in _as_list(spec.get("eras"), "eras", name)],
            overrides=_as_list(spec.get("overrides"), "overrides", name),
            exclude_prefixes=_as_list(spec.get("exclude_prefixes"), "exclude_prefixes", name),
            filter_note=str(spec.get("filter_note") or ""),
            sources=_as_list(spec.get("sources"), "sources", name),
            samples=_as_list(spec.get("samples"), "samples", name),
            on_missing=on_missing,
            filtered=_as_bool(spec.get("filtered", False), "filtered", name),
            root_override=spec.get("root") or None))
    return productions


def load_friends(module_dirs: list[str]) -> list[FriendProduction]:
    """friends.yml of every module: at the module root or, like the sample
    jsons, one directory below it (``Hadron_Analyzers/Hadron/friends.yml``)."""
    productions: list[FriendProduction] = []
    for module_dir in module_dirs:
        candidates = [module_dir] + sorted(
            os.path.dirname(p) for p in glob.glob(os.path.join(module_dir, "*", FRIENDS_FILENAME)))
        for candidate in candidates:
            for production in load_module_friends(candidate):
                if any(p.name == production.name for p in productions):
                    raise ValueError(
                        f"friend production '{production.name}' is declared twice")
                productions.append(production)
    return productions


def friends_for(module_dirs: list[str], analyzer: str, era: str,
                sample: Optional[str] = None) -> list[FriendProduction]:
    return [p for p in load_friends(module_dirs) if p.applies(analyzer, era, sample)]


def load_summary(production: FriendProduction, era: str, sample: str) -> dict:
    path = production.summary_path(era, sample)
    if not os.path.isfile(path):
        raise FileNotFoundError(
            f"friend '{production.name}' has no {SUMMARY_FILENAME} for {era}/{sample} "
            f"({path}); run: sknano_friend.py validate -f {production.name} -e {era} -i {sample}")
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


@dataclass
class PairedFriends:
    """What the submission attaches to one sample."""
    paths: list[list[str]]          # per central file, the friend files
    attached: list[dict]            # per production: provenance and normalisation
    missing_sumw: float = 0.        # unfiltered: subtract from the central sumW
    missing_sumsign: float = 0.
    norm: Optional[dict] = None     # filtered: {"sumw", "sumsign", "nmc"} replaces it

    def normalisation(self, sumw: float, sumsign: float) -> tuple[float, float]:
        if self.norm:
            return self.norm["sumw"], self.norm["sumsign"]
        return sumw - self.missing_sumw, sumsign - self.missing_sumsign


def pair_friends(productions: list[FriendProduction], central_paths: list[str],
                 era: str, sample: str, is_mc: bool) -> PairedFriends:
    paired = PairedFriends(paths=[[] for _ in central_paths], attached=[])
    for production in productions:
        friend_paths = [production.friend_path(p) for p in central_paths]
        missing = [p for p in friend_paths if not os.path.isfile(p)]
        if missing:
            message = (f"friend '{production.name}' is missing {len(missing)}/"
                       f"{len(friend_paths)} files for {era}/{sample}, e.g. {missing[0]}; "
                       f"build it with: sknano_friend.py build -f {production.name} "
                       f"-e {era} -i {sample}")
            if production.on_missing == "error":
                raise RuntimeError(message)
            print(f"\033[93mWarning: {message}; running without it\033[0m")
            continue
        summary = load_summary(production, era, sample)
        known = set(summary.get("files", {}))
        unknown = [p for p in friend_paths if p not in known]
        if unknown:
            raise RuntimeError(
                f"friend '{production.name}': {len(unknown)} files of {era}/{sample} are not in "
                f"its {SUMMARY_FILENAME} (e.g. {unknown[0]}); re-run sknano_friend.py validate")
        for i, path in enumerate(friend_paths):
            paired.paths[i].append(path)
        entry = {
            "production": production.name,
            "root": production.root,
            "summary": production.summary_path(era, sample),
            "overrides": production.overrides,
            "filtered": production.filtered,
            "n_status": summary.get("n_status", [0, 0, 0]),
            "missing_lumis": len(summary.get("missing_lumis", [])),
        }
        if is_mc and production.filtered:
            if not summary.get("norm"):
                raise RuntimeError(
                    f"friend '{production.name}' is filtered but its {SUMMARY_FILENAME} for "
                    f"{era}/{sample} has no 'norm'; re-run sknano_friend.py validate")
            if paired.norm:
                raise RuntimeError(
                    f"two filtered friends normalise {era}/{sample}; attach at most one")
            paired.norm = dict(summary["norm"])
            entry["norm"] = paired.norm
        elif is_mc:
            entry["missing_sumw"] = float(summary.get("missing_sumw", 0.))
            entry["missing_sumsign"] = float(summary.get("missing_sumsign", 0.))
            paired.missing_sumw += entry["missing_sumw"]
            paired.missing_sumsign += entry["missing_sumsign"]
        paired.attached.append(entry)
    return paired
