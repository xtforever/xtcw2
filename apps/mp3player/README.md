# mp3player — Audio-Player-Demo

> **Status: Implementierung.** Player, DSP-Engine und Playlist-Bibliothek
> sind lauffähig; offen sind Feinschliff und der vollständige Gesamttest.

## 0. Aktueller Stand

Umgesetzt:

- `apps/mp3player/script.lua` — minimale Kopie des AdmPanel-pty-Wrappers
- `apps/mp3player/c/mp3fft.c` + `c/Makefile` — C-FFT als Lua-5.3-Modul
- `wbuild_widgets/WAudioChassis.widget` — feste 1160×770-Shell mit
  prozeduralem Metall-Hintergrund, Schrauben und pixelgenauen Children
- `wbuild_widgets/WXftLabel.widget` — minimales Xft-Label ohne retex,
  `ParentRelative`-Fenster für echte Transparenz über der Chassis-Textur
- `apps/mp3player/player.lua` — mpg123-`-R`-Controller mit State-Machine
- `apps/mp3player/id3.lua` — ID3v1/v2.2/v2.3/v2.4-Parser
- `apps/mp3player/m3u.lua` — M3U lesen/schreiben
- `apps/mp3player/library.lua` — Legacy-Scan/Fallback
- `apps/mp3player/playlist_library.lua` — Playlist-Bibliothek, Alias, Auto-M3U
- `apps/mp3player/library_ui.lua` — eigenständige Bibliotheks-GUI
- `apps/mp3player/analysis.lua` — PCM-Cache über `ffmpeg` + FFT-Glue (mpg123-Fallback)
- `apps/mp3player/player_dsp.lua` — Lua-Fassade für die DSP-Engine
- `apps/mp3player/c/mp3engine.c` — libmpg123 + 5-Band-EQ + libpulse-simple + FFT
- `wbuild_widgets/WVfdDisplay.widget` — VFD mit 7-Segment-Zeiten und Glow
- `wbuild_widgets/WSpectrum.widget` — FFT-Balken mit dB-Raster
- `wbuild_widgets/WVuMeter.widget` — segmentierte L/R-Pegelanzeige
- `wbuild_widgets/WSeekBar.widget` — SeekBar mit Drag/Click und Zeit-Labels
- `wbuild_widgets/WTransportButton.widget` — flache Icon-Buttons mit Caption
- `wbuild_widgets/WModulePanel.widget` — rundes, dunkles Modulgehäuse mit Schrauben
- `apps/mp3player/main.lua` — Chassis, VFD, Playlist-Bibliothek,
  Transport- und Album-Buttons, WSeekBar, 5-Band-EQ, Volume-Fader,
  Live-Spektrum, VU-Meter, LIBRARY-Button
- `apps/mp3player/run_library.sh` — eigenständiger Bibliotheks-Start
- `apps/mp3player/theme.hdf` — dunkles App-Theme
- `tests/test_mp3_fft.lua`, `tests/test_mp3_id3.lua`,
  `tests/test_mp3_m3u.lua`, `tests/test_mp3_library.lua`,
  `tests/test_mp3_analysis.lua`, `tests/test_mp3_chassis.c`,
  `tests/test_mp3_xftlabel.c`, `tests/test_mp3_widgets.c`,
  `tests/test_mp3engine.sh`, `tests/test_mp3engine_module.sh`,
  `tests/test_playlist_library.lua`, `tests/test_library_ui.sh`,
  `tests/test_mp3_player_wrapper.sh`, `tests/test_mp3player.sh`

Screenshots: `widget_photo/mp3player_playing.png`,
`widget_photo/mp3player_spectrum.png`, `widget_photo/mp3player_vu.png`,
`widget_photo/mp3player_seekbar.png`, `widget_photo/mp3player_vfd.png`,
`widget_photo/mp3player_transport.png`,
`widget_photo/mp3player_module_panel.png`,
`widget_photo/mp3player_library_ui.png`,
`widget_photo/mp3player_library_app.png`,
`widget_photo/mp3player_library_persist.png`.

Noch offen: letzter Design-Feinschliff, vollständiger Gesamttest und
Performance-/Langzeittests.

## 1. Ziel

`apps/mp3player` ist eine eigenständige LUI-Test-App, die die grafischen und
interaktiven Möglichkeiten des XTCW-Widget-Sets demonstriert:

- 5-Band-Graphic-EQ (N-Band konfigurierbar)
- echte FFT-Spektrum-Anzeige
- Play/Pause, Stop, Prev/Next Track, Prev/Next Album
- Seek-/Progressbar mit Zeit-Anzeige und Drag/Click
- ID3v1-/ID3v2-Tags
- M3U-Playlists lesen und automatisch erzeugen
- rekursives Musikverzeichnis mit Unterverzeichnissen
- Sortierung nach M3U-Reihenfolge oder ID3-Tags
- Konfiguration als Lua-Code
- feste, pixelgenau aufgebaute Player-Oberfläche im Stil von
  `mp3-player-design.jpg`

**Nicht in v1:** eigener Audio-DSP-Engine, Netzwerk-Streaming, Cover-Art,
Remote-Hosts.

## 2. Design-Referenz

Optik:

- dunkles, leicht gebürstetes Metallgehäuse
- große VFD-Anzeige in Blau-Grün/Cyan mit Glow
- segmentierte VU-Meter links/rechts (Grün → Gelb → Rot)
- orange Spektrum-Balken mit dB-Raster
- flache, eckige Transport-Buttons, keine modernen runden Buttons
- alle sechs Transport-Buttons exakt `84×72`; Labels werden auf zwei Zeilen
  umgebrochen (z. B. `PREV` / `ALBUM`), SeekBar rechts daneben, ebenfalls
  `72 px` hoch
- Volume-Fader rechts in einem gerundeten `WModulePanel`
- EQ-Fader mit dB-Skala und 0-dB-Linie
- Schrauben/Bevel an den Gehäuseecken

Referenzbild: `mp3-player-design.jpg`
Funktionsreferenz: `mp3-player.txt`

**Bandanzahl:** Default sind **5 Bänder** gemäß Anforderung:
`60 Hz, 250 Hz, 1 kHz, 4 kHz, 12 kHz`.
Das Referenzbild zeigt 6, `mp3-player.txt` nennt 7. Die UI wird aus
`config.eq_bands` generiert; 6/7 Bänder sind reine Konfigurationsvarianten.

## 3. Architektur

```
config.lua (Lua-Table)
      │
      ▼
library.lua ── scan (rekursiv) ── id3.lua (v1/v2) ── m3u.lua
      │
      ▼
Queue/Playlist (Tracks, Album-Gruppen, Sortierung)
      │
      ├──► player.lua ── mpg123 -R (script_start/script_write)
      │        Events: @P, @F, @I, @T, @S
      │        Kommandos: L, P, S, J, V, E, T
      │
      └──► analysis.lua ── ffmpeg → PCM-Cache
               │
               ▼
           mp3fft.so (C-FFT) → Spektrum-Bänder + RMS/Peak
               │
               ▼
      UI-Timer (xtapptimeout, ~25 fps)
               │
               ▼
   WAudioChassis
     ├── WVfdDisplay
     ├── WVuMeter (L/R)
     ├── WSeekBar
     ├── EqFader (5×)
     └── WSpectrum
```

## 4. `WAudioChassis` — feste Shell und Hintergrund

`WAudioChassis` ist das zentrale Widget der App. Es ist eine
**`Board`-Subklasse** und wird als einziges managed Child des Root-Shell
verwendet. Damit verhält es sich „quasi als Shell“ für den Player, ohne die
Risiken einer echten `TopLevelShell`-Subklasse.

### Eigenschaften

- feste Größe: Default **1160 × 770 px**
- **kein Resize zur Laufzeit**
- beliebig viele Child-Widgets
- pixelgenaue Positionierung: Kinder setzen `:x`, `:y`, `:width`, `:height`
  (Core-Geometrie). `location`/`abs_*` stehen zusätzlich für Board-Kinder
  zur Verfügung.
- Hintergrund wird einmalig in eine Pixmap gerendert
- optionales Hintergrundbild (PNG/SVG), sonst prozeduraler Metall-Look

### Ressourcen

| Ressource | Typ | Default | Bedeutung |
|---|---|---|---|
| `title` | String | "XTCW MP3 Player" | Fenstertitel des Root-Shell |
| `chassisWidth` | int | 1160 | feste Breite |
| `chassisHeight` | int | 770 | feste Höhe |
| `backgroundImage` | String | "" | optionales PNG/SVG |
| `metalColor` | Pixel | #3a3d40 | Metall-Grundfarbe |
| `metalHi` | Pixel | #5a5e62 | Highlight |
| `metalLo` | Pixel | #1c1e20 | Schatten |
| `screwColor` | Pixel | #8a8d90 | Schrauben |
| `frameWidth` | int | 0 | kein Frame-Rahmen |
| `outerOffset` | int | 0 | kein zusätzlicher Offset |
| `innerOffset` | int | 0 | kein zusätzlicher Offset |

### Feste Größe

- `query_geometry` liefert immer die feste Größe.
- `set_values_almost` erzwingt die feste Größe.
- `realize` setzt auf dem Shell-Parent:
  - `XtNminWidth` = `XtNmaxWidth` = `chassisWidth`
  - `XtNminHeight` = `XtNmaxHeight` = `chassisHeight`
  - `XtNwidth` / `XtNheight` = feste Größe
- Ein programmatisches `XtSetValues(width, height)` oder ein WM-Resize
  ändert die tatsächliche Größe nicht.

### Hintergrund

Standardmäßig prozedural mit Cairo:

- vertikaler Metall-Gradient
- feine horizontale Bürstlinien
- Vignette
- umlaufendes Bevel
- vier Schrauben in den Ecken

Der Hintergrund wird in `realize` einmal in eine Pixmap gerendert; `expose`
blittet nur noch. Das hält die Oberfläche flackerfrei und schnell.

Optional kann `backgroundImage` ein PNG (Cairo) oder SVG (XImage2) laden.
Das Design-JPG wird nicht direkt eingebettet, weil es die komplette UI
enthält. Für einen exakten Gehäuse-Hintergrund wird ein reines
Hintergrund-PNG als Asset abgelegt.

### Koordinaten (Startwerte)

| Panel | `x y width height` |
|---|---|
| `WVfdDisplay` | `36 36 1088 178` |
| VU-Modul LEFT | `36 218 508 96` |
| VU-Modul RIGHT | `576 218 488 96` |
| Volume-Modul | `1072 218 80 196` |
| Volume-Fader (relativ) | `8 8 64 180` |
| Transport-Gridbox | `24 320 660 92` |
| Transport-Buttons | `44 330 … 84×72` |
| Seek-Modul | `596 324 472 84` |
| `WSeekBar` (relativ) | `8 6 456 72` |
| EQ-Modul | `36 448 708 282` |
| `WSpectrum` | `820 448 300 282` |

Die Werte werden im Polish-Schritt anhand von Screenshots feinjustiert.

### LUI-Einbindung

```lisp
(window :id "win" :title "XTCW MP3 Player" :width 1160 :height 770
  (WAudioChassis :id "chassis" :chassisWidth 1160 :chassisHeight 770
    (WVfdDisplay :id "vfd" :x 36 :y 36 :width 1088 :height 178)
    (WModulePanel :id "vupanel_l" :x 36 :y 218 :width 508 :height 96
  (WVuMeter :id "vu_l" :label "LEFT" :x 18 :y 16 :width 472 :height 64 :transparent 1))
    (WModulePanel :id "vupanel_r" :x 576 :y 218 :width 488 :height 96
  (WVuMeter :id "vu_r" :label "RIGHT" :x 18 :y 16 :width 452 :height 64 :transparent 1))
    (WSeekBar :id "seek" :x 44 :y 386 :width 700 :height 52)
    (WSpectrum :id "spec" :x 820 :y 448 :width 300 :height 282)
    ;; weitere Panels …
  ))
```

`WAudioChassis` ist kein `topLevelShellWidgetClass`; LUI ersetzt es daher
nicht durch eine `Gridbox`, sondern hängt es direkt als einziges Child an
den Root-Shell.

## 5. C-FFT: `mp3fft.so`

Die FFT wird **nicht** in Lua berechnet. Stattdessen gibt es ein kleines
Lua-C-Modul:

```
apps/mp3player/c/mp3fft.c
apps/mp3player/c/Makefile
apps/mp3player/c/mp3fft.so   (Build-Artefakt)
```

### Warum C?

- 1024-Punkt-FFT bei 25 fps ist in C praktisch kostenlos.
- PCM-Datei lesen, Fenstern, Bit-Reversal und Butterflies bleiben in C.
- Lua bekommt nur fertige dB-Werte und RMS/Peak.
- Keine Lua-Byte-Schleifen, keine GC-Last pro Frame.

### Implementierung

- selbstständige radix-2-Cooley-Tukey-FFT in `double`
- keine FFTW-Abhängigkeit
- Hann-Fenster und Bit-Reversal-Tabelle werden einmal pro Analyzer
  vorberechnet
- `int16`-Little-Endian-Stereo-PCM wird direkt aus der Datei gelesen
- Mono-Mix für das Spektrum: `(L + R) / 2`
- RMS/Peak werden pro Kanal berechnet
- log-verteilte Bänder (Default 24) von ca. 40 Hz bis 10 kHz
- Rückgabe in dBFS, Clamp auf `min_db..max_db`

### Lua-API

```lua
local mp3fft = require("mp3fft")

local analyzer = mp3fft.new{
  rate    = 22050,
  size    = 1024,
  bands   = 24,
  min_db  = -50,
  max_db  = 0,
}

local r, err = analyzer:process_pcm_file(pcm_path, offset_bytes, 2)
if not r then
  -- z. B. Datei zu kurz, Modulfehler
end

-- r.levels : Array mit 24 dB-Werten
-- r.rms    : { left_db, right_db }
-- r.peak   : { left_db, right_db }
-- r.bytes  : tatsächlich gelesene Bytes
```

Weitere Funktionen:

- `mp3fft.version()` → String
- `analyzer:reset()` → Zustand zurücksetzen
- `analyzer:process_pcm_data(string, channels)` → für Tests ohne Datei

### Build

```bash
make -C apps/mp3player/c
```

Das Makefile nutzt:

```bash
pkg-config --cflags --libs lua5.3
```

und linkt `-lm -shared -fPIC`.

`run.sh` baut `mp3fft.so` automatisch, wenn es fehlt oder älter als die
Quellen ist.

### Laden in Lua

`main.lua` setzt:

```lua
local root = os.getenv("MP3PLAYER_ROOT")
package.path  = root .. "/apps/mp3player/?.lua;" .. package.path
package.cpath = root .. "/apps/mp3player/c/?.so;" .. package.cpath
```

### Fallback

Wenn `mp3fft.so` nicht gebaut werden kann:

- App startet trotzdem.
- Spektrum bleibt aus, Statusmeldung „SPECTRUM OFF“.
- VU-Meter können optional aus einer einfachen Peak-Berechnung in Lua
  gespeist werden oder ebenfalls ausbleiben.
- Kein stiller Fallback auf eine langsame Lua-FFT.

### Tests

- `tests/test_mp3_fft.c`:
  - Sinus bei 1 kHz → Peak im erwarteten Bin
  - Hann-Fenster und Band-Mapping
  - RMS/Peak für bekannte Amplituden
- `tests/test_mp3_fft.lua`:
  - Modul laden
  - PCM-Fixture verarbeiten
  - 24 endliche dB-Werte
  - Peak-Band nahe 1 kHz
  - fehlende Datei → `nil, err`
- Performance: 100 × 1024-Punkt-FFT deutlich unter 1 s.

## 5b. Standalone DSP-Proof: `mp3engine_test`

Bevor eine Engine in die Lua-App integriert wird, gibt es ein eigenständiges
C-Testprogramm:

```
apps/mp3player/c/mp3engine_test.c
```

Es testet den kompletten DSP-Pfad ohne LUI:

- Decoding über **libmpg123**
- 5-Band-Peaking-EQ in `float`
- Ausgabe über **libpulse-simple**
- FFT/RMS/Peak pro Block
- optional `--null` für headless Tests

Build (benötigt `libmpg123-dev` und `libpulse-dev`):

```bash
make -C apps/mp3player/c mp3engine_test
```

Beispiele:

```bash
# echte Ausgabe über den Default-Sink
./apps/mp3player/c/mp3engine_test track.mp3

# headless, nur Decode/DSP/Spektrum
./apps/mp3player/c/mp3engine_test --null --seconds 5 track.mp3

# anderer Sink und EQ
./apps/mp3player/c/mp3engine_test --sink headset_eq.sink \
    --eq 0,2,0,-1,3 track.mp3
```

Ausgabe:

```text
format: 44100 Hz, 2 ch, enc=0x200
duration: 93.99 s
spec -50 -50 ... -48
pos 0.43 / 93.99 s
```

Der Test ist **nicht** Teil des Default-Builds. `make -C apps/mp3player/c`
baut weiterhin nur `mp3fft.so`.

`tests/test_mp3engine.sh` überspringt sich selbst, wenn
`libmpg123-dev` fehlt.

## 5c. In-Process-DSP-Engine

Die App nutzt standardmäßig `player.engine = "dsp"`:

```
libmpg123 -> 5-Band-Biquad-EQ -> libpulse-simple
                    │
                    └-> FFT/RMS/Peak -> Lua-Status (25 fps)
```

Vorteile gegenüber dem mpg123-Fallback:

- kein `forkpty`/Kommando-Protokoll
- kein `ffmpeg`-PCM-Cache
- echter 5-Band-EQ statt Subband-Näherung
- Spektrum/VU aus dem Post-EQ-Signal
- sample-genaue Position und Seek

`eq_bands` aus der Lua-Config wird an die Engine als `freqs` übergeben und
bestimmt sowohl die Biquad-Frequenzen als auch die Labels der fünf
EQ-Fader. Der mpg123-Fallback bildet die Frequenzen nur genähert auf die
32 MPEG-Subbänder ab.

Lua-API (`player_dsp.lua`):

```lua
player:load(path)
player:toggle()
player:stop()
player:seek(seconds)
player:set_volume(0..100)
player:set_eq_band(1..5, db)
player:update()
```

Die Engine läuft in einem Worker-Thread; Lua liest nur einen
mutex-geschützten Status-Snapshot.

Build:

```bash
make -C apps/mp3player/c mp3engine.so
```

`run.sh` versucht den Build automatisch. Schlägt er fehl, fällt die App auf
`player.lua` (mpg123) zurück.

Konfiguration:

```lua
player = {
  engine = "dsp",              -- "dsp" | "mpg123"
  sink = "",                   -- "" = Default-Sink
}
```

## 6. ID3, M3U und Bibliothek

### ID3v2

Unterstützt v2.2, v2.3 und v2.4:

- `TIT2`/`TT2` → Titel
- `TPE1`/`TP1` → Artist
- `TALB`/`TAL` → Album
- `TRCK`/`TRK` → Track
- `TYER`/`TDRC`/`TYE` → Jahr
- `TCON`/`TCO` → Genre
- `TLEN` → Dauer (falls vorhanden)

Berücksichtigt:

- Syncsafe-Größen
- Extended Header
- Unsynchronisation
- UTF-8, UTF-16 mit BOM, UTF-16BE, Latin-1 → UTF-8
- fehlerhafte/abgeschnittene Tags → Fallback, kein Crash

### ID3v1

- letzte 128 Bytes
- `TAG`-Header
- v1.1-Track-Byte
- Latin-1 → UTF-8

### Fallback

- Titel = Dateiname ohne Endung
- Album = Elternverzeichnis
- Artist = „Unknown Artist“

### M3U

- `#EXTM3U`
- `#EXTINF:<sekunden>,<titel>`
- Kommentare und Leerzeilen
- UTF-8-BOM
- relative Pfade relativ zur M3U-Datei
- absolute Pfade
- fehlende Dateien werden übersprungen und gemeldet

### Playlist-Bibliothek

Die Bibliothek ist jetzt playlist-orientiert:

- **Album = M3U-Playlist**
- **Track = Eintrag innerhalb der M3U**
- Alias = Anzeigename des Albums
- Die M3U-Reihenfolge ist verbindlich
- ID3-Tags dienen nur der Anzeige und der automatischen Playlist-Erzeugung
- Eine M3U darf Tracks aus beliebigen Verzeichnissen/Alben enthalten

Ein Bibliothekseintrag:

```lua
{
  alias = "Attero Dominatus",
  playlist = "/pfad/album.m3u",
  source_dir = "/pfad/musik",
  generated = false,
  group_key = "",
  track_count = 10,
  missing = false,
  last_scan = 1234567890,
}
```

Gespeichert wird die Bibliothek getrennt von `config.lua`:

```
~/.local/share/xtcw-mp3player/library.lua
```

Schreiben erfolgt atomar über eine Temp-Datei + `os.rename`.

Ist die Bibliothek noch leer, importiert `playlist_library.ensure(cfg)` beim
ersten App-Start automatisch `cfg.music_dir`, erzeugt die nötigen M3Us und
schreibt `library.lua` dauerhaft. Weitere Starts laden die bestehende
Bibliothek ohne erneuten Import.

### Verzeichnis hinzufügen

`playlist_library.add_directory(cfg, dir)`:

1. Verzeichnis kanonisieren
2. rekursiv alle `*.m3u`/`*.m3u8` finden
3. jede M3U als Album aufnehmen
   - Alias-Default: ID3-Album, wenn alle Tracks dasselbe Album haben
   - bei gemischten Playlists: M3U-Dateiname ohne Endung
   - Alias bleibt bei späteren Scans erhalten und ist im GUI editierbar
4. alle von M3Us referenzierten MP3s sammeln
5. verbleibende, nicht referenzierte MP3s finden
6. diese nach `albumartist + album` gruppieren
   - Fallback: Verzeichnisname
7. pro Gruppe eine M3U erzeugen
8. Duplikate über den kanonisierten Playlist-Pfad vermeiden
9. Warnungen für fehlende/kaputte Dateien sammeln

Wichtig: Auch ein Verzeichnis **mit** M3U kann noch unreferenzierte MP3s
enthalten. Deshalb wird mit der Menge der bereits referenzierten Dateien
gearbeitet, nicht nur mit „M3U vorhanden? ja/nein“.

### Generierte M3Us

Automatisch erzeugte Playlists liegen im verwalteten Ordner:

```
~/.local/share/xtcw-mp3player/playlists/
```

- Dateiname aus dem Alias, sanitisiert
- Kollisionen mit Suffix ` (2)`, ` (3)`, …
- absolute Pfade in der M3U
- `generated = true`, `source_dir` und `group_key` werden gespeichert
- Rescan aktualisiert vorhandene generierte Playlists statt Duplikate anzulegen

Externe M3Us werden nie verändert oder gelöscht.

### Rescan und Entfernen

`rescan`:

- liest alle vorhandenen Playlists neu
- aktualisiert Track-Anzahl und Missing-Status
- entdeckt neue M3Us in den bekannten Quellverzeichnissen
- regeneriert automatische Playlists
- behält Aliase bei

`remove`:

- entfernt den Bibliothekseintrag
- löscht nur generierte M3Us im verwalteten Ordner
- externe M3Us bleiben unangetastet

### Sortierung und Album-Navigation

- Tracks werden **exakt in M3U-Reihenfolge** geladen.
- Track-Titel kommen bevorzugt aus den ID3-Tags; `#EXTINF`/Dateiname dient
  nur als Fallback.
- Keine ID3-Umsortierung innerhalb eines Albums.
- `PREV/NEXT ALBUM` springt an Playlist-Grenzen.
- Gemischte M3Us funktionieren dadurch automatisch.
- Der Legacy-Modus ohne Bibliothekseinträge fällt weiterhin auf
  `library.lua` mit einem `music_dir` zurück.

### Standalone-Bibliotheks-GUI

Eigenständig startbar:

```bash
./apps/mp3player/run_library.sh
./apps/mp3player/run_library.sh /pfad/zur/config.lua
```

GUI:

- Album-Liste: Alias, Tracks, Quelle, Typ (`m3u`/`generated`)
- Track-Liste: `#`, Titel, Artist, Album, Pfad
- Buttons:
  - `ADD DIRECTORY`
  - `RESCAN`
  - `RENAME`
  - `REMOVE`
  - `CLOSE`
- Pfad- und Alias-Eingabe über `WLineEdit`
- Alben/Tracks über `WListView`
- läuft ohne Audio-Engine und ohne Player-Prozess

Integration im Player:

- kleiner `LIBRARY`-Button unter dem Volume-Panel
- startet `run_library.sh` als separaten Prozess
- nach Beenden lädt der Player die Bibliotheksdatei neu
- `LIBRARY` ist auch ohne laufende Wiedergabe nutzbar

### Bibliotheks-Konfiguration

```lua
library_file = home .. "/.local/share/xtcw-mp3player/library.lua",
library_playlist_dir = home .. "/.local/share/xtcw-mp3player/playlists",
```

### Tests

- `tests/test_playlist_library.lua`:
  - M3U-Album, Alias, M3U-Reihenfolge
  - `ensure()` importiert und persistiert beim ersten Lauf
  - gemischte Playlist
  - Auto-Gruppierung nach ID3
  - Rescan, Rename, Remove
  - keine Duplikate
- `tests/test_library_ui.sh`:
  - Standalone-GUI startet headless und rendert

## 7. Player-Controller## 7. Player-Controller

Start:

```lua
local script = require("script")
local job = script.start_argv(
  { "mpg123", "-R", "-o", output },
  { on_event = "on_player_event" }
)
```

Kommandos über `script.write`:

| Kommando | Bedeutung |
|---|---|
| `L <pfad>` | Track laden und abspielen |
| `LP <pfad>` | Track laden, pausiert |
| `P` | Play/Pause umschalten |
| `S` | Stop |
| `J <sek>s` | zu Sekunde springen |
| `K <sample>` | zu Sample springen |
| `V <0..100>` | Volume |
| `E 3 <band> <mult>` | EQ-Band setzen |
| `T` | ID3-Tags ausgeben |
| `STATE` / `SAMPLE` / `FORMAT` | Zusatzstatus |

Events:

- `@P 0` = stopped
- `@P 1` = paused
- `@P 2` = playing
- `@F <frame> <frames_left> <sec> <sec_left>`
- `@I`, `@T`, `@S`

Der pty-basierte Runner kann Kommando-Echos liefern. Zeilen, die nicht mit
`@` beginnen, werden ignoriert.

### Wiederverwendung des AdmPanel-Script-Wrappers

Der pty-basierte Runner aus `commander/script_process.c` wird bereits von
`apps/admpanel/script.lua` gekapselt. Für den Player wird eine minimale Kopie
unter `apps/mp3player/script.lua` verwendet:

- `start_argv(argv, opts)` → `script_start`
- `write(job, text)` → `script_write`
- `stop(job)` → `script_stop`
- `is_running(job)` → `script_is_running`

`apps/admpanel/scriptmeta.lua` wird **nicht** benötigt.

Wichtig:

- Immer `start_argv` mit einer argv-Tabelle verwenden.
- Kommandos an `mpg123` immer mit `\n` abschließen.
- Nur Zeilen verarbeiten, die mit `@` beginnen (pty-Echo filtern).
- `@F`-Events nur als letzten Zustand speichern; UI-Update im 25-fps-Timer.
- `@T`-Blöcke mehrzeilig parsen.

### Output-Fallback

`player.lua` probiert die konfigurierten Ausgänge der Reihe nach:

1. `player.output` (Default `pulse`)
2. `player.fallback_outputs` (Default `alsa`, `dummy`)
3. `dummy` als letzter Notausgang

Wenn ein Ausgangsmodul nicht antwortet oder mpg123 mit einem
`out123`-Fehler beendet wird, startet der Controller automatisch den
nächsten Ausgang und lädt den aktuellen Track erneut. Volume und EQ werden
dabei erneut angewendet.

### State-Machine

```
idle → playing ⇄ paused → stopped
                  │
                  └→ error
```

Track-Ende (`@P 0`/EOF) → nächster Track gemäß Repeat/Wrap-Konfiguration.

### EQ-Mapping

- UI-dB → Multiplier: `10^(dB/20)`
- Clamp: `0.1 .. 4.0`
- Jedes UI-Band wird auf einen Bereich der 32 MPEG-Subbänder gemappt und
  mit `E 3 <band> <mult>` gesetzt.

**Wichtig:** Das ist eine genäherte, codec-subband-basierte EQ-Lösung.
Für einen präzisen 5-Band-EQ wäre ein eigener C-DSP-Helper mit Biquads
nötig. Das ist bewusst nicht Teil von v1.

### Fallback

- `mpg123` fehlt oder Audio-Device nicht verfügbar:
  - `-o dummy` bzw. `player.mode = "demo"`
  - UI bleibt bedienbar
  - Spektrum/VU aus PCM-Cache oder auf 0
- `ffmpeg` fehlt:
  - Spektrum/VU deaktiviert
  - VFD/Transport/Seek bleiben funktionsfähig

## 8. Analyse und UI-Update

### PCM-Cache

Beim Track-Load:

```bash
ffmpeg -v error -i <datei> -vn -ac 2 -ar 22050 -f s16le <cache>.pcm
```

- asynchron über `script_start`/`sh -c`
- Abbruch bei Track-Wechsel
- Cache-Key: Pfad + mtime + Größe + Rate
- LRU/Cleanup nach `cache_max_mb`

### UI-Timer

Ein einziger Timer:

```lua
xtapptimeout(40, tick)   -- ~25 fps
```

`tick`:

1. letzten Player-Zustand lesen
2. VFD nur bei Sekunden-/Trackwechsel aktualisieren
3. SeekBar nur bei Positionsänderung aktualisieren (nicht während Drag)
4. PCM-Fenster an aktueller Position an `mp3fft` geben
5. VU/Spektrum mit geglätteten Werten aktualisieren

`gui.set` nur bei tatsächlicher Wertänderung.

### Glättung

- VU: Attack ~10 ms, Release ~300 ms, Peak-Hold ~1,5 s
- Spektrum: Attack/Release pro Band, z. B. 0,35/0,12

## 9. Custom Widgets

Alle als `Wheel`-Subklassen, Cairo-basiert, mit gecachter Pixmap/Surface.
Muster: `wbuild_widgets/EqFader.widget`.

### `WVfdDisplay`

- Ressourcen: `album`, `title`, `artist`, `elapsed`, `remaining`,
  `playState`, `fontSize`, `vfdColor`, `vfdDim`, `vfdBg`, `vfdGlow`
- links: `ALBUM:` + Album, `TRACK:` + Titel
- rechts: große Zeit, `ELAPSED`, gestrichelte Linie, `-MM:SS`,
  `REMAINING`
- Zeitziffern als 7-Segment-Primitive (Cairo)
- Album/Titel als Monospace-Text mit Glow und Ellipsis
- statische Labels cachen; nur geänderte Texte neu rendern

### `WVuMeter`

- Ressourcen: `level`, `peak` (0,1-dB-Schritte), `label`, `segments`,
  `minDb`, `maxDb`, `transparent`, Farben
- horizontale Segmentleiste
- dB-Skala: `-20 -10 -6 -3 0 +3 +6`
- Peak-Marker
- Farbzonen Grün → Gelb → Rot
- inaktive Segmente bleiben in ihrer Zonenfarbe schwach sichtbar, damit der
  Farberlauf auch bei niedrigem Pegel erkennbar ist
- der Farbbalken folgt dem Peak-Envelope; RMS allein lag bei leiser Musik
  deutlich unter dem Max-Marker

### `WSpectrum`

- Ressourcen: `levels` (kommaseparierte dB-Werte), `bands`, `minDb`,
  `maxDb`, `label`, `barColor`, `gridColor`, `showScale`
- Label/Skala in `DejaVu Sans Condensed-9`; 40 px Abstand bis zum Grid
- dunkles Panel, dB-Raster
- orange Balken mit Gradient/Highlight
- optionale Peak-Caps

### `WSeekBar`

- Ressourcen: `positionMs`, `durationMs`, `dragging`, Farben, Font
- Track, oranger Fill, Knopf
- runde Ecken (`cornerRadius`, Default 8), per XShape am Fenster beschnitten
- `showBorder = False` im App-Einsatz: kein eigener Rahmen im Modulpanel
- Track und Fill mit runden Enden
- Zeit links und rechts (`MM:SS` / `-MM:SS`)
- Drag, Click-to-Seek, Tastatur Links/Rechts/Home/End
- ruft den geerbten `callback` beim Commit

### EqFader-Anpassungen

- `showTicks = False` für EQ und Volume → keine Tick-Striche mehr
- `hideEndLabels = True` beim Volume-Fader → keine Labels/Marken bei
  `0` und `100`
- `showDb = False` beim Volume-Fader → Labels `20, 40, 60, 80`
- `labelOnTop = True` für die EQ-Fader → Frequenzlabels oben
- 14 px vertikaler Abstand zwischen Frequenzlabel und Fader-Skala
- kondensierte Schrift:
  `DejaVu Sans Condensed-9` (Label) und `-7` (Skala)
- der separate Header „5-BAND EQ“ wurde entfernt
- die fünf EQ-Fader sitzen in einem `WModulePanel` mit runden Ecken,
  dunklem Gradient, Bevel und vier Schrauben
- die Fader selbst sind `transparent = True`; ihr `ParentRelative`-Fenster
  lässt die Panel-Textur zwischen Track, Thumb und Labels sichtbar
- die beiden VU-Meter sitzen ebenfalls in je einem `WModulePanel` und sind
  transparent; die Panel-Textur scheint zwischen den Segmenten durch
- die transparenten VU-Meter zeichnen in eine mit der Parent-Panel-Pixmap
  vorbelegte Backing-Pixmap (kein Clear-Flackern, keine Geisterpixel)
- alle drei Modulpanels laufen mit `showScrews = False` (keine Schrauben)
- die Panels sind von den Chassis-Ecken abgerückt, damit sie die vier
  Gehäuseschrauben nicht berühren
- VU-Label und Skala nutzen `DejaVu Sans Condensed-9`; die Skala beginnt
  bei x=110, damit „RIGHT“ und „-20“ klar getrennt sind

### `WModulePanel`

- `Board`-Subklasse als Modulgehäuse
- runde Ecken (`cornerRadius`), per XShape auch am Fenster beschnitten
- dunkler Gradient, Bevel und vier Schrauben
- beliebige Child-Widgets, pixelgenau positioniert
- Screenshot: `widget_photo/mp3player_module_panel.png`

### `WTransportButton`

- flacher Button mit Cairo-Icon und kleiner Caption
- Icons: `play`, `pause`, `stop`, `prev`, `next`, `prev_album`, `next_album`
- Hover-/Pressed-Zustände
- Callback beim Loslassen
- Play-Button wechselt das Icon je nach Player-State (`play`/`pause`)
- runde Ecken (`cornerRadius`, Default 8), per XShape auch am Fenster
  beschnitten
- Caption wird bei Bedarf an Leerzeichen oder `/` auf zwei Zeilen umgebrochen
- Screenshot: `widget_photo/mp3player_transport.png`

### `WXftLabel`

Extrem kleines Test-Widget, bewusst ohne retex:

- Ressourcen: `label`, `xftFont`, `fg_norm`, `align`, `padX`, `padY`
- zeichnet direkt mit `XftDrawStringUtf8`
- Fenster hat `backgroundPixmap = ParentRelative`
- `WAudioChassis` veröffentlicht seine gerenderte Metall-Pixmap als
  `backgroundPixmap`; dadurch scheint die Chassis-Textur durch das Label
- Test: `tests/test_mp3_xftlabel.c`
- Screenshot: `widget_photo/mp3player_transparent_xft_label.png`

### `WAudioChassis`

Siehe Abschnitt 4.

## 9b. Grafik- und Widget-Techniken

Dieser Abschnitt beschreibt die Techniken, die beim Bau der mp3player-Widgets
entstanden sind. Sie sind bewusst allgemein gehalten und auch für andere
Audio-/Panel-Demos wiederverwendbar.

### 9b.1 Pixelgenaue Positionierung

- `WAudioChassis` ist eine feste Shell (1160×770) und das einzige Child des
  Root-Shell.
- Beliebige Kinder werden über ihre **Core-Geometrie** positioniert:
  `:x`, `:y`, `:width`, `:height`.
- `location`/`abs_x`/`rel_x` gehören zu `Board`-Widgets und funktionieren
  nur für Kinder, die selbst `Board`-Subklassen sind.
- Die Hauptpanels liegen auf festen Koordinaten; innerhalb der Panels dürfen
  `Gridbox`/`VBox`/`HBox` für Gruppen verwendet werden.
- Das Chassis bleibt starr; `query_geometry`/`set_values_almost` und
  WM-Min/Max-Hints verhindern Resize.

### 9b.2 Backing-Pixmap und Background-Pixmap

Zwei verschiedene Dinge:

| Begriff | Bedeutung |
|---|---|
| **Backing-Pixmap** | Offscreen-Zeichenfläche eines Widgets; Cairo zeichnet hinein, `XCopyArea` blittet ins Fenster |
| **Background-Pixmap** | X11-Ressource `XtNbackgroundPixmap`; wird vom X-Server/Xt zum Löschen des Fensters benutzt |

Wichtige Muster:

- `WAudioChassis` rendert Metall, Bevel und Schrauben **einmal** in eine
  Backing-Pixmap und veröffentlicht sie zusätzlich als `backgroundPixmap`.
- `WModulePanel` macht dasselbe für den Modul-Hintergrund.
- Kinder mit `backgroundPixmap = ParentRelative` (z. B. `WXftLabel`,
  `WVuMeter`) sehen dadurch die Textur des Parents.
- Vor dem Freigeben einer Pixmap immer erst
  `XtVaSetValues(..., XtNbackgroundPixmap, None, ...)` und ggf.
  `XSetWindowBackgroundPixmap(..., None)` setzen.
- Beim Resize wird die Backing-Pixmap neu erzeugt; `destroy_bg`/`destroy_pixmap`
  müssen die alte Pixmap sauber abhängen.

### 9b.3 Runde Ecken mit XShape

X11-Fenster sind rechteckig. Für echte runde Ecken:

1. 1-Bit-Maske mit `XCreatePixmap(..., 1)` erzeugen.
2. Rechteck + vier `XFillArc`-Ecken in die Maske zeichnen.
3. `XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, mask, ShapeSet)`.
4. Bei `cornerRadius = 0` die Shape mit `None` zurücksetzen.
5. Nach `realize`, bei `resize` und bei Änderung von `cornerRadius` erneut
   anwenden.

Wichtig gegen Eck-Artefakte:

- Die Backing-Pixmap **vor** dem Clipping vollständig mit der Panel-Farbe
  füllen.
- Shape-Maske und Cairo-Rundpfad an derselben Geometrie ausrichten
  (`0,0,w,h`, gleicher Radius).
- Sonst bleiben schwarze oder weiße Pixel in den Ecken sichtbar.

Verwendet von:

- `WModulePanel` (EQ-, VU-, Volume-, Seek-Modul)
- `WTransportButton`
- `WSeekBar`

### 9b.4 Transparente Widgets

X11-Child-Windows haben keine Alpha-Transparenz. Für „transparente“ Widgets
gibt es zwei Wege:

**a) ParentRelative + Background-Pixmap**

- Fenster bekommt `backgroundPixmap = ParentRelative`.
- Der Parent muss eine passende `backgroundPixmap` veröffentlichen.
- `XClearWindow` füllt freie Flächen mit der Parent-Textur.
- Verwendet von `WXftLabel`.

**b) Backing-Pixmap mit Parent-Textur vorbelegen**

- Backing-Pixmap aus der `XtNbackgroundPixmap` des Parents kopieren:
  `XCopyArea(parent_bg, pixmap, ...)`.
- Darauf das Widget zeichnen und die Pixmap ins Fenster blitten.
- **Nicht** aus dem Parent-Fenster kopieren: dort kann der vorherige
  Child-Frame enthalten sein → Geisterpixel.
- Verwendet von `WVuMeter` im transparenten Modus.

`EqFader` bietet zusätzlich `transparent = True`: Es zeichnet direkt ins
Fenster, nach `XClearWindow`, und lässt so die Panel-Textur zwischen Track,
Thumb und Labels sichtbar. Das ist für seltene Updates gedacht; häufig
aktualisierte Widgets nutzen die Backing-Pixmap-Variante.

### 9b.5 Flackern und Geisterpixel vermeiden

- `set_values` vergleicht **String-Inhalte** (`strcmp`), nicht Pointer.
- Neue Strings immer kopieren, bevor der alte Speicher freigegeben wird.
- Nach dem Kopieren direkt in die Backing-Pixmap zeichnen und `False`
  zurückgeben, damit Xt kein Clear/Expose auslöst.
- Die gerenderte Pixmap als `backgroundPixmap` setzen: Ein echtes Expose
  löscht dann mit dem letzten Frame, nicht mit einer hellen Default-Farbe.
- In Lua die zuletzt gesetzten Werte cachen (`last_vfd`, `last_spec`,
  `last_vu`) und `gui.set` nur bei echter Änderung aufrufen.
- Einen zentralen 100-ms-Timer verwenden, keine per-Widget-Timer.

### 9b.6 Text und Fonts

- `WXftLabel` nutzt direkt `XftDrawStringUtf8` und **kein retex**.
- `EqFader` trennt Label-Font (`xftFont`) und Skalen-Font (`scaleFont`).
- Für die EQ-/VU-/Spektrum-Beschriftung wird
  `DejaVu Sans Condensed-9` bzw. `-7` verwendet.
- `WVfdDisplay` zeichnet Zeit-Ziffern als 7-Segment-Primitive und Album/Titel
  mit Cairo-Text + Glow.
- `vfd_text_fit` kürzt lange Texte mit `…` und respektiert UTF-8-Grenzen;
  dadurch kann ein abgeschnittener Multibyte-String Cairo nicht mehr in den
  Error-State bringen.
- `WTransportButton` bricht Captions an Leerzeichen oder `/` auf zwei Zeilen
  um.

### 9b.7 Spezialisierte Widgets

| Widget | Besonderheit |
|---|---|
| `WAudioChassis` | feste Shell, Metall-Hintergrund, pixelgenaue Kinder, WM-Hints |
| `WModulePanel` | rundes Modulgehäuse, optionale Schrauben, XShape, Background-Pixmap |
| `WXftLabel` | minimales Xft-Label, ParentRelative, kein retex |
| `WTransportButton` | Cairo-Icons, runde Ecken, zweizeilige Captions |
| `WSeekBar` | runde Ecken, Drag/Click, optionale Border, Track/Fill mit runden Enden |
| `WSpectrum` | FFT-Balken, dB-Raster, Backing-Pixmap, Condensed-Font |
| `WVuMeter` | Segmentleiste, Peak-Marker, Zonenfarben, transparenter Modus |
| `WVfdDisplay` | 7-Segment-Zeit, Glow, Text-Fitting, UTF-8-sicher |

### 9b.8 Visuelle Regressionstests

- `tests/test_mp3_chassis.c` prüft feste Größe und pixelgenaue Kinder.
- `tests/test_mp3_xftlabel.c` prüft Xft-Text und ParentRelative.
- `tests/test_mp3_widgets.c` prüft VFD, Transport, Panel, Spektrum, VU und
  SeekBar.
- `tools/widget_shot.sh` erzeugt Einzel-Screenshots.
- `xrefresh` mehrfach auslösen, um Expose-/Geisterpixel-Probleme sichtbar zu
  machen.
- Screenshots liegen unter `widget_photo/`.

### 9b.9 Typische Fallstricke

- Ein LUI-`(window ...)` wird intern zu einer `Gridbox`; für eine feste Shell
  `WAudioChassis` direkt als Top-Level-Node verwenden.
- LUI mag keine negativen Zahl-Literale; negative Werte per `gui.set` oder
  als String-Resource setzen.
- `Wheel` hat bereits ein Integer-`state`; deshalb heißt das VFD-Resource
  `playState`.
- `ParentRelative` funktioniert nur, wenn der Parent eine
  `backgroundPixmap` gesetzt hat.
- Shape-Maske und Cairo-Pfad müssen zusammenpassen, sonst gibt es
  schwarze/weiße Eckpixel.
- mpg123-pty liefert Echo-Zeilen; nur `@`-Zeilen parsen.
- mpg123-Ausgangsmodule können fehlen/hängen; Output-Fallback verwenden.

## 10. Konfiguration (`config.lua`)

Die Konfiguration ist ausführbarer Lua-Code und liefert eine Tabelle zurück.

```lua
local home = os.getenv("HOME")

return {
  music_dir = home .. "/Musik",
  playlist = nil,                 -- nil/"auto" oder Pfad zur .m3u
  auto_playlist = "auto.m3u",
  extensions = { "mp3" },
  sort = "album",                 -- "m3u" | "album" | "artist" | "title" | "filename"
  start_track = 1,
  volume = 80,

  eq_bands = { 60, 250, 1000, 4000, 12000 },
  eq_gain  = { 0, 0, 0, 0, 0 },   -- dB

  spectrum = {
    bands  = 24,
    min_db = -50,
    max_db = 0,
    fps    = 25,
  },

  analysis = {
    rate      = 22050,
    fft_size  = 1024,
    cache_dir = home .. "/.cache/xtcw-mp3player",
    cache_max_mb = 512,
  },

  player = {
    engine           = "dsp",    -- "dsp" | "mpg123"
    sink             = "",       -- "" = Default-Sink
    command          = "mpg123",
    output           = "pulse",
    fallback_outputs = { "alsa", "dummy" },
  },

  chassis = {
    width  = 1160,
    height = 770,
  },
}
```

Laden:

1. `MP3PLAYER_CONFIG`
2. `~/.config/xtcw/mp3player.lua`
3. `apps/mp3player/config.lua`

Defaults mergen, Typen prüfen, Werte clampen, unbekannte Felder ignorieren.
Config in einer begrenzten Umgebung laden (`loadfile` mit eigener Env).

## 11. Verzeichnislayout

```
apps/mp3player/
  README.md
  run.sh
  main.lua
  config.lua
  ui.lua
  player.lua
  library.lua
  id3.lua
  m3u.lua
  analysis.lua            -- ffmpeg-Fallback für mpg123
  player_dsp.lua          -- DSP-Engine-Fassade
  fft.lua                 -- Wrapper um mp3fft.so
  theme.hdf
  c/
    Makefile
    mp3fft.c
    mp3engine.c
    mp3engine_test.c
    mpg123_min.h
  assets/
    icons/                -- optionale XPM/SVG-Icons
```

Neue Widgets:

```
wbuild_widgets/
  WAudioChassis.widget
  WVfdDisplay.widget
  WVuMeter.widget
  WSpectrum.widget
  WSeekBar.widget
  WTransportButton.widget
  WModulePanel.widget
  WXftLabel.widget
```

## 12. Build und Start

```bash
# Toolkit + Widgets
make all
make widgets

# C-FFT-Modul
make -C apps/mp3player/c

# App starten
./apps/mp3player/run.sh

# Mit eigener Config
./apps/mp3player/run.sh /pfad/zur/config.lua
```

`run.sh`:

- ermittelt `ROOT`
- exportiert `MP3PLAYER_ROOT`
- setzt `MP3PLAYER_CONFIG` (Argument oder Default)
- baut `mp3fft.so` bei Bedarf
- startet `run_lui.sh apps/mp3player/main.lua`
  mit `-themeFile apps/mp3player/theme.hdf -theme default`

## 13. Bedienung

| Taste / Aktion | Funktion |
|---|---|
| Space | Play/Pause |
| Links / Rechts | ±5 s seek |
| Up / Down | Volume |
| N / P | nächster/vorheriger Track |
| Shift+N / Shift+P | nächstes/vorheriges Album |
| L | Bibliothek öffnen |
| Q / Esc | Beenden |

Transport-Buttons:

- `PREV ALBUM`
- `PREV TRACK`
- `PLAY/PAUSE`
- `STOP`
- `NEXT TRACK`
- `NEXT ALBUM`

## 14. Tests

```bash
# Reine Lua-Tests
lua5.3 tests/test_mp3_id3.lua
lua5.3 tests/test_mp3_m3u.lua
lua5.3 tests/test_mp3_library.lua
lua5.3 tests/test_playlist_library.lua
lua5.3 tests/test_mp3_analysis.lua

# C-FFT
lua5.3 tests/test_mp3_fft.lua

# Standalone libmpg123 + Pulse DSP proof (skips without libmpg123-dev)
./tests/test_mp3engine.sh

# In-process DSP engine (null output)
./tests/test_mp3engine_module.sh

# Widgets / Chassis
xvfb-run --auto-servernum ./tests/test_mp3_chassis
xvfb-run --auto-servernum ./tests/test_mp3_xftlabel
xvfb-run --auto-servernum ./tests/test_mp3_widgets

# Wrapper + mpg123 -R (headless, dummy/pulse output)
./tests/test_mp3_player_wrapper.sh

# Standalone library GUI
./tests/test_library_ui.sh

# App-Smoke
MP3PLAYER_CONFIG=tests/fixtures/mp3player/config.lua \
  xvfb-run --auto-servernum ./run_lui.sh apps/mp3player/main.lua -Tracelevel 2

# Gesamttests
make run_tests
```

Zusätzlich:

- Pfade mit Leerzeichen/Umlauten
- fehlende Audiodateien
- kaputte ID3-Tags
- sehr lange Album-/Titeltitel
- kein Audio-Device (`-o dummy`)
- `mp3fft.so` fehlt

## 15. Akzeptanzkriterien

1. `./apps/mp3player/run.sh` startet unter X11 und Xvfb ohne Fehler.
2. `WAudioChassis` ist das einzige direkte Child des Root-Shell und enthält
   alle App-Widgets.
3. Chassis bleibt fest `1160×770`; Resize-Versuche ändern die Größe nicht.
4. Mindestens 50 Child-Widgets lassen sich pixelgenau positionieren.
5. Hintergrund wird einmal gerendert und bei Expose nur geblittet.
6. VFD, zwei VU-Meter, sechs Transport-Buttons, SeekBar, Volume,
   5 EQ-Fader und Spektrum sind sichtbar und überlappen nicht.
7. Play/Pause/Stop/Prev/Next Track/Prev/Next Album funktionieren.
8. Seek per Drag/Click funktioniert; Zeit links/rechts stimmt.
9. ID3v1 und ID3v2 werden gelesen und angezeigt; Fallback bei kaputten Tags.
10. M3U wird gelesen; ohne M3U wird automatisch eine erzeugt.
11. Sortierung nach M3U und nach ID3 liefert korrekte Reihenfolgen.
12. Config ist Lua-Code; Änderungen an `music_dir`, `sort`, `volume`,
    `eq_bands` wirken ohne Codeänderung.
13. EQ-Regler verändern den Klang hörbar (mpg123-EQ, genähert).
14. Spektrum/VU reagieren auf echte PCM-Daten, laufen mit ~25 fps und
    klingen bei Pause/Stop aus.
15. Idle-CPU < 2 %, Wiedergabe-UI-CPU < 10 %, Track-Wechsel < 300 ms,
    Seek-Reaktion < 100 ms, Scan von 1000 Dateien < 2 s (Cache warm).
16. `test_mp3_id3`, `test_mp3_m3u`, `test_mp3_library`, `test_mp3_fft`,
    `test_mp3_chassis`, `test_mp3_xftlabel`, `test_mp3_widgets`,
    `test_mp3_player_wrapper` und `test_mp3player.sh` laufen grün;
    `make run_tests` bleibt grün.
17. `t1.sh mp3player` startet die App; README und `know_bugs.md` sind
    aktualisiert.

## 16. Risiken und Gegenmaßnahmen

| Risiko | Gegenmaßnahme |
|---|---|
| `mpg123` fehlt / kein Audio-Device | `dummy`/`demo`-Fallback, klare Statusmeldung |
| `ffmpeg` fehlt | Spektrum/VU deaktivieren, Rest bleibt nutzbar |
| `mp3fft.so` nicht baubar | App startet, „SPECTRUM OFF“, kein Lua-FFT-Fallback |
| mpg123-EQ nur 32 Subbänder | Dokumentieren; optional später C-Biquad-Engine |
| pty-Echo | nur `@`-Zeilen parsen |
| ID3-Müll/Encoding-Fehler | defensiver Parser, `pcall`, Fallback |
| LUI/wbuild-Fallen | `know_bugs.md` befolgen; negative Werte per `gui.set` oder `location`-String |
| Audio/Analyse-Sync | Position aus `@F`; bei Seek Offset neu setzen |
| Cache wächst | `cache_max_mb`, LRU-Cleanup |
| WM ignoriert Min/Max-Hints | `query_geometry`/`set_values_almost` erzwingen feste Größe |
| Feste Koordinaten bei anderer DPI | Startgröße fix; Koordinaten ggf. per Config skalieren |
| Viele Children verlangsamen Expose | Hintergrund-Pixmap einmal; Children cachen selbst |

## 17. Umsetzungsreihenfolge

1. **Preflight/Skelett**
   - `apps/mp3player/` anlegen
   - `run.sh`, `config.lua`, leeres `main.lua`
   - `mpg123`, `ffmpeg`, `lua5.3`, Cairo, Xvfb prüfen

2. **`WAudioChassis`**
   - `Board`-Subklasse
   - feste Größe, WM-Hints, `query_geometry`, `set_values_almost`
   - Hintergrund-Pixmap, Schrauben/Bevel
   - C-Test: feste Größe, 50 Children, pixelgenaue Positionen
   - LUI-Test

3. **Datenlayer**
   - `id3.lua`, `m3u.lua`, `library.lua`, Config-Loader
   - Fixtures und Lua-Unit-Tests

4. **C-FFT**
   - `c/mp3fft.c`, `c/Makefile`
   - Lua-API, Build in `run.sh`
   - C- und Lua-Tests, Performance-Test

5. **Player-Controller**
   - `player.lua`, Event-Parser, State-Machine
   - Tests mit `mpg123 -R -o dummy`

6. **Analyse**
   - PCM-Cache über `ffmpeg`
   - `fft.lua` als Wrapper um `mp3fft.so`
   - Glättung, VU/Spektrum-Daten

7. **Custom Widgets**
   - `WVfdDisplay`, `WVuMeter`, `WSpectrum`, `WSeekBar`
   - C-Regressionstest und `widget_shot`-Screenshots

8. **App-Layout**
   - Panels im Chassis positionieren
   - Callbacks, Timer, Tastatur
   - funktionaler MVP

9. **Bibliothek/Playlists**
   - `WDialog` + `WListView`
   - M3U laden/speichern/rescan
   - Sortierung umschaltbar

10. **Polish/Integration**
    - Theme, Glow, Feinschliff der Koordinaten
    - `t1.sh`, `demos/README.md`, `demos/desc/mp3player.retex`
    - `know_bugs.md`, `Improved-Widget-Set.md`
    - Gesamttests und Performance-Messung
