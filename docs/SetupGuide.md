# Setup Guide

[Documentation index](README.md)

This is the guide for someone setting SKNanoAnalyzer up for the first time. It
walks through `./bootstrap.sh`, the interactive wizard, and explains every
question it asks: what it means, what to type on the SNU cluster, what to type
anywhere else, what happens if you get it wrong, and how to change it later.

If you already know the framework and only want the manual steps, see
[Getting Started](GettingStarted.md#manual-setup).

## Index

- [1. Before you start](#1-before-you-start)
- [2. Clone and run the wizard](#2-clone-and-run-the-wizard)
- [3. The questions, one by one](#3-the-questions-one-by-one)
- [4. First run](#4-first-run)
- [5. Common errors](#5-common-errors)
- [6. Where the answers are stored](#6-where-the-answers-are-stored)

## 1. Before you start

You need three things.

**A shell account on the machine you will work on.** On the SNU cluster this is
your account on the login node (`tamsa1`/`tamsa2`). Everything below is run
there, in a terminal.

**Git and curl.** Both are present on the cluster. On a personal Linux machine,
install them with your package manager. macOS gets both from the Xcode command
line tools (`xcode-select --install`).

**An ssh key registered at gitlab.cern.ch.** One submodule, `external/RoccoR`
(the muon momentum corrections), is hosted there and is cloned over ssh, so
without a key the wizard cannot fetch it and the build will fail. Create a key
and register it:

```bash
# Press Enter at every prompt to accept the defaults.
ssh-keygen -t ed25519 -C "your.name@cern.ch"

# Print the public half and copy the whole line.
cat ~/.ssh/id_ed25519.pub
```

Open <https://gitlab.cern.ch> → your avatar → **Preferences** → **SSH Keys** →
**Add new key**, paste the line, and save. Check that it works:

```bash
ssh -T git@gitlab.cern.ch -p 7999
```

A greeting means the key is accepted. `Permission denied (publickey)` means it
is not; the most common cause is pasting the private key file
(`id_ed25519`) instead of the public one (`id_ed25519.pub`).

You do *not* need a key for `external/jsonpog-integration`. That submodule is
optional: the same corrections are read through the `CMS_corrections` symlink
into `/cvmfs` whenever cvmfs is mounted.

## 2. Clone and run the wizard

Forking the repository to your own GitHub account first is recommended, so that
you have somewhere to push your work.

```bash
git clone --recurse-submodules git@github.com:CMSSNU/SKNanoAnalyzer.git
cd SKNanoAnalyzer
./bootstrap.sh
```

`--recurse-submodules` may print errors if your gitlab key is not registered
yet; that is not fatal here, the wizard asks about submodules again later.

The wizard walks through eleven steps. Every question shows a default in square
brackets, and pressing **Enter** accepts it. On the SNU cluster the defaults are
the right answer everywhere, so a first-time cluster user can just press Enter
at every prompt.

Two things worth knowing before you start:

- **It is safe to re-run.** Every step checks whether it is already done and
  reports `already done:` instead of redoing it. If a step fails, fix the cause
  and run `./bootstrap.sh` again.
- **`./bootstrap.sh --yes` asks nothing** and takes every default. Use it in CI,
  or to resume after you have already answered the questions once.

The two long steps are creating the package environment (20–30 minutes) and the
first build (also tens of minutes, because it compiles LHAPDF from source and
downloads LibTorch). Both can be answered `n` and done later.

## 3. The questions, one by one

### Q1 — Path to the micromamba executable

**What it is.** Micromamba is a small, fast, conda-compatible package manager.
It is what installs ROOT, correctionlib, ONNX Runtime and the compiler as one
consistent set. This question is about the location of the `micromamba` program
itself, which is a single file of a few tens of MB.

**What to enter.** If the wizard found an existing micromamba it offers that
path — press Enter. If it found none, it offers to download and install one, and
then asks for the directory to put it in.

- **SNU cluster:** accept `/data6/Users/$USER/micromamba_bin`.
- **Other machines:** accept `$HOME/micromamba_bin`, or point at a micromamba
  you already have.

**Why not the home directory on a cluster.** Worker nodes usually do not mount
`/home`. A micromamba that lives only in your home directory works
interactively and then fails in every batch job.

**If you get it wrong.** `source setup.sh` fails with
`micromamba: command not found`.

**Changing it later.** The `[MAMBA_EXE]` line in `config/config.$USER`.

The wizard downloads the release tarball directly rather than running the
official `install.sh`, because that script is interactive and edits your shell
profile. Nothing in your `.bashrc` or `.zshrc` is touched — the path is recorded
in the config file, and `setup.sh` reads it from there.

### Q2 — Environment root (`MAMBA_ROOT_PREFIX`)

**What it is.** Where the *environments* live, as opposed to the micromamba
program from Q1. The `Nano` environment is several GB, so this needs real space.
Micromamba creates `<root>/envs/Nano` under it.

**What to enter.**

- **SNU cluster:** `/data6/Users/$USER/micromamba_envs` (the default). Any
  shared-storage path works as long as the worker nodes can read it.
- **Other machines:** `$HOME/micromamba` is fine.

**If you get it wrong.** If you put it somewhere the worker nodes cannot see,
interactive work is fine and every condor job dies at startup, because the batch
image reaches the environment through this path.

**Changing it later.** `[MAMBA_ROOT_PREFIX]` in `config/config.$USER`. Moving an
existing environment root also means rebuilding the batch image (Q7), which
symlinks into it.

### Q3 — Create the `Nano` environment

**What it is.** The environment that holds ROOT 6.40, GCC 13.3, correctionlib,
ONNX Runtime and the Python stack. The name must be exactly `Nano`; `setup.sh`
and the batch jobs look for that name.

**What to enter.** `y` unless you already have it. The wizard installs from
`docs/Nano-linux-64.lock`, a pinned list of exact package URLs, which is
reproducible and needs no dependency solving. It falls back to solving
`docs/Nano.yml` when a package URL has been retired upstream, and on any
platform that has no lock file (macOS, for instance).

This takes roughly 20–30 minutes and downloads a few GB.

**Do not upgrade ROOT, Abseil, correctionlib or the compiler on their own
afterwards.** They are one C++ ABI-compatible set. Mixing ROOT 6.40 (C++20) with
older Abseil headers makes Cling load the removed `<ciso646>` header, and every
analyzer job then crashes at startup.

**If you skip it.** `source setup.sh` fails at `micromamba activate Nano`. Run
the wizard again, or create it by hand:

```bash
micromamba create -n Nano -f docs/Nano-linux-64.lock
```

### Q4 — Output directory (`SKNANO_OUTPUT`)

**What it is.** Where analysis results are written — merged histograms and
RNTuple files, one subdirectory per analyzer and era. It grows to hundreds of
GB, so it belongs on bulk storage, not in your home directory.

**What to enter.**

- **SNU cluster:** `/gv0/Users/$USER/SKNanoOutput` (the default).
- **Other machines:** any directory on a disk with room to spare.

The wizard creates the directory and verifies it can write a file there.

**Changing it later.** `[SKNANO_OUTPUT]` in `config/config.$USER`.

### Q5 — Log directory (`SKNANO_RUNLOG`)

**What it is.** Where each submission writes its run directory: the generated
condor and DAG files, per-job logs, a source snapshot, and
`run_manifest.json`. This is where you look when a job fails.

**What to enter.** The same rule as Q4 — `/gv0/Users/$USER/SKNanoRunlog` on the
cluster. It is much smaller than the output directory but has many small files.

**Changing it later.** `[SKNANO_RUNLOG]` in `config/config.$USER`.

### Q6 — Input root (`SKNANO_INPUT_ROOT`)

**What it is.** The top directory of the NanoAOD production you analyse. Sample
metadata deliberately stores only the part of the path *below* this, so moving
to a different site or a different production is one value here rather than an
edit to every sample json.

**What to enter.**

- **SNU cluster:** `/gv0/DATA/SKNano/NanoAODv15_RNTuple` (the default).
- **Other machines:** wherever your copy of the production lives. If you have
  none yet, accept the default; you can still build the framework and run over
  local files.

The wizard only warns when the directory does not exist — it never stops.

**If you get it wrong.** `SKNano.py` finds no input files for any sample. Check
what resolves with:

```bash
python3 python/sampleManager.py --era 2024 --checkSamplePaths
```

**Changing it later.** `[SKNANO_INPUT_ROOT]` in `config/config.$USER`. This is
also how you keep working against an older production while a new one is still
being written.

### Q7 — Batch execution

**What it is.** Condor jobs run inside an Apptainer (formerly Singularity)
image. The image is deliberately thin: a base AlmaLinux 9 plus micromamba, with
the `Nano` environment from Q2 *symlinked* in rather than copied. That way
installing a package does not mean rebuilding the image.

**The three choices.**

- **(a) an existing `.sif`** — you already built one, or a colleague shares one.
  You are then asked for its absolute path.
- **(b) build one now** — the wizard substitutes your Q2 path into
  `templates/Nano.def` and runs `apptainer build --fakeroot`. It then asks where
  to write it; the default is `<parent of your output dir>/Image/Nano.sif`.
  The build takes a few minutes and needs network access.
- **(c) none** — no image. Everything still builds and runs locally; only condor
  submission is unavailable.

**What to enter.** `b` the first time on the cluster, `a` afterwards, `c` on a
laptop.

**If you get it wrong.** With a wrong or missing image path, condor jobs fail
immediately at startup. An empty value is a valid, supported answer: `SKNano.py`
then uses `MAMBA_ROOT_PREFIX` directly instead of a container.

**Changing it later.** `[SINGULARITY_IMAGE]` in `config/config.$USER`. To build
the image by hand at any time:

```bash
sed "s|.NANO_ENV_PATH.|$MAMBA_ROOT_PREFIX|g" templates/Nano.def > /tmp/Nano.def
apptainer build --fakeroot /path/to/Nano.sif /tmp/Nano.def
```

### Q8 — Telegram notifications

**What it is.** Optional. With a bot configured, `SKNano.py` sends a submission
summary when you submit and a completion report when the DAG finishes: elapsed
time, output counts, failed nodes, common failure signatures, and the rescue
command if a retry is needed.

**What to enter.** Press Enter twice to leave it off. To turn it on:

1. In Telegram, message `@BotFather` and send `/newbot`. Follow the prompts; it
   replies with a token that looks like `1234567890:AA...`.
2. Search for your new bot, open it, and send it any message.
3. Open `https://api.telegram.org/bot<TOKEN>/getUpdates` in a browser and read
   your chat id out of `"from":{"id":<CHAT_ID>`.

Give the token to the first question and the chat id to the second.

**Security.** The token lives in `config/config.$USER`, which the wizard chmods
to `600`. It is never copied into generated DAG scripts. Delivery is
best-effort: a bad token is written to the reporter log and never changes an
analysis result.

**Changing it later.** `[TOKEN_TELEGRAMBOT]` and `[USER_CHATID]` in
`config/config.$USER`. Blanking either one disables reporting.

### Q9 — Initialise git submodules

**What it is.** `git submodule update --init --recursive`.

**What to enter.** `y`.

**What can go wrong.** Cloning `external/RoccoR` needs the gitlab.cern.ch ssh
key from [section 1](#1-before-you-start). Without it you get
`Permission denied (publickey)`. The wizard does not abort — it tells you which
submodule is missing and moves on — but the build will fail until you fix it:

```bash
# after registering the key
git submodule update --init external/RoccoR
```

`external/jsonpog-integration` failing is only a warning: the corrections it
carries are also reachable through `CMS_corrections`, a symlink into
`/cvmfs/cms-griddata.cern.ch`, and that is what `setup.sh` points
`JSONPOG_REPO_PATH` at. If you have no cvmfs *and* no jsonpog-integration,
correction lookups fail at runtime; see
[section 5](#5-common-errors) for the override.

The analysis-group submodules (`Vcb_Analyzers`, `Hadron_Analyzers`, …) are
private repositories. Failing to clone them is expected and harmless unless you
work on that analysis.

### Q10 — Environment check

No question. The wizard reports whether cvmfs is mounted, whether `/gv0` is
reachable, which compiler is on `PATH`, whether `git`, `curl`, `tar`, `unzip`
and `wget` exist, and how much disk space is free on the three directories it
will use. Everything here is a warning; nothing stops the setup.

### Q11 — Build the project now

**What it is.** `source setup.sh && ./scripts/build.sh`.

**What to enter.** `y`.

The *first* build is much slower than later ones, because `setup.sh` also
compiles LHAPDF from source and downloads LibTorch (about 180 MB) on its first
run. Later builds are incremental.

If it fails, the wizard prints the command to re-run by hand. Nothing else in
the setup is undone.

## 4. First run

Open a new shell and set the environment up. You must do this **once in every
new shell** — it is what puts `SKNano.py` on your `PATH` and activates the
`Nano` environment:

```bash
cd /path/to/SKNanoAnalyzer
source setup.sh
```

A healthy run prints the banner, `Reading configuration from …`, the paths you
chose, and the ROOT, LHAPDF, correctionlib and ONNX Runtime locations.

Run the unit tests:

```bash
ctest --test-dir "$SKNANO_BUILDDIR" --output-on-failure
```

Then the smoke test — ten jobs over a single sample with the example analyzer:

```bash
SKNano.py -a ExampleRun -i DYto2E_MLL50to120 -e 2024 -n 10
```

Watch it with `condor_q`, and check the run directory that the submission
printed, under your `SKNANO_RUNLOG`. When the DAG finishes, the merged output
is at:

```bash
ls $SKNANO_OUTPUT/ExampleRun/2024/
```

One `.root` file per sample means the whole chain — environment, build,
container, condor, merge — works.

`SKNano.py` always submits to HTCondor. On a machine with no pool, add
`--no_exec`: it writes the whole working area under `SKNANO_RUNLOG` without
submitting anything. The `run.sh` it generates there takes a job index —
numbered from 1, matching the `job_<n>.cc` files beside it — and runs that one
job.

```bash
SKNano.py -a ExampleRun -i DYto2E_MLL50to120 -e 2024 -n 1 --no_exec
```

Then, in the working directory the submission printed:

```bash
# with [SINGULARITY_IMAGE] set, run.sh expects the container's own /opt/conda
apptainer exec -B /gv0 -B /data6 "$SINGULARITY_IMAGE" bash run.sh 1

# with [SINGULARITY_IMAGE] empty, run.sh points at your MAMBA_ROOT_PREFIX
bash run.sh 1
```

`run.sh` is not marked executable — HTCondor sets that itself — so invoke it
through `bash`.

See [Getting Started](GettingStarted.md#how-to-submit-the-job) for the full
option list.

## 5. Common errors

**`Permission denied (publickey)` while initialising submodules.**
Your gitlab.cern.ch ssh key is missing or not registered. Follow
[section 1](#1-before-you-start), verify with
`ssh -T git@gitlab.cern.ch -p 7999`, then
`git submodule update --init external/RoccoR`.

**`micromamba: command not found` from `setup.sh`.**
`[MAMBA_EXE]` in `config/config.$USER` is empty or points at a file that is not
there. Re-run `./bootstrap.sh`, or fix the line by hand.

**`micromamba activate Nano` fails, or `Nano` is not found.**
The environment does not exist under the `[MAMBA_ROOT_PREFIX]` you configured.
Check with `micromamba env list`. The two paths must agree: an environment
created under a different root prefix is invisible. Create it with
`micromamba create -n Nano -f docs/Nano-linux-64.lock`.

**`correctionlib not found in conda environment`.**
`setup.sh` activated something other than the intended `Nano` environment —
usually a stale `conda`/`mamba` initialisation block in your `.bashrc` or
`.zshrc`. Run `micromamba env list` and `echo $CONDA_PREFIX` after sourcing to
see what was actually activated.

**`apptainer build` fails with a `--fakeroot` error.**
Your account needs `/etc/subuid` and `/etc/subgid` entries for unprivileged
image building; ask the cluster administrator. Alternatives: build the image on
a machine where you do have rights and copy the `.sif` over, or answer **(c)**
at Q7 and use a colleague's image by putting its path in
`[SINGULARITY_IMAGE]`.

**cvmfs is not mounted.**
`/cvmfs/cms-griddata.cern.ch` is absent on most non-CMS machines, and the
`CMS_corrections` symlink then dangles. Initialise
`external/jsonpog-integration` and override the variable after sourcing, since
`setup.sh` points it at `CMS_corrections` unconditionally:

```bash
source setup.sh
export JSONPOG_REPO_PATH=$SKNANO_HOME/external/jsonpog-integration
```

**LibTorch download fails.**
`scripts/install_libtorch.sh` pins an exact build. Nightly builds are eventually
removed upstream; if the pinned one is gone, pick another from
<https://download.pytorch.org/libtorch/> and re-run:

```bash
LIBTORCH_VERSION=2.9.0+cpu ./scripts/install_libtorch.sh
```

**The build succeeds but jobs die immediately on the cluster.**
Almost always a path that the worker nodes cannot see. Check that
`[MAMBA_ROOT_PREFIX]`, `[SKNANO_OUTPUT]` and `[SKNANO_RUNLOG]` are all on shared
storage and none of them is under `/home`.

## 6. Where the answers are stored

Everything the wizard learns goes into one file, `config/config.$USER`, which is
gitignored and readable only by you:

```
[PACKAGE] mamba
[MAMBA_EXE] /data6/Users/me/micromamba_bin/micromamba
[MAMBA_ROOT_PREFIX] /data6/Users/me/micromamba_envs
[SKNANO_OUTPUT] /gv0/Users/me/SKNanoOutput
[SKNANO_RUNLOG] /gv0/Users/me/SKNanoRunlog
[SKNANO_INPUT_ROOT] /gv0/DATA/SKNano/NanoAODv15_RNTuple
[TOKEN_TELEGRAMBOT]
[USER_CHATID]
[SINGULARITY_IMAGE] /gv0/Users/me/Image/Nano.sif
```

`setup.sh` reads it on every `source`. The format is one `[KEY] value` per line:

- A key that is present but has **no value keeps the built-in default**. That is
  why `config/config.default` can list every key empty.
- **Values may not contain spaces.** The parser takes the second
  whitespace-separated field, so a path with a space is silently truncated. The
  wizard refuses such a path; a hand edit will not.
- `[SINGULARITY_IMAGE]` empty is meaningful, not broken: it selects
  container-less execution.

`config/config.default` is the template listing every supported key.

