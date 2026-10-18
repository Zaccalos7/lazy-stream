#!/usr/bin/env bash
set -euo pipefail
# Put this script beside the amd64 and arm64 .deb files. APT installs native dependencies.
task_directory="${1:-$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)}"
task_arch="$(dpkg --print-architecture)"
case "$task_arch" in
  amd64|arm64) ;;
  *) printf 'Architettura non supportata: %s\n' "$task_arch" >&2; exit 2 ;;
esac
task_selected=""
task_version=""
shopt -s nullglob
for task_package in "$task_directory"/orbis-stream_*_"$task_arch".deb; do
  [[ "$(dpkg-deb --field "$task_package" Architecture)" == "$task_arch" ]] || continue
  [[ "$(dpkg-deb --field "$task_package" Package)" == "orbis-stream" ]] || continue
  task_candidate="$(dpkg-deb --field "$task_package" Version)"
  if [[ -z "$task_selected" ]] || dpkg --compare-versions "$task_candidate" gt "$task_version"; then
    task_selected="$task_package"
    task_version="$task_candidate"
  fi
done
if [[ -z "$task_selected" ]]; then
  printf 'Nessun pacchetto OrbisStream per %s in %s\n' "$task_arch" "$task_directory" >&2
  exit 1
fi
task_selected="$(realpath -- "$task_selected")"
printf 'Architettura %s: %s\n' "$task_arch" "$task_selected"
if [[ "${2:-}" == "--print-path" ]]; then
  exit 0
fi
if [[ "$EUID" == 0 ]]; then
  exec apt install "$task_selected"
else
  exec sudo apt install "$task_selected"
fi
