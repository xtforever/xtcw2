# Widget Test Automation Plan

## Overview
Automate testing of XTCW widgets using TRACE(50,...) output with trace_level=50 and XtCallActionProc to simulate user interactions.

## Key Mechanisms

### 1. TRACE Output (trace_level=50)
- TRACE macro defined in `utils/mls.h`
- Format: `TRACE(level, "format", ...)`
- Level 50 = widget function tracing (highest detail)
- Widgets output: `TRACE(50, "<widget-name> <function> <args>")`
- Output goes to stderr via `deb_trace()`

### 2. Simulating Clicks (XTACTION)
- Use `XtCallActionProc(widget, action_name, event, params, num_params)`
- Common actions:
  - `"activate"` - button press simulation
  - `"set"` / `"reset"` - toggle widgets
  - `"notify"` - trigger callbacks
  - `"focus_in"` / `"focus_out"` - focus changes
  - `"SetKeyboardFocus"` - keyboard focus

### 3. Scheduling Tests (XtAppAddTimeout)
- `XtAppAddTimeout(app_context, ms, callback, data)`
- Schedules test functions after Xt main loop starts
- Allows UI to initialize before testing begins

### 4. LUI Parser Integration
- Parser: `lui/parser.lua` - LPeg-based S-expression parser
- Backend: `lui/backend_xt.lua` - Creates widgets via `xtcreate()`
- GUI: `lui/gui_xt.lua` - Event loop and callbacks

## Implementation Plan

### Phase 1: TRACE Enhancement
```c
// Add to all widget action procedures
TRACE(50, "%s %s args=%s", XtName(w), __FUNCTION__, args_str);
```

### Phase 2: Test Harness
```lua
-- test_harness.lua
local test = {}
local trace_log = {}
local expected = {}

function test.start_trace_capture()
    -- Redirect stderr to capture TRACE output
    -- Parse lines matching: "widget_name function_name args"
end

function test.expect(widget, action, args)
    table.insert(expected, {widget=widget, action=action, args=args})
end

function test.click(widget_id)
    local w = backend.get_widget(widget_id)
    if w then
        xtcallaction(w, "activate", nil, nil, 0)
    end
end

function test.verify()
    -- Compare trace_log against expected
    -- Return pass/fail with mismatches
end

return test
```

### Phase 3: Lua Test API
```lua
-- Example test script
local lui_test = require('lui_test')

-- Define UI
local ui = [[
(window :id "test_win" :title "Button Test"
  (grid
    (button :id "btn1" :label "Click Me" :callback "test_click")
    (label :id "result" :label "Not clicked")
  ))
]]

-- Define test sequence
local function run_tests()
    -- Expect initial state
    test.expect("btn1", "initialize", "")
    
    -- Simulate click
    test.click("btn1")
    test.expect("btn1", "activate", "")
    test.expect("btn1", "callback", "test_click")
    
    -- Verify all expectations met
    local pass, errors = test.verify()
    if not pass then
        print("FAIL:", table.concat(errors, "\n"))
        xtappexitflag()
    else
        print("PASS")
        xtappexitflag()
    end
end

-- Run UI with test harness
lui_test.run(ui, run_tests)
```

### Phase 4: C Integration
```c
// Add to luarunner.c or new test_runner.c

static void test_timeout_callback(XtPointer client_data, XtIntervalId *id)
{
    lua_State *L = (lua_State*)client_data;
    
    // Call Lua test function
    lua_getglobal(L, "run_scheduled_tests");
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        fprintf(stderr, "Test error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

// Schedule tests after UI initialization
void schedule_tests(lua_State *L)
{
    XtAppAddTimeout(LUAXT_APP, 500, test_timeout_callback, L);
}
```

## Widget Coverage Matrix

| Widget | Actions Testable | Status |
|--------|-----------------|--------|
| button | activate, highlight, reset | needs TRACE(50) |
| toggle | set, reset, notify | needs TRACE(50) |
| edit | focus_in, focus_out, activate | needs TRACE(50) |
| label | (passive, no actions) | N/A |
| list | notify, scroll | needs TRACE(50) |
| grid | (container) | N/A |
| slider | notify, set | needs TRACE(50) |

## Missing Features Detected

1. **No TRACE(50) in widgets** - Currently max TRACE level is 8
   - Need to add TRACE(50,...) to all widget action procs
   - Standardize format: `widget_name function args`

2. **No xtcallaction Lua binding** - Need to expose XtCallActionProc
   ```c
   // Add to luarunner.c
   static int lua_xtcallaction(lua_State *L) {
       Widget w = luaarg_to_widget(L, 1);
       const char *action = lua_tostring(L, 2);
       XtCallActionProc(w, action, NULL, NULL, 0);
       return 0;
   }
   ```

3. **No test scheduler** - Need XtAppAddTimeout wrapper
   ```c
   static int lua_xtapptimeout(lua_State *L) {
       int ms = lua_tointeger(L, 1);
       XtAppAddTimeout(LUAXT_APP, ms, test_timeout_callback, L);
       return 0;
   }
   ```

4. **No trace capture from Lua** - Can't read TRACE output
   - Option A: Write TRACE to file, Lua reads file
   - Option B: Add callback mechanism for TRACE
   - Option C: Use socket/pipe for trace streaming

## LUI Advanced GUI Verification

### Current Capabilities
- S-expression syntax for widget trees
- Property mapping (`:id`, `:label`, `:callback`)
- Container nesting (grid, box)
- Callback registration via `LUA(name)` strings

### Test Cases for Advanced GUI
```lua
-- Multi-level nesting
local complex_ui = [[
(window :title "Complex"
  (grid :id "outer"
    (grid :id "inner1"
      (button :id "b1")
      (button :id "b2"))
    (grid :id "inner2"
      (edit :id "e1")
      (label :id "l1"))))
]]

-- Dynamic widget manipulation
function test_dynamic()
    -- Create widget after initial build
    local new_btn = xtcreate("newbtn", "command", "outer", "label", "New")
    
    -- Modify properties
    gui.set("l1", "label", "Updated")
    
    -- Verify via TRACE
    test.expect("l1", "set_values", "label=Updated")
end
```

### Missing LUI Features
1. **No conditional rendering** - Can't do if/then/else in LUI
2. **No loops for lists** - Must manually write each list item
3. **No computed properties** - Can't bind expressions to properties
4. **Limited error reporting** - Parser errors show position but not context
5. **No widget references in callbacks** - Can't pass widget ID to callback easily

## Error Reporting Precision

### Current State
- Parser: Shows line/col and context (20 chars before/after)
- Runtime: Lua errors show traceback
- Widget errors: Often silent or generic "Widget not found"

### Improvements Needed
1. Widget path tracking - show full path in errors
2. Callback validation - verify handler exists before registering
3. Property type checking - validate resource types
4. Action validation - warn if action doesn't exist on widget

## Implementation Steps

1. Add TRACE(50) to all widget action procs
2. Create xtcallaction Lua binding
3. Create xtapptimeout Lua binding
4. Build test_harness.lua module
5. Write example tests for each widget
6. Document expected TRACE formats
7. Create CI integration script

## Example Complete Test

```lua
-- button_test.lua
local test = require('test_harness')
local lui = require('lui')

-- Test configuration
test.set_trace_level(50)

-- Build UI
lui.run([[ (window (button :id "btn" :label "Test")) ]])

-- Define test
test.sequence({
    { action = "capture_start" },
    { action = "click", widget = "btn" },
    { action = "expect", widget = "btn", event = "activate" },
    { action = "expect", widget = "btn", event = "notify" },
    { action = "verify" }
})

-- Run and exit
local result = test.run()
os.exit(result and 0 or 1)
```
