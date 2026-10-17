#!/bin/bash
set -euo pipefail
if [[ "$(uname -s)" != Darwin || "$(uname -m)" != arm64 ]]; then
    echo "Compila su un Mac Apple Silicon con macOS 14+ e Xcode Command Line Tools." >&2
    exit 1
fi
cd "$(dirname "$0")/.."
ffmpeg_dir="${ORBIS_FFMPEG_DIR:-/opt/homebrew/bin}"
for binary in ffmpeg ffprobe; do
    [[ -x "$ffmpeg_dir/$binary" ]] || { echo "Manca $ffmpeg_dir/$binary. Imposta ORBIS_FFMPEG_DIR." >&2; exit 1; }
done
app_identity="${ORBIS_APP_SIGN_IDENTITY:--}"
installer_identity="${ORBIS_INSTALLER_SIGN_IDENTITY:-}"
notary_profile="${ORBIS_NOTARY_PROFILE:-}"
if [[ -n "$notary_profile" && ( "$app_identity" == - || -z "$installer_identity" ) ]]; then
    echo "La notarizzazione richiede entrambe le identità Developer ID." >&2; exit 1
fi
macos_major="$(sw_vers -productVersion)"
macos_major="${macos_major%%.*}"
[[ "$macos_major" -ge 14 ]] || { echo "Richiesto macOS 14 o successivo." >&2; exit 1; }
xcrun --find swift >/dev/null
ORBIS_FFMPEG_DIR="$ffmpeg_dir" xcrun swift test --arch arm64
xcrun swift build -c release --arch arm64
binary_dir="$(xcrun swift build -c release --arch arm64 --show-bin-path)"
version="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' macOS/Info.plist)"
mkdir -p dist
staging="$(mktemp -d "${TMPDIR:-/tmp}/orbis-build.XXXXXX")"
trap 'rm -rf "$staging"' EXIT
app_dir="$staging/root/Applications/OrbisStream.app"
mkdir -p "$app_dir/Contents/MacOS" "$app_dir/Contents/Resources/FFmpeg"
cp "$binary_dir/OrbisStream" "$app_dir/Contents/MacOS/OrbisStream"
cp macOS/Info.plist "$app_dir/Contents/Info.plist"
cp LICENSE "$app_dir/Contents/Resources/LICENSE"
# SwiftPM-generated resource accessor plus native main-bundle localization.
for resource_bundle in "$binary_dir"/*.bundle; do
    [[ -d "$resource_bundle" ]] || continue
    cp -R "$resource_bundle" "$app_dir/Contents/Resources/"
done
for localization in Sources/OrbisMac/Resources/*.lproj; do
    cp -R "$localization" "$app_dir/Contents/Resources/"
done
xcrun swift scripts/bundle-ffmpeg.swift "$ffmpeg_dir" "$app_dir"
sign_args=(--force --sign "$app_identity")
if [[ "$app_identity" != - ]]; then sign_args+=(--options runtime --timestamp); fi
while IFS= read -r -d '' binary; do
    [[ "$(lipo -archs "$binary")" == arm64 ]] || { echo "Architettura errata: $binary" >&2; exit 1; }
    codesign "${sign_args[@]}" "$binary"
done < <(find "$app_dir/Contents/Frameworks" "$app_dir/Contents/Helpers" "$app_dir/Contents/MacOS" -type f -print0)
"$app_dir/Contents/Helpers/ffmpeg" -L > "$app_dir/Contents/Resources/FFmpeg/LICENSE.txt" 2>&1
"$app_dir/Contents/Helpers/ffmpeg" -buildconf > "$app_dir/Contents/Resources/FFmpeg/buildconf.txt" 2>&1
if [[ -n "${ORBIS_FFMPEG_LICENSE_DIR:-}" ]]; then
    cp -R "$ORBIS_FFMPEG_LICENSE_DIR" "$app_dir/Contents/Resources/FFmpeg/ThirdParty"
fi
codesign "${sign_args[@]}" "$app_dir"
codesign --verify --deep --strict "$app_dir"
"$app_dir/Contents/Helpers/ffprobe" -version >/dev/null
"$app_dir/Contents/Helpers/ffmpeg" -hide_banner -demuxers > "$staging/ffmpeg-demuxers.txt" 2>&1
if ! /usr/bin/grep -q webp_anim "$staging/ffmpeg-demuxers.txt"; then
    echo "Richiesto FFmpeg 9 o successivo con demuxer webp_anim per gli overlay WebP animati." >&2; exit 1
fi
"$app_dir/Contents/Helpers/ffmpeg" -hide_banner -decoders > "$staging/ffmpeg-decoders.txt" 2>&1
for decoder in libvpx libvpx-vp9; do
    if ! /usr/bin/grep -q "$decoder" "$staging/ffmpeg-decoders.txt"; then
        echo "Manca il decoder $decoder necessario agli overlay WebM con alpha." >&2; exit 1
    fi
done
"$app_dir/Contents/Helpers/ffmpeg" -hide_banner -h full > "$staging/ffmpeg-help.txt" 2>&1
if ! /usr/bin/grep -q readrate_initial_burst "$staging/ffmpeg-help.txt"; then
    echo "La distribuzione FFmpeg deve supportare readrate_initial_burst." >&2; exit 1
fi
"$app_dir/Contents/Helpers/ffmpeg" -hide_banner -loglevel error -f lavfi -i color=size=64x64:rate=30 -frames:v 1 -c:v h264_videotoolbox -allow_sw 0 -f null -
pkgbuild --root "$staging/root" --identifier com.orbis.stream --version "$version" --install-location / "$staging/component.pkg"
cat > "$staging/requirements.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<plist version="1.0"><dict><key>arch</key><array><string>arm64</string></array><key>os</key><array><string>14.0</string></array></dict></plist>
PLIST
productbuild --synthesize --product "$staging/requirements.plist" --package "$staging/component.pkg" "$staging/Distribution.xml"
installer_args=()
if [[ -n "$installer_identity" ]]; then installer_args+=(--sign "$installer_identity" --timestamp); fi
installer="$PWD/dist/OrbisStream-$version-arm64.pkg"
productbuild --distribution "$staging/Distribution.xml" --package-path "$staging" "${installer_args[@]}" "$installer"
if [[ -n "$notary_profile" ]]; then
    xcrun notarytool submit "$installer" --keychain-profile "$notary_profile" --wait
    xcrun stapler staple "$installer"
    xcrun stapler validate "$installer"
fi
# Replace only the generated application after all packaging checks succeed.
if [[ -e dist/OrbisStream.app ]]; then rm -rf dist/OrbisStream.app; fi
cp -R "$app_dir" dist/OrbisStream.app
printf 'App: %s\nInstaller: %s\n' "$PWD/dist/OrbisStream.app" "$installer"
