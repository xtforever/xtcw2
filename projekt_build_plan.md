# Projekt-Build- und Strukturplan (Plain C / Linux / GNU Make)

## 1. Zielarchitektur

    project/
    │
    ├── src/
    │   ├── core/
    │   ├── widgets/
    │   ├── layout/
    │   └── util/
    │
    ├── include/
    │   └── project/
    │
    ├── widgets/            # *.widget Quellen
    │
    ├── generated/          # vom Precompiler erzeugt
    │
    ├── build/              # alle .o, .a, .so
    │
    ├── Makefile
    └── Dockerfile

### Regeln

-   Keine `.o`-Dateien im Repository
-   Kein Build im `src/`
-   Generierter Code strikt getrennt
-   Include-Pfade sauber halten

------------------------------------------------------------------------

## 2. Compiler- und Build-Flags

``` make
CC := gcc

CFLAGS := -Wall -Wextra -Werror -std=c11 -fPIC
LDFLAGS :=
```

`-fPIC` ist erforderlich für Shared Libraries.

------------------------------------------------------------------------

## 3. Integration des Widget-Precompilers

Annahme:

    widgets/button.widget

wird zu:

    generated/button.c
    generated/button.h

Make-Regel:

``` make
WIDGETS := $(wildcard widgets/*.widget)
GEN_C   := $(patsubst widgets/%.widget,generated/%.c,$(WIDGETS))
GEN_H   := $(patsubst widgets/%.widget,generated/%.h,$(WIDGETS))

generated/%.c generated/%.h: widgets/%.widget
    ./widget_precompiler $< generated/
```

Wichtig: - Generator muss deterministisch arbeiten - Generierter Code
darf nicht manuell verändert werden

------------------------------------------------------------------------

## 4. Quellen und Objekte

``` make
SRC := $(wildcard src/**/*.c) $(GEN_C)
OBJ := $(patsubst %.c,build/%.o,$(SRC))

build/%.o: %.c
    mkdir -p $(dir $@)
    $(CC) $(CFLAGS) -Iinclude -Igenerated -c $< -o $@
```

------------------------------------------------------------------------

## 5. Statische Library

``` make
build/libproject.a: $(OBJ)
    ar rcs $@ $^
```

------------------------------------------------------------------------

## 6. Shared Library

``` make
build/libproject.so: $(OBJ)
    $(CC) -shared -o $@ $^
```

------------------------------------------------------------------------

## 7. Gesamttarget

``` make
all: build/libproject.a build/libproject.so
```

------------------------------------------------------------------------

## 8. Automatische Header-Abhängigkeiten

``` make
CFLAGS += -MMD -MP
-include $(OBJ:.o=.d)
```

------------------------------------------------------------------------

## 9. Dockerfile (reproduzierbare Umgebung)

``` dockerfile
FROM gcc:13

RUN apt-get update && apt-get install -y make

WORKDIR /app
COPY . .

RUN make
```

Optional: Compiler-Version fixieren (z.B. Debian + gcc-13).

------------------------------------------------------------------------

## 10. Typische Fehlerquellen

1.  Alte `.o`-Dateien weiterverwenden → vermeiden
2.  Unterschiedliche Compiler-Flags → ABI-Probleme
3.  Shared Library ohne `-fPIC`
4.  Nicht-deterministischer Codegenerator

------------------------------------------------------------------------

## 11. Modularisierung (optional empfohlen)

Bei wachsender Codebasis:

    libcore.a
    libwidgets.a
    liblayout.a

Dann Aggregation in:

    libproject.so

Verhindert zyklische Abhängigkeiten und verbessert Wartbarkeit.

------------------------------------------------------------------------

## 12. Migrationsplan

1.  Alle alten `.o` löschen
2.  Neue Verzeichnisstruktur anlegen
3.  Generator isolieren
4.  Makefile neu schreiben (nicht patchen)
5.  Alles vollständig neu kompilieren
6.  Docker einführen
7.  Optional: CI/CD integrieren

------------------------------------------------------------------------

## 13. Architekturprinzip

Objektdateien sind keine Architektur.\
Quellcode + Buildsystem definieren die Struktur und Wartbarkeit des
Projekts.
