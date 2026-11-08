#!/bin/bash
# Clean-build helper used by ci/Dockerfile.
#
# Clones (or, if a checkout already exists, pulls) xtcw2 and runs a clean
# "make all".  Exits non-zero on any build error and prints the tail of the
# build log.
#
# Environment:
#   REPO_URL  git URL or path to a (bare) repository. Default /src/xtcw2.git
#             (meant to be bind-mounted read-only).
#   REPO_REF  branch/tag/commit to build. Default: master
#   KEEP      if "1", reuse /work/repo and `git pull` instead of a fresh clone.
set -euo pipefail

REPO_URL="${REPO_URL:-/src/xtcw2.git}"
REPO_REF="${REPO_REF:-master}"
KEEP="${KEEP:-0}"
WORK=/work/repo
LOG=/tmp/build.log

echo "### clean-build: repo=${REPO_URL} ref=${REPO_REF} keep=${KEEP}"

# A bind-mounted repository is owned by a different uid than the container
# user; allow git to read it (ephemeral build container).
git config --global --add safe.directory '*' 2>/dev/null || true

if [ "$KEEP" = "1" ] && [ -d "$WORK/.git" ]; then
    echo "### pulling into existing checkout"
    git -C "$WORK" fetch origin "$REPO_REF"
    git -C "$WORK" checkout -q FETCH_HEAD
    git -C "$WORK" reset -q --hard FETCH_HEAD
else
    rm -rf "$WORK"
    # Local paths need file:// for a shallow (--depth) clone.
    case "$REPO_URL" in
        /*|./*|../*) CLONE_URL="file://$REPO_URL" ;;
        *)           CLONE_URL="$REPO_URL" ;;
    esac
    git clone --depth=1 --branch "$REPO_REF" "$CLONE_URL" "$WORK"
fi
cd "$WORK"

echo "### HEAD: $(git log --oneline -1)"
echo "### gcc: $(gcc --version | head -1)"
echo "### swig: $(swig -version 2>/dev/null | sed -n '2p')"
echo "### submodules: $(git submodule status 2>/dev/null | grep -c . || true) (expect 0; nanosvg is vendored)"

echo "### build: make all"
set +e
make all 2>&1 | tee "$LOG"
rc=${PIPESTATUS[0]}
set -e

if [ "$rc" -ne 0 ]; then
    echo "### BUILD FAILED (exit $rc) — last 80 lines:"
    tail -80 "$LOG"
    exit "$rc"
fi

echo "### BUILD OK"

if [ "${RUN_TESTS:-0}" = "1" ]; then
    echo "### tests: xvfb-run -a ./tests/run_all_tests.sh"
    set +e
    xvfb-run -a ./tests/run_all_tests.sh 2>&1 | tee /tmp/tests.log
    trc=${PIPESTATUS[0]}
    set -e
    if [ "$trc" -ne 0 ]; then
        echo "### TESTS FAILED (exit $trc) — last 100 lines:"
        tail -100 /tmp/tests.log
        exit "$trc"
    fi
    echo "### TESTS OK"
fi
