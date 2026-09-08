# Setting up micromamba + singularity environment

[Documentation index](README.md)
 
## Overview

This guide sets up the shared micromamba environment and Singularity image used
to run SKNanoAnalyzer on the SNU cluster, by hand. `./bootstrap.sh` automates
all of it — see the [Setup Guide](SetupGuide.md) — so read this page when you
want to know what the wizard is doing, or when a step needs fixing.

### What is micromamba?

Micromamba is a lightweight conda-compatible package manager. It installs the
compiler, ROOT, correction libraries, and Python tools as one versioned
environment.

### What is singularity?

Singularity, also distributed as Apptainer, gives batch nodes a consistent base
operating system. The image mounts the shared `Nano` environment, keeping the
large package environment outside the image and available to every worker.

## Setting up micromamba
### Install micromamba

`./bootstrap.sh` installs micromamba for you (see [Setup Guide](SetupGuide.md));
this section is the manual equivalent.

The upstream installer is interactive and edits your shell profile:

```bash
"${SHELL}" <(curl -L micro.mamba.pm/install.sh)
```

Two of its defaults are wrong on a cluster and must be changed.

```
Micromamba binary folder? [~/.local/bin]
```
***Do not use the default path.*** Use `/data6/Users/$USER/micromamba_bin`
instead. The default is under the home directory, which the worker nodes do not
mount.

```
Prefix location? [~/micromamba]
```
***Do not use the default path,*** for the same reason. Use
`/data6/Users/$USER/micromamba_envs`.

Answer `Y` to `Init shell` and to `Configure conda-forge`.

The installer appends an initialisation block to your shell profile:

```bash
# >>> mamba initialize >>>
# !! Contents within this block are managed by 'micromamba shell init' !!
export MAMBA_EXE='/data6/Users/your_id/micromamba_bin/micromamba';
export MAMBA_ROOT_PREFIX='/data6/Users/your_id/micromamba_envs';
__mamba_setup="$("$MAMBA_EXE" shell hook --shell bash --root-prefix "$MAMBA_ROOT_PREFIX" 2> /dev/null)"
if [ $? -eq 0 ]; then
    eval "$__mamba_setup"
else
    alias micromamba="$MAMBA_EXE"  # Fallback on help from micromamba activate
fi
unset __mamba_setup
# <<< mamba initialize <<<
```

That block is optional for SKNanoAnalyzer: `setup.sh` reads the same two paths
from `[MAMBA_EXE]` and `[MAMBA_ROOT_PREFIX]` in `config/config.$USER`, so a
checkout works even in a shell that never initialised micromamba. Record them
there whichever way you installed it.

After restarting your terminal, verify:
```bash
micromamba --version
```

### Install required packages

From the repository root, create the shared `Nano` environment from the pinned
Linux lock file:

```bash
micromamba create -n Nano -f docs/Nano-linux-64.lock
micromamba activate Nano
```

If a locked package URL is no longer available, use the solver input with
`micromamba env create -f docs/Nano.yml`. Do not upgrade ROOT, Abseil,
correctionlib, or the compiler independently; they form one C++ ABI-compatible
toolchain.

## Setting up Singularity
### Make Singularity image

Now we need to create the Singularity image.
Fundamentally, this is conceptually almost identical to building a new computer and installing the OS fresh. Therefore, you must configure the necessary environment settings when creating the image. Then, each time you run the image, it's like booting up a new computer with exactly this configuration already complete.

This is done through a `.def` file. The repository ships one at
[`templates/Nano.def`](../templates/Nano.def), so there is nothing to paste: it
carries a single placeholder for the environment root you chose above.

```bash
sed "s|.NANO_ENV_PATH.|$MAMBA_ROOT_PREFIX|g" templates/Nano.def > Nano.def
apptainer build --fakeroot Nano.sif Nano.def
```

`./bootstrap.sh` does exactly this when you answer **(b)** to the batch
execution question.

The definition downloads an AlmaLinux 9 base image and installs micromamba into
it:

```bash
curl -Ls https://micro.mamba.pm/api/micromamba/linux-64/latest | tar -xvj bin/micromamba
```

That micromamba does not contain `root`, `correctionlib`, or anything else we
installed earlier. Rather than rebuilding the image every time a package
changes, the `Nano` environment stays outside the image and is symlinked in:

```bash
ln -sfn [NANO_ENV_PATH]/envs/Nano /opt/conda/envs/Nano || true
echo 'export PATH=/opt/conda/bin:$PATH' > /etc/profile.d/conda.sh
echo 'export MAMBA_ROOT_PREFIX=/opt/conda' >> /etc/profile.d/conda.sh
```

The consequence is that the environment root must be on storage the worker nodes
can read, and that moving it means rebuilding the image.

Put the resulting path in `[SINGULARITY_IMAGE]` in `config/config.$USER`. An
empty value is valid and means "run without a container".

After building the image, return to [Getting Started](GettingStarted.md) to
configure, build, and run SKNanoAnalyzer.
