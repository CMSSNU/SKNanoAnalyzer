# Development Guide

- [Development Guide](#development-guide)
  - [Before Making Changes](#before-making-changes)
  - [Review the Development done by others](#review-the-development-done-by-others)
    - [Testing PRs](#testing-prs)


## Before Making Changes
This Analyzer is developed to use with `SystematicHelper` class. If you want to add some new correction, or etc. that relates with the systematic variation, please end it's list of parameters should end with
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
## Review the Development done by others
### Testing PRs
Here is a recommended way to test PRs.
```bash
# Always start from the clean state
git clone --recurse-submodules git@github.com:$GITACCOUNT/SKNanoAnalyzer.git
cd SKNanoAnalyzer
git remote add upstream git@github.com:CMSSNU/SKNanoAnalyzer.git

# fetch the PR
git fetch upstream pull/$PRNUMBER/head:pr$PRNUMBER
git checkout pr$PRNUMBER

# for example, fetching PR 16
git fetch upstream pull/16/head:pr16
git checkout pr16
```

