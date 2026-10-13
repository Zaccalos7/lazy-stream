#!/usr/bin/env python3
"""Verify that a release archive or .deb runs from its installed layout."""

import pathlib
import subprocess
import sys
import tempfile
import tarfile


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
        else:
            with tarfile.open(package) as archive:
                archive.extractall(tmp, filter="data")
            app = next(root.glob("*/bin/orbis-stream"))
        if not app.is_file():
            raise RuntimeError("Executable not found in package")
        if package.suffix == ".deb":
            binary_version = subprocess.check_output([str(app), "--version"], text=True).strip()
            if binary_version != package_version:
                raise RuntimeError("Package and executable versions differ")
        subprocess.run([sys.executable, str(pathlib.Path(__file__).with_name("native_launch.py")), str(app)], check=True, timeout=30)
        subprocess.run(
            [sys.executable, str(pathlib.Path(__file__).with_name("integration.py")), str(app)],
            check=True,
            timeout=90,
        )
        print("Extracted release package: helper, resources, HTTP and streaming: OK")


if __name__ == "__main__":
    main()
