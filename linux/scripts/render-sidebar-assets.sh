#!/usr/bin/env bash
# Rigenera le immagini native dagli originali Windows senza dipendere dal plugin Qt SVG/ICO.
set -euo pipefail
resources="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../web/resources" && pwd)"
for flag in "$resources"/flags/*.svg; do
  rsvg-convert --width 66 --height 45 --output "${flag%.svg}.png" "$flag"
done
convert "$resources/favicon.ico[-1]" "$resources/favicon.png"
