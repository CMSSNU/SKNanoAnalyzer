# Development Guide

- [Development Guide](#development-guide)
  - [Setting up repository for central development](#setting-up-repository-for-central-development)
  - [Before Making Changes](#before-making-changes)
    - [Interplay between `SystematicHelper` and `Correction`](#interplay-between-systematichelper-and-correction)
    - [Avoid to use Magic Number or Magic Literal](#avoid-to-use-magic-number-or-magic-literal)
  - [Pull Request: HowTo](#pull-request-howto)
    - [Testing PRs open by others](#testing-prs-open-by-others)
    - [Open Pull Request](#open-pull-request)

## Setting up repository for central development
**Always start from the clean state!**
```bash
git clone --recurse-submodules git@github.com:$GITACCOUNT/SKNanoAnalyzer.git
git remote add upstream git@github.com:CMSSNU/SKNanoAnalyzer.git
# fetch the latest changes
git fetch upstream && git rebase upstream/main # if the rebase fails, check the conflicts and resolve them

# create a new branch for your development
git checkout -b $DEVBRANCH
# Now start your development
```

## Before Making Changes
### Interplay between `SystematicHelper` and `Correction`
This Analyzer is developed to use with `SystematicHelper` class. If you want to add some new correction, or etc. that relates with the systematic variation, please it's list of parameters should end with
```cpp
// Correction.h
float Correction::YOUR_NEW_FUNCTION(PARMETERS... , const variation syst, const TString &source="total")
```
the type of `variation` is defined in `Correction.h`, which is the `enum class` for the systematic variation. 
```cpp
enum class variation
{
    nom,
    up,
    down
};
```
`const TString &source` stands for the source of the systematic variation. Please use `total` for the total systematic variation and also for default value. If you want some example on this, `Correction::GetBTaggingSF` is one good example.

### Avoid to use Magic Number or Magic Literal
Please avoid to use string as key, Since it will make a confusion to which key should be passed to function. 
I recommend to use `enum class` for the key. For example, in [`Electron.h`](../DataFormats/include/Electron.h), I defined
```cpp
  enum class ElectronID {
      NOCUT,
      POG_VETO,
      POG_LOOSE,
      POG_MEDIUM,
      POG_TIGHT,
      POG_HEEP,
      POG_MVAISO_WP80,
      POG_MVAISO_WP90,
      POG_MVANOISO_WP80,
      POG_MVANOISO_WP90,
  };
```
and used it for further development. If one use string as key, It will be hard clarify to use among `"POG_TIGHT"` or `"TIGHT"` or `"T"` or `"POGTIGHT"` or etc.

In same sense, Pleas use `PhysicalConstants.h` under `AnalyzerTools/include` for the physical constants. This is included in `AnalyzerCore.h` by default, so you can use it without any additional include as
```cpp
// Veto Z mass window
if (fabs(mass - Z_MASS) < 15) return false;
```

## Pull Request: HowTo
### Testing PRs open by others
Here is a recommended way to test PRs.
```bash
# Assuming starting from the clean state and you are in the development branch
# fetch the PR
git fetch upstream pull/$PRNUMBER/head:pr$PRNUMBER
git checkout pr$PRNUMBER

# for example, fetching PR 16
git fetch upstream pull/16/head:pr16
git checkout pr16
```

### Setting ssh-key for gitlab.cern.ch within github workflow
- If you make changes in the CMSSNU/SKNanoAnalyzer repository, it will automatically execute the Github Actions to check if the environment is correctly set up and build is successful.
- To activave ssh authentication with gitlab.cern.ch, you should add the private key.
1. make a new ssh key
```bash
ssh-keygen -t ed25519 -C "gitlab-ci-key" -f ~/.ssh/id_ed25519_gitlab_gha
```
2. Add the public key to the gitlab repository. Go to the [gitlab.cern.ch](https://gitlab.cern.ch) -> Preferences -> SSH Keys -> Add an SSH key
3. Add the private key in the upstream repository. Go the the github repository -> Settings -> Secrets and variables -> Actions -> New repository secret. Add the private key as `SSH_PRIVATE_KEY`.
> pull_request_target trigger the workflow from the upstream repo, so nothing should be done from the forked repository.
