#!/usr/bin/env bash
set -euo pipefail

# Punto di ingresso per chi vuole l'installer .deb: compila, testa e impacchetta.
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
exec "$script_dir/build-linux.sh" --deb
