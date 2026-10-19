# Development Guide

[Documentation index](README.md)

- [Repository setup](#repository-setup)
- [Systematic-aware corrections](#systematic-aware-corrections)
- [Typed constants and identifiers](#typed-constants-and-identifiers)
- [Unit tests](#unit-tests)
- [Repository size](#repository-size)
- [Testing another pull request](#testing-another-pull-request)
- [GitLab authentication in GitHub Actions](#gitlab-authentication-in-github-actions)

## Repository setup

Start development from an up-to-date branch with a clean working tree:

```bash
git clone --recurse-submodules git@github.com:$GITACCOUNT/SKNanoAnalyzer.git
git remote add upstream git@github.com:CMSSNU/SKNanoAnalyzer.git
git fetch upstream
git rebase upstream/main
git checkout -b $DEVBRANCH
```

Physics analyzers do not belong in the backend repository. Read
[Analyzer Development](AnalyzerDevelopment.md) before creating a new analyzer
or custom NanoAOD view.

## Systematic-aware corrections

Corrections managed by `SystematicHelper` should end their parameter list with
the systematic variation and source:

```cpp
// Correction.h
float Correction::YOUR_NEW_FUNCTION(PARAMETERS..., const variation syst,
                                      const TString &source = "total");
```

`variation` is the scoped enumeration defined in `Correction.h`:

```cpp
enum class variation { nom, up, down };
```

Use `"total"` for the combined uncertainty and as the default source. Refer to
`Correction::GetBTaggingSF` for an existing implementation. The AK4 and AK8
JES lookups accept `"total"` in any case, or an empty source, and normalise it
to the JME `"Total"` key; other sources must match the JSON key exactly.

## Typed constants and identifiers

Prefer `enum class` values to ambiguous string keys. Electron IDs, for example,
are defined in [`LeptonIDEnums.h`](../DataFormats/include/LeptonIDEnums.h):

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

Use the constants in `AnalyzerTools/include/PhysicalConstants.h` instead of
embedding numeric literals. `AnalyzerCore.h` already includes this header:

```cpp
// Veto Z mass window
if (std::abs(mass - Z_MASS) < 15.)
    return false;
```

## Unit tests

Tests are [GoogleTest](https://google.github.io/googletest/) suites registered
with `ctest`. gtest is declared in [`Nano.yml`](Nano.yml), so a rebuilt
environment already has it.

```bash
source setup.sh
./scripts/build.sh
ctest --test-dir "$SKNANO_BUILDDIR" --output-on-failure
ctest --test-dir "$SKNANO_BUILDDIR" -N    # list without running
```

`BUILD_TESTING` is on by default via `include(CTest)`; configure with
`-DBUILD_TESTING=OFF` to skip building them.

A module keeps its suites in `<Module>/tests/`, wired from that module's
`CMakeLists.txt`:

```cmake
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
```

Hand-built SoA fixtures live in
[`DataFormats/tests`](../DataFormats/tests) and are exported as the
`SKNanoTestSupport` interface target — link that rather than naming the
directory by path.

Two rules keep the suites usable on machines that are not this one:

- **Do not set the `ENVIRONMENT` test property.** `ctest` inherits the shell it
  runs in, so a suite sees whatever `setup.sh` exported. Copying variables in
  at configure time pins them to whoever configured the tree.
- **Skip, do not fail, when an input is unreachable.** Which correction files
  exist is decided by the era yml, so probe the object and
  `GTEST_SKIP()` on the exception instead of testing a hardcoded path.
  `AnalyzerTools/tests/TauCorrectionTest.cc` does this.

Layer and codegen checks (`scripts/check_layers.sh`, the analysis-module
contract tests) are plain scripts or assert-based executables registered the
same way; they predate gtest and there is no need to convert them.

## Repository size

`.git` is far larger than the working tree, because `data/` used to carry
enumerated input paths and multi-megabyte JSON that does not delta-compress.
The sources of growth are gone — sample inputs derive from
`$SKNANO_INPUT_ROOT` and the large LUTs live with the module that owns them —
but the objects already written stay in history.

Reclaiming them means rewriting history: `git filter-repo` to drop the large
blobs, a force-push of every branch, and every existing clone becoming invalid.
That is a coordinated operation, not a cleanup commit, so it is deliberately not
bundled with the changes that stopped the growth. Until it happens, avoid
committing anything large under `data/`: the era yml resolves correction files
from outside the repository and sample inputs need no paths at all.

A local `git gc` is safe and worth running; note that interrupted operations can
leave `.git/objects/pack/tmp_pack_*` files behind, which `gc` will not remove on
its own.

## Testing another pull request

Fetch a pull request into a local branch before building and testing it:

```bash
git fetch upstream pull/$PRNUMBER/head:pr$PRNUMBER
git checkout pr$PRNUMBER

# Example: PR 16
git fetch upstream pull/16/head:pr16
git checkout pr16
```

## GitLab authentication in GitHub Actions

The upstream GitHub Actions workflow needs an SSH key to access dependencies
on `gitlab.cern.ch`:

1. Create a dedicated key:

```bash
ssh-keygen -t ed25519 -C "gitlab-ci-key" -f ~/.ssh/id_ed25519_gitlab_gha
```

2. Add the public key under GitLab **Preferences > SSH Keys**.
3. Add the private key to the upstream GitHub repository as the Actions secret
   `SSH_PRIVATE_KEY`.

The `pull_request_target` workflow runs in the upstream repository context, so
adding this secret to a fork does not configure the upstream job.
