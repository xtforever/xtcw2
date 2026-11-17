#!/bin/bash
# Keep the LUI registry, the widget sources and the generated registry in sync.
#
# The roadmap docs claim that widgets are "implemented"; this check makes the
# cheap part of that claim verifiable: every W* class exposed through the LUI
# registry must have a matching wbuild_widgets/*.widget source, and every
# wbuild widget must be listed in lui/registry_generated.lua.
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PASS=0
FAIL=0
ok()  { echo "ok - $1"; PASS=$((PASS + 1)); }
bad() { echo "FAIL - $1"; FAIL=$((FAIL + 1)); }

# 1. Every W* class in the LUI registry has a widget source.
while IFS= read -r class; do
    [ -n "$class" ] || continue
    if [ -f "$ROOT/wbuild_widgets/$class.widget" ]; then
        continue
    fi
    bad "registry class $class has no wbuild_widgets/$class.widget"
done < <(grep -oE "class = '[^']+'" "$ROOT/lui/registry.lua" |
         sed "s/class = '//;s/'//" | grep -E '^W[A-Z]' | sort -u)

# 2. Every wbuild W* widget is listed in the generated registry.
while IFS= read -r path; do
    class="$(basename "$path" .widget)"
    if grep -q "\"$class\"" "$ROOT/lui/registry_generated.lua"; then
        continue
    fi
    bad "wbuild_widgets/$class.widget is missing from lui/registry_generated.lua"
done < <(find "$ROOT/wbuild_widgets" -maxdepth 1 -name 'W*.widget' | sort)

# 3. Roadmap inventory must not claim "no table/tree/model abstraction" any more.
if grep -q "no table/tree/model abstraction" "$ROOT/Improved-Widget-Set.md"; then
    bad "Improved-Widget-Set.md still claims there is no table/tree abstraction"
else
    ok "roadmap inventory is current"
fi

if [ "$FAIL" -eq 0 ]; then
    echo "test_docs_consistency PASSED ($PASS checks)"
    exit 0
fi
echo "test_docs_consistency FAILED ($FAIL failures, $PASS checks)"
exit 1
