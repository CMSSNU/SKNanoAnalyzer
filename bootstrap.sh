#!/usr/bin/env bash
#
# SKNanoAnalyzer one-shot setup wizard.
#
#     ./bootstrap.sh          # interactive: every question has a default
#     ./bootstrap.sh --yes    # accept every default (CI, or a second run)
#
# It asks a handful of questions, writes config/config.$USER, and performs the
# steps a new user would otherwise do by hand: install micromamba, create the
# Nano environment, create the output directories, build the Apptainer image
# used by batch jobs, initialise the submodules, and build the project.
#
# Every step is idempotent. A second run detects what is already in place and
# reports it as "already done" instead of redoing it.
#
# The companion document is docs/SetupGuide.md, which explains what each
# question means and which value to give on which machine.

set -uo pipefail

# ---------------------------------------------------------------------------
# Basics
# ---------------------------------------------------------------------------

if [[ -n "${BASH_VERSION:-}" ]]; then
    if [[ "${BASH_SOURCE[0]}" != "$0" ]]; then
        echo "bootstrap.sh must be executed, not sourced: ./bootstrap.sh" >&2
        return 1
    fi
else
    echo "bootstrap.sh needs bash. Run it as ./bootstrap.sh" >&2
    exit 1
fi

SKNANO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$SKNANO_ROOT" || exit 1

if [[ ! -f setup.sh || ! -d config ]]; then
    echo "bootstrap.sh must live in the SKNanoAnalyzer repository root." >&2
    exit 1
fi

ASSUME_YES=0
for arg in "$@"; do
    case "$arg" in
        -y|--yes) ASSUME_YES=1 ;;
        -h|--help)
            # The header comment, up to the first line that is not a comment.
            awk 'NR < 3 { next } /^#/ { sub(/^# ?/, ""); print; next } { exit }' \
                "${BASH_SOURCE[0]}"
            exit 0
            ;;
        *)
            echo "Unknown option: $arg (try --help)" >&2
            exit 1
            ;;
    esac
done

if [[ ! -t 0 && $ASSUME_YES -eq 0 ]]; then
    echo "stdin is not a terminal; continuing as if --yes had been given."
    ASSUME_YES=1
fi

if [[ -t 1 && -z "${NO_COLOR:-}" ]]; then
    C_RESET=$'\033[0m'; C_BOLD=$'\033[1m'; C_GREEN=$'\033[32m'
    C_YELLOW=$'\033[33m'; C_RED=$'\033[31m'; C_BLUE=$'\033[36m'
else
    C_RESET=""; C_BOLD=""; C_GREEN=""; C_YELLOW=""; C_RED=""; C_BLUE=""
fi

STEP_NO=0
WARNINGS=()

step()  { STEP_NO=$((STEP_NO + 1)); printf '\n%s[%d/%d] %s%s\n' "$C_BOLD$C_BLUE" "$STEP_NO" "$TOTAL_STEPS" "$*" "$C_RESET"; }
info()  { printf '      %s\n' "$*"; }
ok()    { printf '      %s%s%s\n' "$C_GREEN" "$*" "$C_RESET"; }
skip()  { printf '      %s%s%s\n' "$C_GREEN" "already done: $*" "$C_RESET"; }
warn()  { printf '      %s%s%s\n' "$C_YELLOW" "warning: $*" "$C_RESET"; WARNINGS+=("$*"); }
fail()  { printf '      %s%s%s\n' "$C_RED" "error: $*" "$C_RESET"; }

TOTAL_STEPS=11

# ---------------------------------------------------------------------------
# Question helpers -- the only two the script uses
# ---------------------------------------------------------------------------

# ask VAR "question" "default"
ask() {
    local __var=$1 __prompt=$2 __default=${3:-} __reply=""
    if [[ $ASSUME_YES -eq 1 ]]; then
        __reply=$__default
        printf '      %s [%s] -> %s\n' "$__prompt" "$__default" "${__default:-(empty)}"
    else
        printf '\n'
        if ! IFS= read -e -r -p "      $__prompt [$__default]: " __reply; then
            printf '\n'
            __reply=""
        fi
        [[ -z $__reply ]] && __reply=$__default
    fi
    printf -v "$__var" '%s' "$__reply"
}

# ask_yn "question" [y|n]  -> returns 0 on yes
ask_yn() {
    local __prompt=$1 __default=${2:-y} __reply="" __hint
    [[ $__default == y ]] && __hint="Y/n" || __hint="y/N"
    if [[ $ASSUME_YES -eq 1 ]]; then
        printf '      %s [%s] -> %s\n' "$__prompt" "$__hint" "$__default"
        [[ $__default == y ]]
        return
    fi
    while true; do
        printf '\n'
        if ! IFS= read -e -r -p "      $__prompt [$__hint]: " __reply; then
            printf '\n'
            __reply=""
        fi
        [[ -z $__reply ]] && __reply=$__default
        case "${__reply,,}" in
            y|yes) return 0 ;;
            n|no)  return 1 ;;
            *) printf '      Please answer y or n.\n' ;;
        esac
    done
}

# ---------------------------------------------------------------------------
# Path helpers
# ---------------------------------------------------------------------------

# A path we may create: writable today, or with a writable existing ancestor.
dir_is_usable() {
    local d=${1:-}
    [[ -n $d ]] || return 1
    if [[ -d $d ]]; then
        [[ -w $d ]]
        return
    fi
    local parent=$d
    while [[ ! -e $parent && $parent != / && $parent != . ]]; do
        parent=$(dirname "$parent")
    done
    [[ -d $parent && -w $parent ]]
}

# Reject values that setup.sh cannot parse: it reads "[KEY] value" and takes the
# second whitespace-separated field, so a value containing whitespace would be
# silently truncated.
value_is_clean() {
    [[ "$1" != *[[:space:]]* ]]
}

ask_path() {
    local __var=$1 __prompt=$2 __default=$3 __value
    while true; do
        ask __value "$__prompt" "$__default"
        if [[ -n $__value ]] && ! value_is_clean "$__value"; then
            fail "the path may not contain spaces; setup.sh cannot read it back."
            [[ $ASSUME_YES -eq 1 ]] && break
            continue
        fi
        break
    done
    printf -v "$__var" '%s' "$__value"
}

# ---------------------------------------------------------------------------
# config/config.$USER
# ---------------------------------------------------------------------------

CONFIG_FILE="$SKNANO_ROOT/config/config.$USER"

config_get() {
    local key=$1
    [[ -f $CONFIG_FILE ]] || return 0
    # awk, not cut: cut prints the whole line when it holds no delimiter, so a
    # valueless "[KEY]" would come back as the literal string "[KEY]".
    awk -v k="[$key]" '$1 == k { print $2; exit }' "$CONFIG_FILE" 2>/dev/null
}

config_set() {
    local key=$1 value=$2 tmp
    tmp="$CONFIG_FILE.tmp.$$"
    if [[ -f $CONFIG_FILE ]] && grep -q "^\[$key\]" "$CONFIG_FILE"; then
        awk -v k="[$key]" -v v="$value" \
            '$1 == k { print (v == "" ? k : k " " v); next } { print }' \
            "$CONFIG_FILE" > "$tmp" && mv "$tmp" "$CONFIG_FILE"
    else
        printf '[%s]%s\n' "$key" "${value:+ $value}" >> "$CONFIG_FILE"
    fi
}

if [[ -f $CONFIG_FILE ]]; then
    CONFIG_EXISTED=1
else
    CONFIG_EXISTED=0
    # Start from the template so the file lists every supported key in the
    # documented order; config_set then fills the values in place.
    cp "$SKNANO_ROOT/config/config.default" "$CONFIG_FILE"
fi
chmod 600 "$CONFIG_FILE" 2>/dev/null

# ---------------------------------------------------------------------------
# Platform
# ---------------------------------------------------------------------------

case "$(uname -s)" in
    Linux)  OS_TAG="linux" ;;
    Darwin) OS_TAG="osx" ;;
    *) echo "Unsupported operating system: $(uname -s)" >&2; exit 1 ;;
esac
case "$(uname -m)" in
    x86_64|amd64) ARCH_TAG="64" ;;
    aarch64|arm64) ARCH_TAG="aarch64"; [[ $OS_TAG == osx ]] && ARCH_TAG="arm64" ;;
    *) echo "Unsupported CPU architecture: $(uname -m)" >&2; exit 1 ;;
esac
MAMBA_PLATFORM="${OS_TAG}-${ARCH_TAG}"

printf '%s\n' "$C_BOLD$C_GREEN"
cat <<'BANNER'
  ####################################################################
                    SKNanoAnalyzer setup wizard
  ####################################################################
BANNER
printf '%s\n' "$C_RESET"
info "repository : $SKNANO_ROOT"
info "user       : $USER"
info "platform   : $MAMBA_PLATFORM"
info "config     : $CONFIG_FILE"
if [[ $ASSUME_YES -eq 1 ]]; then
    info "mode       : --yes (every question takes its default)"
else
    info "mode       : interactive (press Enter to accept the value in [ ])"
fi
info "guide      : docs/SetupGuide.md"

# ---------------------------------------------------------------------------
# 1. micromamba
# ---------------------------------------------------------------------------

step "micromamba"

detect_micromamba() {
    local c
    for c in "$(config_get MAMBA_EXE)" "${MAMBA_EXE:-}" "$(command -v micromamba 2>/dev/null)" \
             "/data6/Users/$USER/micromamba_bin/micromamba" \
             "$HOME/micromamba_bin/micromamba" \
             "$HOME/.local/bin/micromamba" \
             "$HOME/micromamba/bin/micromamba"; do
        [[ -n $c && -x $c ]] && { printf '%s\n' "$c"; return 0; }
    done
    return 1
}

# Sets INSTALLED_MICROMAMBA on success. It reports progress on stdout, so it
# returns the path through a variable rather than through a command
# substitution.
INSTALLED_MICROMAMBA=""
install_micromamba() {
    local target_dir=$1 url tmp
    url="https://micro.mamba.pm/api/micromamba/${MAMBA_PLATFORM}/latest"
    mkdir -p "$target_dir" || return 1
    tmp=$(mktemp -d) || return 1
    info "downloading $url"
    # The published install.sh is interactive and edits the shell profile, so
    # unpack the release tarball directly instead.
    if ! curl -fsSL "$url" | tar -xj -C "$tmp" bin/micromamba; then
        rm -rf "$tmp"
        return 1
    fi
    mv "$tmp/bin/micromamba" "$target_dir/micromamba" || { rm -rf "$tmp"; return 1; }
    chmod +x "$target_dir/micromamba"
    rm -rf "$tmp"
    INSTALLED_MICROMAMBA="$target_dir/micromamba"
}

MAMBA_BIN=""
if detected=$(detect_micromamba); then
    ask_path MAMBA_BIN "Path to the micromamba executable" "$detected"
else
    info "micromamba was not found on this machine."
    if ask_yn "Install micromamba now?" y; then
        if dir_is_usable "/data6/Users/$USER"; then
            default_bin_dir="/data6/Users/$USER/micromamba_bin"
        else
            default_bin_dir="$HOME/micromamba_bin"
        fi
        ask_path bin_dir "Directory to install the micromamba executable into" "$default_bin_dir"
        if install_micromamba "$bin_dir"; then
            MAMBA_BIN="$INSTALLED_MICROMAMBA"
            ok "installed $MAMBA_BIN"
        else
            fail "micromamba installation failed (no network, or the directory is not writable)."
            fail "Install it manually, then re-run ./bootstrap.sh."
            exit 1
        fi
    else
        fail "micromamba is required. Aborting."
        exit 1
    fi
fi

if [[ ! -x $MAMBA_BIN ]]; then
    fail "$MAMBA_BIN is not an executable file."
    exit 1
fi
[[ -n ${detected:-} && $MAMBA_BIN == "$detected" ]] && skip "$MAMBA_BIN"
config_set MAMBA_EXE "$MAMBA_BIN"
export MAMBA_EXE="$MAMBA_BIN"

# ---------------------------------------------------------------------------
# 2. environment root (MAMBA_ROOT_PREFIX)
# ---------------------------------------------------------------------------

step "Environment root (MAMBA_ROOT_PREFIX)"
info "This is where the package environments themselves are stored; it needs"
info "several GB. On a cluster the worker nodes usually cannot see your home"
info "directory, so put it on shared storage instead."

default_root=$(config_get MAMBA_ROOT_PREFIX)
if [[ -z $default_root ]]; then
    if [[ -n ${MAMBA_ROOT_PREFIX:-} && -d ${MAMBA_ROOT_PREFIX:-} ]]; then
        default_root="$MAMBA_ROOT_PREFIX"
    elif [[ -d "/data6/Users/$USER/micromamba/envs" ]]; then
        default_root="/data6/Users/$USER/micromamba"
    elif [[ -d "/data6/Users/$USER/micromamba_envs" ]] || dir_is_usable "/data6/Users/$USER"; then
        default_root="/data6/Users/$USER/micromamba_envs"
    else
        default_root="$HOME/micromamba"
    fi
fi

ask_path MAMBA_ROOT "Environment root directory" "$default_root"
mkdir -p "$MAMBA_ROOT" 2>/dev/null
if [[ ! -d $MAMBA_ROOT || ! -w $MAMBA_ROOT ]]; then
    fail "$MAMBA_ROOT is not a writable directory."
    exit 1
fi
case "$MAMBA_ROOT" in
    "$HOME"|"$HOME"/*)
        warn "the environment root is under your home directory; on a cluster whose worker nodes do not mount /home, batch jobs will not find it." ;;
esac
config_set MAMBA_ROOT_PREFIX "$MAMBA_ROOT"
export MAMBA_ROOT_PREFIX="$MAMBA_ROOT"

# ---------------------------------------------------------------------------
# 3. the Nano environment
# ---------------------------------------------------------------------------

step "Nano environment"

nano_env_exists() {
    [[ -x "$MAMBA_ROOT/envs/Nano/bin/python" || -d "$MAMBA_ROOT/envs/Nano/conda-meta" ]]
}

if nano_env_exists; then
    skip "$MAMBA_ROOT/envs/Nano"
else
    info "The Nano environment holds ROOT, correctionlib, ONNX Runtime and the"
    info "compiler toolchain. They are one ABI-compatible set, so it is created"
    info "from the pinned lock file rather than solved."
    if ask_yn "Create the Nano environment now? (roughly 20-30 minutes)" y; then
        created=0
        lock_file="docs/Nano-${MAMBA_PLATFORM}.lock"
        if [[ -f $lock_file ]]; then
            info "micromamba create -n Nano -f $lock_file"
            if "$MAMBA_EXE" create -y -n Nano -f "$lock_file"; then
                created=1
            else
                warn "the lock file did not apply; falling back to the solver input."
            fi
        else
            info "no lock file for $MAMBA_PLATFORM; solving docs/Nano.yml instead."
        fi
        if [[ $created -eq 0 ]]; then
            info "micromamba env create -n Nano -f docs/Nano.yml"
            "$MAMBA_EXE" env create -y -n Nano -f docs/Nano.yml && created=1
        fi
        if [[ $created -eq 1 ]] && nano_env_exists; then
            ok "created $MAMBA_ROOT/envs/Nano"
        else
            fail "the Nano environment could not be created."
            fail "Create it by hand and re-run ./bootstrap.sh:"
            if [[ -f $lock_file ]]; then
                fail "  $MAMBA_EXE create -n Nano -f $lock_file"
            else
                fail "  $MAMBA_EXE env create -n Nano -f docs/Nano.yml"
            fi
            exit 1
        fi
    else
        warn "skipping; setup.sh will fail until an environment named Nano exists."
    fi
fi

# ---------------------------------------------------------------------------
# 4-5. output and log directories
# ---------------------------------------------------------------------------

pick_data_dir() {
    # Prefer shared storage that batch jobs can also see.
    local leaf=$1 c
    for c in "/gv0/Users/$USER/$leaf" "/data6/Users/$USER/$leaf"; do
        dir_is_usable "$c" && { printf '%s\n' "$c"; return 0; }
    done
    printf '%s\n' "$HOME/$leaf"
}

make_writable_dir() {
    local d=$1
    mkdir -p "$d" 2>/dev/null || return 1
    local probe="$d/.sknano_write_test.$$"
    : > "$probe" 2>/dev/null || return 1
    rm -f "$probe"
    return 0
}

step "Output directory (SKNANO_OUTPUT)"
info "Analysis results are written here. It grows to hundreds of GB."
default_out=$(config_get SKNANO_OUTPUT)
[[ -z $default_out ]] && default_out=$(pick_data_dir SKNanoOutput)
ask_path SKNANO_OUTPUT_VALUE "Output directory" "$default_out"
if make_writable_dir "$SKNANO_OUTPUT_VALUE"; then
    ok "$SKNANO_OUTPUT_VALUE is writable"
else
    fail "$SKNANO_OUTPUT_VALUE cannot be created or written to."
    exit 1
fi
config_set SKNANO_OUTPUT "$SKNANO_OUTPUT_VALUE"

step "Log directory (SKNANO_RUNLOG)"
info "Condor submission directories, DAG files and job logs are written here."
default_log=$(config_get SKNANO_RUNLOG)
[[ -z $default_log ]] && default_log=$(pick_data_dir SKNanoRunlog)
ask_path SKNANO_RUNLOG_VALUE "Log directory" "$default_log"
if make_writable_dir "$SKNANO_RUNLOG_VALUE"; then
    ok "$SKNANO_RUNLOG_VALUE is writable"
else
    fail "$SKNANO_RUNLOG_VALUE cannot be created or written to."
    exit 1
fi
config_set SKNANO_RUNLOG "$SKNANO_RUNLOG_VALUE"

# ---------------------------------------------------------------------------
# 6. input root
# ---------------------------------------------------------------------------

step "Input root (SKNANO_INPUT_ROOT)"
info "The top of the NanoAOD production. Sample metadata stores only the part"
info "below this, so moving to another site is one value rather than a rewrite"
info "of every sample json."
default_input=$(config_get SKNANO_INPUT_ROOT)
[[ -z $default_input ]] && default_input="/gv0/DATA/SKNano/NanoAODv15_RNTuple"
ask_path SKNANO_INPUT_VALUE "Input root" "$default_input"
if [[ -d $SKNANO_INPUT_VALUE ]]; then
    ok "$SKNANO_INPUT_VALUE exists"
else
    warn "$SKNANO_INPUT_VALUE does not exist. Building and running analyzers on local files still works; change [SKNANO_INPUT_ROOT] in $CONFIG_FILE later."
fi
config_set SKNANO_INPUT_ROOT "$SKNANO_INPUT_VALUE"

# ---------------------------------------------------------------------------
# 7. batch execution
# ---------------------------------------------------------------------------

step "Batch execution"
info "Condor jobs run inside an Apptainer/Singularity image that symlinks the"
info "Nano environment, so the environment stays outside the image."
info "  (a) use an existing .sif image"
info "  (b) build one now from templates/Nano.def"
info "  (c) no image -- run locally only"

existing_image=$(config_get SINGULARITY_IMAGE)
container_cmd=""
for c in apptainer singularity; do
    command -v "$c" >/dev/null 2>&1 && { container_cmd=$c; break; }
done

if [[ -n $existing_image && -f $existing_image ]]; then
    skip "image $existing_image"
    BATCH_CHOICE=a
    IMAGE_PATH=$existing_image
else
    default_choice=b
    [[ -n $existing_image ]] && default_choice=a
    [[ -z $container_cmd ]] && default_choice=c
    if [[ -z $container_cmd ]]; then
        warn "neither apptainer nor singularity is installed here, so option (b) cannot run. Choose (a) if an image already exists, otherwise (c)."
    fi
    while true; do
        ask BATCH_CHOICE "Batch execution: (a) existing image, (b) build now, (c) none" "$default_choice"
        BATCH_CHOICE=${BATCH_CHOICE,,}
        case "$BATCH_CHOICE" in
            a|b|c) break ;;
            *) fail "answer a, b or c."; [[ $ASSUME_YES -eq 1 ]] && { BATCH_CHOICE=c; break; } ;;
        esac
    done
    IMAGE_PATH=""
    case "$BATCH_CHOICE" in
        a)
            ask_path IMAGE_PATH "Path to the .sif image" "$existing_image"
            [[ -n $IMAGE_PATH && ! -f $IMAGE_PATH ]] && warn "$IMAGE_PATH does not exist yet."
            ;;
        b)
            if [[ -z $container_cmd ]]; then
                fail "no apptainer/singularity command; falling back to (c)."
                IMAGE_PATH=""
            else
                default_sif="$(dirname "$SKNANO_OUTPUT_VALUE")/Image/Nano.sif"
                ask_path IMAGE_PATH "Where should the image be written?" "$default_sif"
                mkdir -p "$(dirname "$IMAGE_PATH")"
                def_out="$(dirname "$IMAGE_PATH")/Nano.def"
                sed "s|\[NANO_ENV_PATH\]|$MAMBA_ROOT|g" templates/Nano.def > "$def_out"
                info "definition written to $def_out"
                info "$container_cmd build --fakeroot $IMAGE_PATH $def_out"
                if "$container_cmd" build --fakeroot "$IMAGE_PATH" "$def_out"; then
                    ok "built $IMAGE_PATH"
                else
                    fail "the image build failed. --fakeroot needs subuid/subgid entries"
                    fail "for $USER, or a --remote / sudo build. Continuing without an image;"
                    fail "set [SINGULARITY_IMAGE] in $CONFIG_FILE once you have one."
                    IMAGE_PATH=""
                fi
            fi
            ;;
        c)
            info "no image; jobs will use MAMBA_ROOT_PREFIX directly."
            IMAGE_PATH=""
            ;;
    esac
fi
config_set SINGULARITY_IMAGE "$IMAGE_PATH"

# ---------------------------------------------------------------------------
# 8. Telegram
# ---------------------------------------------------------------------------

step "Telegram notifications (optional)"
info "Leave both empty to disable. Create a bot with @BotFather, then read the"
info "chat id from https://api.telegram.org/bot<TOKEN>/getUpdates."
ask_path TELEGRAM_TOKEN "Telegram bot token (empty = off)" "$(config_get TOKEN_TELEGRAMBOT)"
if [[ -n $TELEGRAM_TOKEN ]]; then
    ask_path TELEGRAM_CHATID "Telegram chat id" "$(config_get USER_CHATID)"
else
    TELEGRAM_CHATID=""
fi
config_set TOKEN_TELEGRAMBOT "$TELEGRAM_TOKEN"
config_set USER_CHATID "$TELEGRAM_CHATID"
if [[ -n $TELEGRAM_TOKEN && -n $TELEGRAM_CHATID ]]; then
    ok "Telegram reporting enabled"
else
    info "Telegram reporting disabled"
fi

config_set PACKAGE mamba
chmod 600 "$CONFIG_FILE" 2>/dev/null

# ---------------------------------------------------------------------------
# 9. submodules
# ---------------------------------------------------------------------------

step "Submodules"

# A cloned-but-unchecked-out submodule still has a .git file in it, so test for
# real content rather than for a non-empty directory.
roccor_ok()  { [[ -f external/RoccoR/RoccoR.cc && -f external/RoccoR/RoccoR.h ]]; }
jsonpog_ok() { [[ -d external/jsonpog-integration/POG ]]; }

# An aborted "git submodule update" leaves a submodule with the right HEAD and
# an empty index, so every later update call considers it up to date and writes
# nothing. Restore the checkout, but only in that exact state -- never over a
# working tree the user may have edited. Untracked files (the RoccoR build
# files setup.sh copies in) survive a hard reset.
repair_submodule() {
    local path=$1
    [[ -e "$path/.git" ]] || return 1
    [[ -z "$(git -C "$path" ls-files 2>/dev/null)" ]] || return 1
    info "repairing the empty checkout in $path"
    git -C "$path" reset --hard HEAD >/dev/null 2>&1
}

update_submodules() {
    # One submodule per invocation. "git submodule update --init --recursive"
    # aborts the whole run on the first repository the user cannot read, which
    # leaves the ones it had already cloned without a checkout.
    local path failed=()
    git submodule init >/dev/null 2>&1
    while read -r path; do
        [[ -n $path ]] || continue
        info "updating $path"
        if git submodule update --init --recursive -- "$path" >/dev/null 2>&1; then
            repair_submodule "$path"
        else
            failed+=("$path")
        fi
    done < <(git config -f .gitmodules --get-regexp '^submodule\..*\.path$' \
             | awk '{print $2}')
    if [[ ${#failed[@]} -gt 0 ]]; then
        info "could not update: ${failed[*]}"
    fi
}

if ! command -v git >/dev/null 2>&1 || [[ ! -d .git ]]; then
    warn "not a git checkout; skipping submodules."
elif roccor_ok; then
    skip "external/RoccoR is checked out"
else
    if ask_yn "Initialise git submodules now?" y; then
        update_submodules
        if roccor_ok; then
            ok "external/RoccoR is checked out"
        else
            fail "external/RoccoR has no source files; the build needs it."
            fail "It lives on gitlab.cern.ch and is cloned over ssh, so you need an"
            fail "ssh key registered there. See docs/SetupGuide.md, section"
            fail "\"Before you start\". After adding the key, run:"
            fail "  git submodule update --init external/RoccoR"
        fi
        if ! jsonpog_ok; then
            warn "external/jsonpog-integration is not checked out. Not fatal: the CMS_corrections symlink into /cvmfs supplies the same corrections."
        fi
        info "analysis-group modules (*_Analyzers) are private repositories;"
        info "failing to clone one is expected unless you work on that analysis."
    else
        warn "submodules skipped; external/RoccoR must be checked out before the build."
    fi
fi

# ---------------------------------------------------------------------------
# 10. environment check (no questions)
# ---------------------------------------------------------------------------

step "Environment check"

if [[ -d /cvmfs/cms-griddata.cern.ch/cat/metadata ]]; then
    ok "cvmfs corrections are mounted"
else
    warn "/cvmfs/cms-griddata.cern.ch is not mounted. Corrections are then read from external/jsonpog-integration, which must be checked out."
fi

if [[ -d /gv0 ]]; then
    ok "/gv0 is mounted"
else
    info "/gv0 is not mounted (expected outside the SNU cluster)"
fi

for tool in git curl tar unzip; do
    command -v "$tool" >/dev/null 2>&1 || warn "'$tool' is not installed; setup steps that use it will fail."
done
if ! command -v wget >/dev/null 2>&1; then
    warn "'wget' is not installed; scripts/install_libtorch.sh needs it."
fi

if command -v cc >/dev/null 2>&1 || command -v gcc >/dev/null 2>&1; then
    ok "host compiler: $( { gcc --version 2>/dev/null || cc --version; } | head -1)"
else
    info "no host compiler on PATH; the Nano environment provides its own."
fi

for d in "$SKNANO_ROOT" "$SKNANO_OUTPUT_VALUE" "$MAMBA_ROOT"; do
    avail=$(df -BG --output=avail "$d" 2>/dev/null | tail -1 | tr -dc '0-9')
    if [[ -n $avail ]]; then
        if [[ $avail -lt 20 ]]; then
            warn "only ${avail}G free on $d"
        else
            ok "${avail}G free on $d"
        fi
    fi
done

# ---------------------------------------------------------------------------
# 11. build
# ---------------------------------------------------------------------------

step "Build"
info "The first build also installs LHAPDF from source and downloads LibTorch,"
info "so it takes considerably longer than later incremental builds."
BUILD_RAN=0
BUILD_OK=0
if ask_yn "Build the project now?" y; then
    BUILD_RAN=1
    # Build this checkout, not whichever one the shell was last set up for:
    # setup.sh keeps an inherited SKNANO_ANALYSIS_MODULE_DIRS as a deliberate
    # override, and a stale one points the build at another repository.
    if env -u SKNANO_ANALYSIS_MODULE_DIRS -u SKNANO_HOME -u SKNANO_BUILDDIR \
           -u SKNANO_INSTALLDIR bash -c 'source setup.sh && ./scripts/build.sh'; then
        BUILD_OK=1
        ok "build finished"
    else
        fail "the build failed. Fix the error above, then run by hand:"
        fail "  source setup.sh && ./scripts/build.sh"
    fi
else
    info "skipped."
fi

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------

printf '\n%s' "$C_BOLD$C_GREEN"
cat <<'BANNER'
  ####################################################################
                              All done
  ####################################################################
BANNER
printf '%s\n' "$C_RESET"

info "configuration : $CONFIG_FILE"
[[ $CONFIG_EXISTED -eq 1 ]] && info "                (updated in place; existing values were kept as defaults)"
info "micromamba    : $MAMBA_EXE"
info "env root      : $MAMBA_ROOT"
info "output        : $SKNANO_OUTPUT_VALUE"
info "logs          : $SKNANO_RUNLOG_VALUE"
info "input root    : $SKNANO_INPUT_VALUE"
info "batch image   : ${IMAGE_PATH:-(none, local running only)}"

printf '\n'
info "In every new shell:"
printf '        %scd %s && source setup.sh%s\n' "$C_BOLD" "$SKNANO_ROOT" "$C_RESET"
printf '\n'
info "Smoke test (ten jobs over one sample):"
printf '        %sSKNano.py -a ExampleRun -i DYto2E_MLL50to120 -e 2024 -n 10%s\n' "$C_BOLD" "$C_RESET"
printf '\n'
if [[ $BUILD_RAN -eq 1 && $BUILD_OK -eq 1 ]]; then
    info "Unit tests:"
    printf '        %sctest --test-dir "$SKNANO_BUILDDIR" --output-on-failure%s\n' "$C_BOLD" "$C_RESET"
    printf '\n'
fi

if [[ ${#WARNINGS[@]} -gt 0 ]]; then
    printf '      %s%d warning(s):%s\n' "$C_YELLOW" "${#WARNINGS[@]}" "$C_RESET"
    for w in "${WARNINGS[@]}"; do
        printf '        - %s\n' "$w" | fold -s -w 72 | sed '2,$s/^/          /'
    done
    printf '\n'
fi
info "Question-by-question explanations: docs/SetupGuide.md"

if [[ $BUILD_RAN -eq 1 && $BUILD_OK -eq 0 ]]; then
    exit 1
fi
exit 0
