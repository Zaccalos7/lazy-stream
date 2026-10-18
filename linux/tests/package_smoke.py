#!/usr/bin/env python3
"""Verify that a release archive or .deb runs from its installed layout."""

import pathlib
import os
import subprocess
import sys
import tempfile
import tarfile
import struct


def main() -> None:
    """Extract a package and test HTTP/streaming against its installed executable."""
    package = pathlib.Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix="orbis-package-") as tmp:
        root = pathlib.Path(tmp)
        if package.suffix == ".deb":
            # Extract without installing system-wide or requiring root.
            subprocess.run(["dpkg-deb", "--extract", str(package), tmp], check=True)
            app = root / "usr/bin/orbis-stream"
            package_version = subprocess.check_output(
                ["dpkg-deb", "--field", str(package), "Version"], text=True
            ).strip()
            architecture = subprocess.check_output(
                ["dpkg-deb", "--field", str(package), "Architecture"], text=True).strip()
            dependencies = subprocess.check_output(["dpkg-deb", "--field", str(package), "Depends"], text=True)
            if "qt6-image-formats-plugins" not in dependencies:
                raise RuntimeError("Missing Qt dependency for manual WebP figures")
            configuration = root / "etc/orbis-stream/orbis-stream.env"
            assert "LAZY_STREAM_HOST=127.0.0.1" in configuration.read_text()
            with tempfile.TemporaryDirectory(prefix="orbis-control-") as control:
                subprocess.run(["dpkg-deb", "--control", str(package), control], check=True)
                assert "/etc/orbis-stream/orbis-stream.env" in (pathlib.Path(control) / "conffiles").read_text()
        else:
            with tarfile.open(package) as archive:
                archive.extractall(tmp, filter="data")
            app = next(root.glob("*/bin/orbis-stream"))
        if not app.is_file():
            raise RuntimeError("Executable not found in package")
        web = app.parent.parent / "share/orbis-stream/web"
        assert (app.parent.parent / "share/doc/orbis-stream/README.md").is_file()
        for file in web.rglob("*"):
            if file.suffix in (".json", ".properties", ".html", ".js") and "tiktok" in file.read_text().lower():
                raise RuntimeError("Removed provider still present in installed product: " + str(file))
        ffmpeg = app.parent.parent / "libexec/orbis-stream/ffmpeg/ffmpeg"
        ffprobe = ffmpeg.with_name("ffprobe")
        env = os.environ.copy()
        env["LAZY_STREAM_HOST"] = "127.0.0.1"
        env["FFMPEG_PATH"] = str(ffmpeg)
        env["FFPROBE_PATH"] = str(ffprobe)
        for tool in (ffmpeg, ffprobe):
            if package.suffix == ".deb":
                elf = tool.read_bytes()[:20]
                machine = struct.unpack("<H", elf[18:20])[0]
                expected = {"amd64": 62, "arm64": 183}[architecture]
                if elf[:5] != b"\x7fELF\x02" or machine != expected:
                    raise RuntimeError("FFmpeg architecture differs from Debian package")
            version = subprocess.check_output([str(tool), "-version"], text=True)
            if "version 9.0.2" not in version.splitlines()[0]:
                raise RuntimeError("Package does not include FFmpeg/FFprobe 9.0.2")
        if package.suffix == ".deb":
            if struct.unpack("<H", app.read_bytes()[18:20])[0] != expected:
                raise RuntimeError("Application architecture differs from Debian package")
            binary_version = subprocess.check_output([str(app), "--version"], text=True).strip()
            if binary_version != package_version:
                raise RuntimeError("Package and executable versions differ")
        runtime_env = env.copy()
        runtime_env.pop("FFMPEG_PATH", None)
        runtime_env.pop("FFPROBE_PATH", None)
        subprocess.run([sys.executable, str(pathlib.Path(__file__).with_name("native_launch.py")), str(app)], check=True, timeout=30, env=runtime_env)
        subprocess.run([sys.executable, str(pathlib.Path(__file__).with_name("network_environment.py")), str(app)], check=True, timeout=30, env=env)
        subprocess.run(
            [sys.executable, str(pathlib.Path(__file__).with_name("integration.py")), str(app)],
            check=True,
            timeout=90,
            env=env,
        )
        subprocess.run(
            [sys.executable, str(pathlib.Path(__file__).with_name("delivery.py")), str(app)],
            check=True, timeout=90, env=env,
        )
        print("Extracted release package: helper, resources, HTTP, native FFmpeg 9 and complete delivery: OK")


if __name__ == "__main__":
    main()
