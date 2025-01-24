# Development Guide

- [Development Guide](#development-guide)
  - [Before Making Changes](#before-making-changes)
    - [Interplay between `SystematicHelper` and `Correction`](#interplay-between-systematichelper-and-correction)
    - [Avoid to use Magic Number or Magic Literal](#avoid-to-use-magic-number-or-magic-literal)
  - [Review the Development done by others](#review-the-development-done-by-others)
    - [Testing PRs](#testing-prs)


## Before Making Changes
### Interplay between `SystematicHelper` and `Correction`
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
and used it for further development. If one use string as key, It will be hard clarify to use aming `"POG_TIGHT"` or `"TIGHT"` or `"T"` or `"POGTIGHT"` or etc.

In same sense, Pleas use `PhysicalConstants.h` under `AnalyzerTools/include` for the physical constants. This is included in `AnalyzerCore.h` by default, so you can use it without any additional include as
```cpp
// Veto Z mass window
if (fabs(mass - Z_MASS) < 15) return false;
```


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

