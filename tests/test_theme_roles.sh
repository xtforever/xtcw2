#!/bin/bash
# Verify that every shipped theme defines the widget surface roles and metrics
# used by the improved widget set. A missing key falls back to the old roles,
# but live theme switching then loses the intended per-surface styling.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PASS=0
FAIL=0
ok()  { echo "ok - $1"; PASS=$((PASS + 1)); }
bad() { echo "FAIL - $1"; FAIL=$((FAIL + 1)); }

KEYS=(
    menu_bg menu_fg menu_sel menu_border
    toolbar_bg toolbar_fg toolbar_border
    status_bg status_fg status_border
    tooltip_bg tooltip_fg tooltip_border
    table_bg table_fg table_sel_bg table_sel_fg
    table_header_bg table_header_fg table_alt_bg table_line
    tree_bg tree_fg tree_sel_bg tree_line
    progress_bg progress_fill progress_border progress_fg
    dialog_bg dialog_border
)

METRICS=(
    menu_height toolbar_height status_height dialog_padding
    table_row_height table_header_height tree_indent tree_row_height
    progress_height
)

check_file() {
    local file="$1"
    local themes
    themes=$(grep -cE '^\(theme ' "$file")
    if [ "$themes" -lt 1 ]; then
        bad "$file: no themes found"
        return
    fi
    local key count
    for key in "${KEYS[@]}" "${METRICS[@]}"; do
        count=$(grep -cE "^[[:space:]]+$key[[:space:]]+" "$file")
        if [ "$count" -lt "$themes" ]; then
            bad "$file: $key in $count/$themes themes"
        fi
    done
    ok "$file: $themes themes carry the surface roles"
}

check_file "$ROOT/themes.hdf"
check_file "$ROOT/cookbook/themes/cookbook.hdf"

if [ "$FAIL" -eq 0 ]; then
    echo "test_theme_roles PASSED ($PASS checks)"
    exit 0
fi
echo "test_theme_roles FAILED ($FAIL failures, $PASS checks)"
exit 1
