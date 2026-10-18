#!/usr/bin/env bash
set -euo pipefail
# Run inside a disposable Ubuntu 24.04 container of the target architecture.
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y --no-install-recommends build-essential cmake pkg-config python3 file \
  libboost-dev libjson-c-dev libsqlite3-dev ffmpeg curl gnupg ca-certificates \
  libxcb1-dev gstreamer1.0-libav gstreamer1.0-plugins-good qt6-base-dev \
  qt6-multimedia-dev qt6-webengine-dev qt6-image-formats-plugins \
  qt6-translations-l10n fonts-noto-core fonts-noto-cjk \
  patchelf libssl-dev libwebp-dev zlib1g-dev libx264-dev libvpx-dev \
  libpulse-dev libxcb-shm0-dev libxcb-xfixes0-dev libxcb-render0-dev libxcb-shape0-dev libva-dev libx265-dev \
  libaom-dev libsvtav1-dev libsvtav1enc-dev libopus-dev libmp3lame-dev \
  libvorbis-dev libtheora-dev
if [[ "$(dpkg --print-architecture)" == amd64 ]]; then
  apt-get install -y --no-install-recommends nasm libvpl-dev libffmpeg-nvenc-dev
fi
task_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
export ORBIS_BUILD_DIR="${ORBIS_BUILD_DIR:-$task_root/build-$(dpkg --print-architecture)}"
# Give artifacts back to the owner of the mounted checkout, including after a failed build.
task_owner="$(stat -c '%u:%g' "$task_root")"
trap 'chown -hR "$task_owner" "$ORBIS_BUILD_DIR"' EXIT
bash "$task_root/scripts/build-linux.sh" --deb
