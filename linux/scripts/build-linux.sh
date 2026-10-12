#!/usr/bin/env bash
set -euo pipefail
# Risolve la cartella dello script per compilare anche quando viene invocato da un'altra directory.
task_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
case "${1:-}" in
  ""|--deb|--package) ;;
  *)
    printf 'Uso: %s [--deb|--package]\n' "$0" >&2
    exit 2
    ;;
esac
cmake -S "$task_root" -B "$task_root/build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "$task_root/build" --parallel "${ORBIS_BUILD_JOBS:-2}"
if [[ "$("$task_root/build/orbis-stream" --version)" != "$(<"$task_root/VERSION")" ]]; then
  printf 'Versione binaria diversa da linux/VERSION\n' >&2
  exit 1
fi
ctest --test-dir "$task_root/build" --output-on-failure
# Genera pacchetti solo dopo il successo dei test, evitando artefatti di una build non verificata.
if [[ "${1:-}" == "--deb" ]]; then
  cpack --config "$task_root/build/CPackConfig.cmake" -G DEB -B "$task_root/build/packages"
  deb="$task_root/build/packages/orbis-stream_$(<"$task_root/VERSION")_$(dpkg --print-architecture).deb"
  python3 "$task_root/tests/package_smoke.py" "$deb"
  printf 'Pacchetto Debian verificato: %s\n' "$deb"
elif [[ "${1:-}" == "--package" ]]; then
  cpack --config "$task_root/build/CPackConfig.cmake" -G TGZ -B "$task_root/build/packages"
  cpack --config "$task_root/build/CPackConfig.cmake" -G DEB -B "$task_root/build/packages"
  deb="$task_root/build/packages/orbis-stream_$(<"$task_root/VERSION")_$(dpkg --print-architecture).deb"
  python3 "$task_root/tests/package_smoke.py" "$deb"
  printf 'Pacchetto Debian verificato: %s\n' "$deb"
  printf 'Archivio: %s\n' "$task_root"/build/packages/OrbisStreamLinux-*.tar.gz
fi
