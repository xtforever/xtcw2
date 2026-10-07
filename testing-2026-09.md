# Testing Plan — September 2026

Ziel: das LUI/`commander_runner`-System reproduzierbar und automatisiert verifizieren,
sodass jede Änderung sicher überprüfbar ist.

## Ist-Zustand (funktioniert bereits)

- `commander_runner` baut und läuft (headless unter Xvfb bewiesen).
- LUI-Skripte laden, Widgets rendern korrekt.
- Makros funktionieren (`defmacro`, `macros.register`, `luiecho`).
- Callbacks: `LUA(name)` → Lua-Global, bare Name → C-Callback (RCB), sofortige Ausführung.
- Event-Loop via `XtAppMainLoop`.

## Gefundene Lücken / Inkonsistenzen

1. **Task-Bindings tot**: `register_task_lua()` ist deklariert, wird aber in `main()` nie
   aufgerufen → `task_copy`, `task_spawn`, `task_control`, `ls`, `wheel_exec_command`
   sind in Lua nicht verfügbar (nur C-seitig `copy_task`).
2. **Inline-Funktions-Callbacks kaputt**: `:callback my_function` (als Funktionswert)
   erzeugt `lua_cb_XXXXXX` in `gui.handlers`; `run_lua_chunk` führt aber
   `lua_cb_XXXXXX()` als *Global* aus → schlägt fehl. Nur `LUA(name)` mit globaler
   Funktion funktioniert.
3. **Zwei konkurrierende Dispatch-Mechanismen**: C-Seite nutzt jetzt sofortige
   Ausführung; `gui.modal_loop` (und damit `gui.alert/confirm/prompt`) nutzt weiterhin
   `luaxt_processevent` + `pullcallback`-Stack → ungetestet, vermutlich inkonsistent.
4. **Verwaister Stack**: `luaxt_pushcallback/pullcallback` wird nach dem Umbau auf
   `XtAppMainLoop` nirgends mehr gedraint (nur noch `lua_task_bindings.c` pusht).
5. **`(style "file")`** ruft `gui.load_css`, das es nicht gibt.
6. **`(require …)`** Plugin-Loading ist Stub.
7. **`_G.class_data`** / zweites `data`-Argument wird in `commander_runner` nicht gesetzt
   (nur `gui.loop` tut das).

## Der Plan

### Phase 1 — Automatisierte Verifikation („dass es funktioniert“)

Reproduzierbarer Test statt Handarbeit.

- **Headless-Test-Harness** (bereits validiert): Xvfb + `xwininfo` + `xdotool` + `scrot`
  + Pixel-Analyse (PIL/numpy).
- **Smoke-Test-Skript**, das für jedes Demo:
  1. `commander_runner -Luafile …` unter Xvfb startet,
  2. prüft: kein Fehler im Log, erwartete Widgets im `xwininfo`-Tree,
  3. Screenshot macht und nicht-leer prüft,
  4. per `xdotool` klickt und die Callback-Reaktion (Pixel-Diff) verifiziert.
- Kriterium „funktioniert“ = alle Demos grün im Skript.

### Phase 2 — Bekannte Bugs beheben (konsolidieren)

- **Eine Dispatch-Quelle**: `run_lua_chunk` zum einzigen Weg machen; `gui.loop`/
  `gui.modal_loop` entweder auf den C-Weg umbiegen oder explizit als „nicht unterstützt
  in commander_runner“ dokumentieren/deaktivieren.
- **Inline-Funktions-Callbacks**: entweder unterstützen (Handler globalisieren oder
  Lookup `handlers[name] or _G[name]` im C-Dispatch nachziehen) oder klar dokumentieren,
  dass nur `LUA(name)`/Globals gehen.
- **Task-Bindings**: `register_task_lua()` in `main()` aufrufen (und Task-Event-Drain an
  `XtAppMainLoop` anpassen, da der Stack weg ist) — oder Dead-Code entfernen.
- **Verwaisten push/pull-Stack** entfernen.
- **`gui.load_css`** implementieren oder `(style …)` als nicht unterstützt markieren.

### Phase 3 — Robustheit & DX

- Besseres Fehler-Reporting (Lua-Fehler mit Datei/Zeile, einheitlicher Pfad statt nur
  `stderr`).
- `class_data`/`data`-Argument konsistent durchreichen.
- Docs (`COMMANDER_RUNNER_TUTORIAL.md`, `lui/README.md`) mit dem Code synchron halten.

### Phase 4 — Features

- Task-UI-Integration (Fortschrittsbalken via `task_set_handler` → Lua-Callback).
- Weitere Widget-Demos (List, Splitter, Spinbox, …) als Abdeckung für den Smoke-Test.

## Empfehlung für den nächsten konkreten Schritt

Mit **Phase 1** anfangen: ein Smoke-Test-Skript schreiben, das die vorhandenen Demos
automatisch verifiziert. Das liefert sofort einen „grün/rot“-Indikator und macht alle
Folgeänderungen (Phase 2) sicher überprüfbar.

---

## Status

- [x] Plan erstellt
- [x] Phase 1: Smoke-Test-Harness — `commander/run_demo_tests.sh`
- [x] Phase 2: Bekannte Bugs (teilweise — siehe unten)
- [ ] Phase 2-Rest: Modal-Dialoge + verwaister Stack konsolidieren
- [ ] Phase 3: Robustheit & DX
- [ ] Phase 4: Features

### Phase 1 — Ergebnis

`commander/run_demo_tests.sh` startet unter Xvfb und prüft pro Demo:
kein Crash, keine Lua/Bootstrap/ASan-Fehler im Log, Fenster erzeugt (`xwininfo`),
nicht-leerer Render (`scrot` + PIL), plus ein funktionaler Wbutton-Test
(Makro `grid_opts`, Widget-Erzeugung, Callback via `xtaction notify`, Label-Roundtrip).

Ausführen: `cd commander && ./run_demo_tests.sh`

Ergebnis: **30 passed, 0 failed** (8 Demos × 3 Checks + Build + 5 funktionale Checks).

### Phase 2 — Ergebnis (abgeschlossen)

1. **Inline-Funktions-Callbacks** — behoben. `backend_xt.process_property` legt eine
   Funktions-Callback zusätzlich als `_G[lua_cb_XXXXXX]` ab, sodass der sofortige
   Chunk-Dispatch von `commander_runner` (`funcname()`) sie findet. Getestet.
2. **Task-Bindings** — aktiviert. `register_task_lua(L_GLOBAL)` wird in `main()`
   aufgerufen; `internal_event_cb` führt das Task-Event jetzt sofort aus (statt den
   verwaisten push-Stack zu füllen). Duplikate (`xtgetvalue`/`xtsetvalue`/`mls_*`, die
   bereits `lua_bridge_register_bindings` registriert) entfernt. `task_copy` end-to-end
   getestet (Job-ID, COMPLETE-Event, Dateiinhalt).
3. **`gui.load_css`** — als No-op mit Warnung ergänzt, damit `(style …)` nicht mehr
   crasht.

Neue funktionale Tests im Harness: Inline-Callback (2 Checks) und task_copy (4 Checks).
Gesamtergebnis: **36 passed, 0 failed**.

### Phase 2 — Rest (offen)

- **Modal-Dialoge** (`gui.alert/confirm/prompt` via `modal_loop`): nutzen noch
  `luaxt_processevent` + `pullcallback`. Mit der sofortigen Ausführung funktioniert das
  Setzen von `running=false` zwar, ist aber ungetestet/re-entrant-fragil. Entweder auf
  den C-Weg umbiegen oder als „nicht unterstützt in commander_runner“ dokumentieren.
- **Verwaister push/pull-Stack** in `luaxt.c`: wird nur noch von `gui.loop`/`modal_loop`
  (Lua/SWIG) benutzt. Entfernen erfordert Änderungen am SWIG-Interface — zurückgestellt.
- **`(require …)`** Plugin-Loading bleibt Stub.
- **`_G.class_data`** / zweites `data`-Argument wird weiterhin nicht durchgereicht.

### Bekannte Nebenbefunde

- LeakSanitizer meldet beim sauberen Exit Speicherlecks (Xt-Interna, MLS-Listen,
  `register_widget`), da die Cleanup-Pfade unvollständig sind. Phase 3.
- Ein hängengebliebener `Xvfb :99` blockiert den Harness-Start (`FATAL: could not start
  Xvfb`). Harness ggf. um Auto-Aufräumen erweitern.
