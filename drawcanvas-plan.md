# DrawCanvas — Drawing-as-a-Plugin Plan

Ziel: die Canvas-Zeichenlogik in ein eigenes, sauber wiederverwendbares Beispiel
extrahieren — ein generisches Zeichen-Widget plus eine Zeichenfunktion als „Plug-in“,
die als Callback an das Widget übergeben wird.

## 1. Ist-Zustand

- `wbuild_widgets/Canvas.widget` ist `@class Canvas(Core)` und bereits ein
  Zeichen-Host: `@PUBLIC <Callback> callback`, `@PRIVATE canvas_draw_t canv`,
  `expose → paint1 → XtCallCallbackList($, $callback, &$canv)`.
- `canvas-draw.h` definiert den Kontext `canvas_draw_t` (GCs, XftDraw/XftFont/XftColor,
  dpy, screen, win/world-Dimensionen, priv_data).
- `canvas-draw-cb.c` ist das de-facto-Plugin: `xim2_init(top)` → `RCB(top, canvas_draw_cb)`,
  `canvas_draw_cb` → `draw_svg()` (nanosvg). Alt-Verkabelung: `*sc.canvas.callback: canvas_draw_cb`.
- Problem: Zeichnen (`draw_svg`) ist mit Infra (`compress_redraw`, `modif_*`, nanosvg)
  vermischt, und `commander_runner.c` ruft `xim2_init` gar nicht auf.

## 2. Idee

- **Generisches Widget** (Kopie von `Canvas.widget`) → Host mit `callback` + `canvas_draw_t`-Kontext.
- **Plug-in** (reines C) → die eigentliche Zeichenfunktion, als Callback registriert.
- Plug-in tauschen = Zeichnung tauschen, ohne Widget-Änderung.

## 3. Datei-Layout

```
wbuild_widgets/DrawCanvas.widget    # Kopie von Canvas.widget, Klasse umbenannt
wbuild_widgets/drawcanvas-draw.h    # Kopie von canvas-draw.h (Kontext canvas_draw_t)
wbuild_widgets/drawcanvas-draw.c    # Plug-in: Zeichenfunktion + init()
demos/demo_drawcanvas.lua           # LUI-Demo
```

Build-Integration analog zu `canvas-draw-cb.c`/`canvas-draw.h`:
- Root-`makefile` `libxtcw`-Target: `cp wbuild_widgets/drawcanvas-draw.{c,h} build/source/`.
- `wbuild_widgets/makefile` globbt `*.widget` automatisch; `register-widgets.sh`
  registriert `DrawCanvas` automatisch (`WcRegisterClassPtr`) und ergänzt
  `lui/registry_generated.lua`.

## 4. Design-Entscheidungen

1. **Symbolkollision**: einziger echter Konflikt ist der exportierte `canvas_get_priv`.
   Export im neuen Widget umbenennen zu `drawcanvas_get_priv`. `canvas_draw_t` ist nur
   ein Typedef (kein Linkersymbol), kann bleiben.
2. **Verkabelung**: `callback`-Ressource ist der Plug-in-Hook. Plug-in-C liefert
   `drawcanvas_draw_cb(Widget,XtPointer,XtPointer)` (call_data = `canvas_draw_t*`) und
   `drawcanvas_plugin_init(Widget top)` (→ `RCB(top, drawcanvas_draw_cb)`).
   `commander_runner.c` ruft `drawcanvas_plugin_init(TopLevel)`; LUI setzt
   `(DrawCanvas :id "c" :callback "drawcanvas_draw_cb")` (bare Name → Wcl → RCB).
3. **Widget minimal halten**: `zoom`-Action, `topx/topy/zoom_thousandth`, SVG-spezifisches
   entfernen. Behalten: `callback`, `foreground`, `xftFont`, `canvas_draw_t canv`,
   `expose→callback`, `realize` (Fenster + XftDraw + 2 GCs), `destroy`, `query_geometry`,
   `set_values`.
4. **Plug-in selbstcontained** (ohne nanosvg): Hintergrund füllen + ein paar Linien/
   Rechtecke + Text via `XftDrawStringUtf8`. Zeigt, dass beliebiges Zeichnen einsteckbar ist.

## 5. Schritt-für-Schritt

1. `DrawCanvas.widget` erstellen (Kopie, Klasse `DrawCanvas`, Export `drawcanvas_get_priv`, getrimmt).
2. `drawcanvas-draw.h` (Kopie von canvas-draw.h, nur `canvas_draw_t` + Plug-in-Deklarationen).
3. `drawcanvas-draw.c` (Zeichenfunktion + `drawcanvas_plugin_init`).
4. Root-`makefile` `libxtcw`: Kopierzeilen für `drawcanvas-draw.{c,h}`.
5. `commander_runner.c`: `drawcanvas_plugin_init(TopLevel)` aufrufen.
6. `demos/demo_drawcanvas.lua`.
7. `make widgets libxtcw commander`.
8. Lauf + Screenshot.

## 6. Verifikation

- `run_demo_tests.sh` pickt `demo_drawcanvas.lua` automatisch auf (Fenster + Render).
- Explizit prüfen, dass `registry_generated.lua` `"DrawCanvas"` enthält und der Callback
  feuert (z.B. Flag beim ersten Draw).

## 7. Alternativen / Trade-offs

- **Simpler (kein neues Widget)**: existierendes `Canvas` wiederverwenden, nur Plug-in-C +
  Demo + `xim2_init`-Aufruf. Kein wbuild-Aufwand; dafür keine eigenständige Kopie.
- **Selbstcontained-Example-Dir** (`examples/drawcanvas/`): saubere Eigentümerschaft, aber
  doppeltes wbuild-Aufrufen und Umgehen von `register-widgets.sh`.
- **Function-Pointer-Ressource statt Callback**: expliziter „Plug-in“-Zugriff, aber
  Xt-Function-Pointer-Ressourcen sind fummelig und verlieren Wcl-Namensauflösung.

## 8. Risiken / offene Punkte

- Der `canvas_draw_t`-Kontext ist roh X11/GC/Xft → Zeichnen muss in C bleiben (kein
  Lua-Plug-in).
- Redraw nur bei Xt-Expose; zustandsabhängiges Neuzeichnen braucht `redraw`/Timer-Helfer.
- `canvas-draw.h`-Deklarationen (`canvas_draw_cb`, `canvas_zoom`, `modif_*`) in der Kopie
  entfernen, da das neue Plug-in sie nicht implementiert.

## Status

- [x] DrawCanvas.widget
- [x] drawcanvas-draw.h
- [x] drawcanvas-draw.c
- [x] Root-makefile libxtcw-Kopierzeilen
- [x] commander_runner.c Plugin-Init
- [x] demos/demo_drawcanvas.lua
- [x] Build + Lauf + Test

## Ergebnis / Abweichungen vom Plan

- Umgesetzt wie geplant. **Abweichung bei der Benennung:** die kopierte
  `canvas-draw.h` wurde zu `drawcanvas-draw.h` mit **umbenanntem Typ**
  `draw_canvas_t` (statt `canvas_draw_t`). Grund: `register_wb.h` inkludiert
  `Canvas.h` *und* `DrawCanvas.h` in derselben Übersetzungseinheit; eine
  identische Redefinition von `struct canvas_draw_st` führt zu einem
  Compile-Fehler. Die Umbenennung macht das Beispiel vollständig isoliert und
  konfliktfrei.
- Dateien:
  - `wbuild_widgets/DrawCanvas.widget` (Klasse `DrawCanvas`, Export
    `drawcanvas_get_priv`, ohne `zoom`/`topx`/`topy`/`zoom_thousandth`)
  - `wbuild_widgets/drawcanvas-draw.h` (`draw_canvas_t` + Plug-in-Deklarationen)
  - `wbuild_widgets/drawcanvas-draw.c` (`drawcanvas_draw_cb` + `drawcanvas_plugin_init`)
  - `demos/demo_drawcanvas.lua`
  - Root-`makefile` (2 `cp`-Zeilen), `commander_runner.c` (`drawcanvas_plugin_init(TopLevel)`)
- Build: `make widgets libxtcw commander` → `DrawCanvas` wird automatisch
  generiert, in `libxtcw.a` kompiliert, via `register-widgets.sh` registriert und
  in `lui/registry_generated.lua` eingetragen.
- Lauf: Fenster mit `DrawCanvas` (480×323) rendert das Plug-in (Kreuz, Rechteck,
  Marker, Text) fehlerfrei.
- Test: `commander/run_demo_tests.sh` → **39 passed, 0 failed** (inkl.
  `demo_drawcanvas`).

