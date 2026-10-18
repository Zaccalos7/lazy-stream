#!/usr/bin/env python3
"""Build the signed FFmpeg 9.0.2 release locally, without changing the system installation."""
import concurrent.futures
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import urllib.request

VERSION = "9.0.2"
SIGNER = "FCF986EA15E6E293A5644F10B4322F04D67658D8"


def main():
    root = Path(os.environ.get("ORBIS_BUILD_DIR", Path(__file__).resolve().parents[1] / "build")).resolve()
    architecture = subprocess.check_output(["dpkg", "--print-architecture"], text=True).strip()
    if architecture not in ("amd64", "arm64"):
        raise RuntimeError(f"Unsupported architecture: {architecture}; use amd64 or arm64")
    source = root / "ffmpeg-source"
    prefix = root / f"ffmpeg-{VERSION}"
    source.mkdir(parents=True, exist_ok=True)
    for name, url in [
        (f"ffmpeg-{VERSION}.tar.xz", f"https://ffmpeg.org/releases/ffmpeg-{VERSION}.tar.xz"),
        (f"ffmpeg-{VERSION}.tar.xz.asc", f"https://ffmpeg.org/releases/ffmpeg-{VERSION}.tar.xz.asc"),
        ("ffmpeg-devel.asc", "https://ffmpeg.org/ffmpeg-devel.asc"),
    ]:
        path = source / name
        if not path.exists():
            urllib.request.urlretrieve(url, path)
    keyring = source / "gnupg"
    keyring.mkdir(mode=0o700, exist_ok=True)
    keyring.chmod(0o700)
    subprocess.run(["gpg", "--homedir", str(keyring), "--batch", "--import",
                    str(source / "ffmpeg-devel.asc")], check=True, capture_output=True)
    verified = subprocess.run(["gpg", "--homedir", str(keyring), "--batch", "--status-fd", "1",
                               "--verify", str(source / f"ffmpeg-{VERSION}.tar.xz.asc"),
                               str(source / f"ffmpeg-{VERSION}.tar.xz")],
                              check=True, capture_output=True, text=True)
    if f"VALIDSIG {SIGNER} " not in verified.stdout:
        raise RuntimeError("The release was not signed by the pinned FFmpeg release key")
    print(f"Verified FFmpeg {VERSION} ({SIGNER})", flush=True)
    packages = ["patchelf", "libssl-dev", "libwebp-dev", "zlib1g-dev", "libx264-dev", "libvpx-dev", "libpulse-dev", "libxcb1-dev", "libxcb-shm0-dev",
                "libxcb-xfixes0-dev", "libxcb-render0-dev", "libxcb-shape0-dev", "libva-dev",
                "libx265-dev", "libaom-dev", "libsvtav1-dev", "libsvtav1enc-dev", "libopus-dev", "libmp3lame-dev",
                "libvorbis-dev", "libtheora-dev"]
    if architecture == "amd64":
        packages += ["nasm", "libvpl-dev", "libffmpeg-nvenc-dev"]
    environment = {**os.environ, "LC_ALL": "C"}
    plan = subprocess.check_output(["apt-get", "-s", "install", "--no-install-recommends", *packages],
                                   text=True, env=environment)
    needed = [line.split()[1] for line in plan.splitlines() if line.startswith("Inst ")]
    needed = sorted(set(needed + packages))
    dependencies = source / "dependencies"
    dependencies.mkdir(exist_ok=True)
    def download(package):
        subprocess.run(["apt-get", "download", package], cwd=dependencies, env=environment,
                       stdout=subprocess.DEVNULL, check=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as executor:
        list(executor.map(download, needed))
    sdk = dependencies / "root"
    for package in dependencies.glob("*.deb"):
        subprocess.run(["dpkg-deb", "-x", str(package), str(sdk)], check=True)
    multiarch = subprocess.check_output(["dpkg-architecture", "-qDEB_HOST_MULTIARCH"], text=True).strip()
    lib = sdk / "usr/lib" / multiarch
    for symlink in lib.glob("*.so*"):
        if symlink.is_symlink() and not symlink.exists():
            system = Path("/usr/lib") / multiarch / symlink.readlink().name
            if system.exists():
                symlink.unlink()
                symlink.symlink_to(system)
    pc = dependencies / "pkgconfig"
    pc.mkdir(exist_ok=True)
    for package in sdk.glob("usr/**/pkgconfig/*.pc"):
        (pc / package.name).write_text(package.read_text()
            .replace("${pcfiledir}", str(package.parent))
            .replace("=/usr", f"={sdk}/usr"))
    environment["PKG_CONFIG_PATH"] = str(pc) + ":" + environment.get("PKG_CONFIG_PATH", "")
    environment["PATH"] = str(sdk / "usr/bin") + ":" + environment["PATH"]
    environment["LD_LIBRARY_PATH"] = str(lib) + ":" + environment.get("LD_LIBRARY_PATH", "")
    tree = source / f"ffmpeg-{VERSION}"
    if not tree.exists():
        with tarfile.open(source / f"ffmpeg-{VERSION}.tar.xz") as archive:
            archive.extractall(source, filter="data")
    arguments = [str(tree / "configure"), f"--prefix={prefix}", "--enable-gpl", "--enable-version3",
                 "--enable-shared", "--disable-static", "--disable-debug", "--disable-doc",
                 "--disable-ffplay", "--disable-autodetect", "--enable-openssl", "--enable-libx264",
                 "--enable-libvpx", "--enable-libwebp", "--enable-libx265", "--enable-libaom",
                 "--enable-libsvtav1", "--enable-libopus", "--enable-libmp3lame", "--enable-libvorbis",
                 "--enable-libtheora", "--enable-libpulse", "--enable-libxcb",
                 "--enable-libxcb-shm", "--enable-libxcb-xfixes", "--enable-vaapi",
                 "--enable-zlib", f"--extra-cflags=-I{sdk}/usr/include",
                 f"--extra-ldflags=-L{lib}"]
    if architecture == "amd64":
        arguments += ["--enable-libvpl", "--enable-ffnvcodec", "--enable-nvenc"]
    with (source / "configure.log").open("w") as log:
        result = subprocess.run(arguments, cwd=tree, env=environment, stdout=log, stderr=log)
    if result.returncode:
        raise RuntimeError(f"FFmpeg configure failed: {source / 'configure.log'}")
    jobs = os.environ.get("ORBIS_FFMPEG_JOBS", str(min(os.cpu_count() or 2, 6)))
    with (source / "build.log").open("w") as log:
        subprocess.run(["make", f"-j{jobs}"], cwd=tree, env=environment, stdout=log, stderr=log, check=True)
        subprocess.run(["make", "install"], cwd=tree, env=environment, stdout=log, stderr=log, check=True)
    # oneVPL may be absent from the base system; keep its runtime next to the FFmpeg libraries.
    for library in lib.glob("libvpl.so.*"):
        shutil.copy2(library.resolve(), prefix / "lib" / library.name)
    patcher = shutil.which("patchelf", path=environment["PATH"])
    for binary in (prefix / "bin").iterdir():
        subprocess.run([patcher, "--set-rpath", "$ORIGIN/../lib:$ORIGIN/../../../lib/orbis-stream/ffmpeg",
                        str(binary)], check=True)
    for library in (prefix / "lib").glob("*.so.*"):
        if not library.is_symlink():
            subprocess.run([patcher, "--set-rpath", "$ORIGIN", str(library)], check=True)
    shutil.copy2(tree / "COPYING.GPLv3", prefix / "COPYING.GPLv3")
    (prefix / "SOURCE.txt").write_text(f"https://ffmpeg.org/releases/ffmpeg-{VERSION}.tar.xz\n"
        f"Architecture: {architecture}\n"
        f"Release signing key: {SIGNER}\nSHA256: " +
        hashlib.sha256((source / f"ffmpeg-{VERSION}.tar.xz").read_bytes()).hexdigest() + "\n")
    subprocess.run([str(prefix / "bin/ffmpeg"), "-version"], env=environment, check=True)
    print(f"FFmpeg and development libraries ready in {prefix}")


if __name__ == "__main__":
    main()
