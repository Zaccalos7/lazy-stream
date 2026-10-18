#!/usr/bin/env bash
set -euo pipefail
# Risolve la cartella dello script per compilare anche quando viene invocato da un'altra directory.
task_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
task_build="${ORBIS_BUILD_DIR:-$task_root/build}"
export ORBIS_BUILD_DIR="$task_build"
task_arch="$(dpkg --print-architecture)"
case "$task_arch" in
  amd64|arm64) ;;
  *) printf 'Architettura non supportata: %s\n' "$task_arch" >&2; exit 2 ;;
esac
case "${1:-}" in
  ""|--deb|--package) ;;
  *)
    printf 'Uso: %s [--deb|--package]\n' "$0" >&2
    exit 2
    ;;
esac
if [[ ! -f "$task_build/ffmpeg-9.0.2/lib/pkgconfig/libavformat.pc" ]]; then
  python3 "$task_root/scripts/prepare-ffmpeg.py"
fi
cmake -S "$task_root" -B "$task_build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "$task_build" --parallel "${ORBIS_BUILD_JOBS:-2}"
if [[ "$("$task_build/orbis-stream" --version)" != "$(<"$task_root/VERSION")" ]]; then
  printf 'Versione binaria diversa da linux/VERSION\n' >&2
  exit 1
fi
# Extracted development dependencies also serve the package smoke test before APT installation.
task_webengine_libs="$task_build/qt-webengine/usr/lib/$(dpkg-architecture -qDEB_HOST_MULTIARCH)"
if [[ -f "$task_webengine_libs/libQt6WebEngineCore.so.6" ]]; then
  export LD_LIBRARY_PATH="$task_webengine_libs${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
  export QT_PLUGIN_PATH="$task_webengine_libs/qt6/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"
fi
ctest --test-dir "$task_build" --output-on-failure
# Genera pacchetti solo dopo il successo dei test, evitando artefatti di una build non verificata.
if [[ "${1:-}" == "--deb" || "${1:-}" == "--package" ]]; then
  mkdir -p "$task_build/packages"
  install -m 755 "$task_root/scripts/install-deb.sh" "$task_build/packages/install-deb.sh"
fi
if [[ "${1:-}" == "--deb" ]]; then
  cpack --config "$task_build/CPackConfig.cmake" -G DEB -B "$task_build/packages"
  deb="$task_build/packages/orbis-stream_$(<"$task_root/VERSION")_$(dpkg --print-architecture).deb"
  python3 "$task_root/tests/package_smoke.py" "$deb"
  printf 'Pacchetto Debian verificato: %s\n' "$deb"
elif [[ "${1:-}" == "--package" ]]; then
  cpack --config "$task_build/CPackConfig.cmake" -G TGZ -B "$task_build/packages"
  cpack --config "$task_build/CPackConfig.cmake" -G DEB -B "$task_build/packages"
  deb="$task_build/packages/orbis-stream_$(<"$task_root/VERSION")_$(dpkg --print-architecture).deb"
  python3 "$task_root/tests/package_smoke.py" "$deb"
  printf 'Pacchetto Debian verificato: %s\n' "$deb"
  printf 'Archivio: %s\n' "$task_build"/packages/OrbisStreamLinux-*.tar.gz
fi
