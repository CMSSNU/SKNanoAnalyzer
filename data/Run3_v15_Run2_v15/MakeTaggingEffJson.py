#!/usr/bin/env python3
"""Build BTV/btaggingEff.json -- the MC b-tagging efficiency map method 1a needs.

The fixed-working-point b-tag event weight is

    w = prod_{tagged}  SF_i  *  prod_{untagged} (1 - SF_j eps_j) / (1 - eps_j)

so it needs a per-jet MC efficiency eps(flavour, |eta|, pt) alongside the BTV
scale factor.  BTV publishes the SF but not the efficiency: the efficiency is a
property of *our* jet selection, so every analysis measures its own.

Input is the private NanoAOD RNTuple, before any event-level b-tag requirement.
The old implementation read BPHEvents, which is written only after >=2 loose
b-tags and therefore measured a conditional, biased efficiency.  Jets use the
analysis pt/eta and 2024 TightLeptonVeto ID cuts.  The tiny difference between
NanoAOD pt and the analysis's recomputed central JEC/JER matters only at bin
edges and is preferable to conditioning the denominator on the tag decision.

Jets are counted unweighted in the high-statistics semileptonic ttbar sample,
which also dominates the selected analysis.  The efficiency is parameterised
in flavour/pt/eta; no process-mixture weighting is needed.

    python data/Run3_v15_Run2_v15/MakeTaggingEffJson.py [--stride N] [--era 2024]
"""
from __future__ import annotations

import argparse, glob, gzip, json
from pathlib import Path

import numpy as np

TAGGER = "UParTAK4"          # the only AK4 tagger BTV published for 2024
WPS = ["L", "M", "T"]        # L is what the selection cuts on; M/T for later
FLAVOURS = [5, 4, 0]         # b / c / udsg, BTV hadronFlavour convention

# Coarse enough that every (flavour, eta, pt) cell keeps four-figure statistics,
# fine enough to follow the strong pt turn-on below ~60 GeV.
PT_EDGES = [25., 30., 40., 50., 70., 100., 150., 200., 300., 1000.]
ETA_EDGES = [0., 0.8, 1.6, 2.4]

WP_FILE = ("/cvmfs/cms-griddata.cern.ch/cat/metadata/BTV/"
           "Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15/latest/btagging.json.gz")


def wp_thresholds():
    d = json.load(gzip.open(WP_FILE, "rt"))
    c = next(x for x in d["corrections"] if x["name"] == f"{TAGGER}_wp_values")
    return {k["key"]: k["value"] for k in c["data"]["content"]}


def nano_files(era, sample, stride, max_files):
    sample_json = (Path(__file__).resolve().parents[2] / "Hadron_Analyzers" /
                   "Hadron" / "data" / "Sample" / era / f"{sample}.json")
    info = json.loads(sample_json.read_text())
    files = sorted(glob.glob(info["path_glob"], recursive=True))[::stride]
    return files[:max_files] if max_files else files


def collect(era, sample, stride, max_files, thr):
    """(|eta|, pt) jet counts per flavour, and per (WP, flavour) tagged counts.

    Filled as RDF histograms in a single pass rather than pulled into numpy:
    the jet arrays run to tens of millions and materialising them costs far
    more time and memory than the map is worth.
    """
    import ROOT
    ROOT.EnableImplicitMT()
    files = nano_files(era, sample, stride, max_files)
    if not files:
        raise SystemExit(f"no NanoAOD files for {sample}")
    print(f"{len(files)} NanoAOD shards from {sample}", flush=True)
    # For |eta| <= 2.4 this is exactly the published 2024
    # AK4PUPPI_TightLeptonVeto correction encoded in jetid.json.gz.
    # ponytail: use Nano central pt without lepton cleaning; add a dedicated
    # pre-tag analyzer lane only if sub-percent bin-edge precision is required.
    jet_cut = (
        "Jet_pt >= 25.f && abs(Jet_eta) <= 2.4f && "
        "Jet_chHEF >= 0.01f && Jet_neHEF >= 0.f && Jet_neHEF < 0.99f && "
        "Jet_chEmEF >= 0.f && Jet_chEmEF < 0.8f && "
        "Jet_neEmEF >= 0.f && Jet_neEmEF < 0.9f && "
        "Jet_muEF >= 0.f && Jet_muEF < 0.8f && "
        "Jet_chMultiplicity >= 1 && Jet_chMultiplicity + Jet_neMultiplicity >= 2"
    )
    d = (ROOT.RDF.FromRNTuple("Events", files)
         .Define("selectedJet", jet_cut)
         .Define("ae", "abs(Jet_eta)")
         .Define("fl", "Jet_hadronFlavour"))
    booked = {}
    for fl in FLAVOURS:
        f = d.Define(f"m{fl}", f"selectedJet && fl == {fl}") \
             .Define(f"e{fl}", f"ae[m{fl}]").Define(f"p{fl}", f"Jet_pt[m{fl}]")
        model = (f"h_all_{fl}", "", len(ETA_EDGES) - 1, np.array(ETA_EDGES),
                 len(PT_EDGES) - 1, np.array(PT_EDGES))
        booked[("all", fl)] = f.Histo2D(model, f"e{fl}", f"p{fl}")
        for wp in WPS:
            g = (d.Define(f"m{fl}{wp}",
                          f"selectedJet && fl == {fl} && "
                          f"Jet_btagUParTAK4B > {thr[wp]}f")
                  .Define(f"e{fl}{wp}", f"ae[m{fl}{wp}]")
                  .Define(f"p{fl}{wp}", f"Jet_pt[m{fl}{wp}]"))
            model = (f"h_{wp}_{fl}", "", len(ETA_EDGES) - 1,
                     np.array(ETA_EDGES), len(PT_EDGES) - 1,
                     np.array(PT_EDGES))
            booked[(wp, fl)] = g.Histo2D(model, f"e{fl}{wp}", f"p{fl}{wp}")
    out = {}
    for k, h in booked.items():
        hh = h.GetValue()
        out[k] = np.array([[hh.GetBinContent(a + 1, p + 1)
                            for p in range(len(PT_EDGES) - 1)]
                           for a in range(len(ETA_EDGES) - 1)])
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--stride", type=int, default=50)
    ap.add_argument("--era", default="2024")
    ap.add_argument("--sample", default="TTLJ_powheg_CustomBPH")
    ap.add_argument("--max-files", type=int, default=0)
    ap.add_argument("--output", type=Path)
    args = ap.parse_args()

    thr = wp_thresholds()
    counts = collect(args.era, args.sample, args.stride, args.max_files, thr)
    for fl in FLAVOURS:
        print(f"  flavour {fl:1d}: {counts[('all', fl)].sum():,.0f} jets", flush=True)

    content = []
    thin = []
    for wp in WPS:
        wp_content = []
        for fl in FLAVOURS:
            den, num = counts[("all", fl)], counts[(wp, fl)]
            assert np.all(num <= den), f"tagged count exceeds denominator: wp={wp}, flav={fl}"
            incl = num.sum() / den.sum() if den.sum() else 0.5
            with np.errstate(invalid="ignore", divide="ignore"):
                eff = np.where(den > 0, num / np.maximum(den, 1), incl)
            # An empty or near-empty cell must not become 0 or 1: both make the
            # method-1a weight singular. Fall back to the flavour-inclusive
            # efficiency and record it.
            for a in range(len(ETA_EDGES) - 1):
                for p in range(len(PT_EDGES) - 1):
                    if den[a, p] < 100:
                        eff[a, p] = incl
                        thin.append((wp, fl, a, p, int(den[a, p])))
            eff = np.clip(eff, 1e-4, 1.0 - 1e-4)
            wp_content.append({
                "key": fl,
                "value": {
                    "nodetype": "multibinning",
                    "inputs": ["abseta", "pt"],
                    "edges": [ETA_EDGES, PT_EDGES],
                    "content": eff.flatten().tolist(),
                    "flow": "clamp",
                },
            })
        content.append({"key": wp,
                        "value": {"nodetype": "category", "input": "flavor",
                                  "content": wp_content}})

    cset = {
        "schema_version": 2,
        "description": (f"MC {TAGGER} b-tagging efficiency for the HadronAnalyzer "
                        f"jet selection, era {args.era}. Measured before event-level "
                        f"b-tagging on {args.sample} private NanoAOD; see "
                        f"MakeTaggingEffJson.py."),
        "corrections": [{
            "name": TAGGER,
            "version": 0,
            "inputs": [
                {"name": "systematic", "type": "string"},
                {"name": "working_point", "type": "string", "description": "L/M/T"},
                {"name": "flavor", "type": "int",
                 "description": "hadron flavor definition: 5=b, 4=c, 0=udsg"},
                {"name": "abseta", "type": "real"},
                {"name": "pt", "type": "real"},
            ],
            "output": {"name": "eff", "type": "real"},
            # The efficiency carries no uncertainty of its own here: the BTV SF
            # variation is the uncertainty method 1a propagates. All three
            # systematic keys therefore point at the same numbers.
            "data": {"nodetype": "category", "input": "systematic",
                     "content": [{"key": s, "value":
                                  {"nodetype": "category", "input": "working_point",
                                   "content": content}}
                                 for s in ("central", "up", "down")]},
        }],
    }

    out = args.output or (Path(__file__).resolve().parent / args.era / "BTV" /
                          "btaggingEff.json")
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(cset, indent=1))
    print(f"wrote {out}")
    if thin:
        print(f"  {len(thin)} cells below 100 jets, filled with the flavour-inclusive "
              f"efficiency:")
        for wp, fl, a, p, n in thin:
            print(f"    wp={wp} flav={fl} |eta|[{ETA_EDGES[a]},{ETA_EDGES[a+1]}) "
                  f"pt[{PT_EDGES[p]:.0f},{PT_EDGES[p+1]:.0f}) n={n}")

    import correctionlib
    c = correctionlib.CorrectionSet.from_file(str(out))[TAGGER]
    print("  readback:", {f"flav{fl}": round(c.evaluate("central", "L", fl, 0.5, 45.), 4)
                          for fl in FLAVOURS})


if __name__ == "__main__":
    main()
