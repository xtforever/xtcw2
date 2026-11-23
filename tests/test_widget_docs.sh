#!/bin/bash
# Verify that wbuild generates TeX, man-page and HDF documentation from the
# resource/method comments in our widgets.
#
# The widgets are regenerated into a temporary directory so the test does not
# touch build/doc, and so it really exercises doc generation instead of only
# inspecting previously generated files.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/widget_docs_test.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

PASS=0
FAIL=0
ok()  { echo "ok - $1"; PASS=$((PASS + 1)); }
bad() { echo "FAIL - $1"; FAIL=$((FAIL + 1)); }

WBUILD="$ROOT/build/bin/wbuild"
WBWRAP="$ROOT/build/bin/wb.sh"
TEXW="$ROOT/build/etc/xtcw/tex.w"
NROFFW="$ROOT/build/etc/xtcw/nroff.w"

if [ ! -x "$WBUILD" ] || [ ! -x "$WBWRAP" ] ||
   [ ! -f "$TEXW" ] || [ ! -f "$NROFFW" ]; then
    echo "SKIP - wbuild tools not built (run make widgets first)"
    exit 0
fi

mkdir -p "$TMP/doc" "$TMP/doc/hdf" "$TMP/src" "$TMP/include"
# wbuild reads the superclass headers; give it a private copy so the test does
# not rewrite build/include.
cp "$ROOT"/build/include/xtcw/*.h "$TMP/include/" 2>/dev/null || true

WIDGETS="WAudioChassis WModulePanel EqFader Wedit WLineEdit"

for w in $WIDGETS; do
    log="$TMP/$w.log"
    if ( cd "$ROOT/wbuild_widgets" &&
         wb_search=./ "$WBWRAP" "$w.widget" "$WBUILD" \
             -d "$TMP/doc" -c "$TMP/src" -i "$TMP/include" \
             --include-prefix=xtcw --hdf-dir="$TMP/doc/hdf" \
             "$TEXW" "$NROFFW" ) >"$log" 2>&1; then
        ok "$w: wbuild doc generation"
    else
        bad "$w: wbuild doc generation"
        tail -20 "$log" | sed 's/^/    /'
    fi
done

check_contains() {
    local file="$1" pattern="$2" label="$3"
    if [ ! -f "$file" ]; then
        bad "$label (missing $file)"
    elif grep -qF -- "$pattern" "$file"; then
        ok "$label"
    else
        bad "$label (pattern not found: $pattern)"
    fi
}

# Resource documentation (public table + per-resource section) and method
# documentation must both be present in the generated TeX.
check_contains "$TMP/doc/WAudioChassis.tex" 'XtNchassisWidth' \
    "WAudioChassis: resource table entry"
check_contains "$TMP/doc/WAudioChassis.tex" 'Realize the chassis at its fixed size' \
    "WAudioChassis: method documentation"
check_contains "$TMP/doc/WModulePanel.tex" 'Paint the gradient panel' \
    "WModulePanel: utility documentation"
check_contains "$TMP/doc/WModulePanel.tex" 'panel fill colour' \
    "WModulePanel: resource documentation"
check_contains "$TMP/doc/EqFader.tex" 'Mouse wheel up: raise the gain' \
    "EqFader: action documentation"
check_contains "$TMP/doc/EqFader.tex" 'Override the Wheel theme hook' \
    "EqFader: multi-line method comment stays one paragraph"
check_contains "$TMP/doc/Wedit.tex" 'font specification for the editor text' \
    "Wedit: public resource documentation"
check_contains "$TMP/doc/WLineEdit.tex" 'Clear the field and notify' \
    "WLineEdit: action documentation"

# The nroff backend must produce a man page with the resource table as well.
check_contains "$TMP/doc/WAudioChassis.3" 'XtNchassisWidth' \
    "WAudioChassis: man-page resource entry"

# HDF backend: structured anchors, superclass chain, owner/inherited data.
check_contains "$TMP/doc/hdf/WAudioChassis.hdf" 'kind widget' \
    "WAudioChassis: HDF widget node"
check_contains "$TMP/doc/hdf/WAudioChassis.hdf" 'anchor widget/WAudioChassis' \
    "WAudioChassis: HDF widget anchor"
check_contains "$TMP/doc/hdf/WAudioChassis.hdf" 'superclass Board' \
    "WAudioChassis: HDF superclass"
check_contains "$TMP/doc/hdf/WAudioChassis.hdf" 'superclass_chain (seq' \
    "WAudioChassis: HDF superclass chain"
check_contains "$TMP/doc/hdf/WAudioChassis.hdf" 'kind method' \
    "WAudioChassis: HDF method node"
check_contains "$TMP/doc/hdf/WAudioChassis.hdf" 'inherited true' \
    "WAudioChassis: HDF inherited resource flag"
check_contains "$TMP/doc/hdf/EqFader.hdf" 'description_links (seq' \
    "EqFader: HDF inline description links"
check_contains "$TMP/doc/hdf/EqFader.hdf" 'target resource/EqFader/XtNvalue' \
    "EqFader: HDF resource link target"

# The widget reference browser parses the generated HDF material and
# renders it as Markdown.  Its self-test does not need a display.
if command -v lua5.3 >/dev/null 2>&1; then
    if lua5.3 "$ROOT/tests/test_hdf_parser.lua" >"$TMP/hdf_parser.log" 2>&1; then
        ok "hdf parser: core syntax, seq, rem, raw strings"
    else
        bad "hdf parser: unit test failed"
        tail -20 "$TMP/hdf_parser.log" | sed 's/^/    /'
    fi
    if lua5.3 "$ROOT/tools/widget_help.lua" --self-test >"$TMP/widget_help.log" 2>&1; then
        ok "widget_help: generated HDF parsed and rendered"
    else
        bad "widget_help: self-test failed"
        tail -20 "$TMP/widget_help.log" | sed 's/^/    /'
    fi
else
    echo "skip - widget_help self-test (no lua5.3)"
fi

echo
echo "widget docs: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
