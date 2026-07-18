#!/bin/bash
#
# Dieses Skript kompiliert die GLSL-Shader des Projekts in das SPIR-V-Format.
#
# Bricht bei Fehlern sofort ab.
set -e

# --- Konfiguration ---
COMPILER="glslangValidator"
SRC_DIR="src"
OUT_DIR="assets"

VERTEX_SHADER="$SRC_DIR/main.vs"
FRAGMENT_SHADER="$SRC_DIR/main.fs"

# --- Logik ---

# 1. Prüfen, ob der Compiler verfügbar ist
if ! command -v $COMPILER &> /dev/null
then
    echo "Fehler: '$COMPILER' wurde nicht gefunden."
    echo "Bitte installiere 'glslang-tools' (z.B. mit 'sudo apt install glslang-tools')."
    exit 1
fi

# 2. Prüfen, ob die Quelldateien existieren
if [ ! -f "$VERTEX_SHADER" ] || [ ! -f "$FRAGMENT_SHADER" ]; then
    echo "Fehler: Shader-Quelldateien nicht gefunden."
    echo "  Erwartet: $VERTEX_SHADER"
    echo "  Erwartet: $FRAGMENT_SHADER"
    exit 1
fi

# 3. Sicherstellen, dass das Ausgabe-Verzeichnis existiert
mkdir -p "$OUT_DIR"

echo "Kompiliere Shader nach SPIR-V mit $COMPILER..."

# Wir verwenden -S <stage>, um die Kompatibilität mit älteren Versionen von glslangValidator zu gewährleisten.
# -G erzeugt SPIR-V für OpenGL, was lose Uniforms erlaubt.
$COMPILER -G -S vert "$VERTEX_SHADER" -o "$OUT_DIR/main.vert.spv" && echo "  [OK] $VERTEX_SHADER -> $OUT_DIR/main.vert.spv"
$COMPILER -G -S frag "$FRAGMENT_SHADER" -o "$OUT_DIR/main.frag.spv" && echo "  [OK] $FRAGMENT_SHADER -> $OUT_DIR/main.frag.spv"

echo "Shader-Kompilierung erfolgreich abgeschlossen."
