#!/usr/bin/env python3
"""Prepare distro Qt WebEngine development dependencies without changing the system."""
import concurrent.futures
import os
from pathlib import Path
import subprocess
import tempfile


def main():
    linux = Path(__file__).resolve().parents[1]
    build = Path(os.environ.get("ORBIS_BUILD_DIR", linux / "build")).resolve()
    destination = build / "qt-webengine"
    multiarch = subprocess.check_output(["dpkg-architecture", "-qDEB_HOST_MULTIARCH"], text=True).strip()
    environment = {**os.environ, "LC_ALL": "C"}
    plan = subprocess.check_output(
        ["apt-get", "-s", "install", "--no-install-recommends", "qt6-webengine-dev", "qt6-image-formats-plugins"],
        text=True, env=environment,
    )
    packages = [line.split()[1] for line in plan.splitlines() if line.startswith("Inst ")]
    if not packages:
        print("Qt WebEngine development packages are already installed.")
        return
    with tempfile.TemporaryDirectory(prefix="orbis-webengine-debs-") as temporary:
        def download(package):
            subprocess.run(["apt-get", "download", package], cwd=temporary,
                           env=environment, check=True)
        with concurrent.futures.ThreadPoolExecutor(max_workers=4) as executor:
            list(executor.map(download, packages))
        destination.mkdir(parents=True, exist_ok=True)
        for package in Path(temporary).glob("*.deb"):
            subprocess.run(["dpkg-deb", "-x", str(package), str(destination)], check=True)
    # Qt imports targets relative to one prefix, including base libraries already on the system.
    libraries = destination / "usr/lib" / multiarch
    libraries.mkdir(parents=True, exist_ok=True)
    for original in (Path("/usr/lib") / multiarch).glob("libQt6*"):
        target = libraries / original.name
        if not target.exists() and not target.is_symlink():
            target.symlink_to(original)
    # Qt also searches beside the development executable; installed builds use distro plugins.
    image_formats = build / "imageformats"
    if not image_formats.exists() and not image_formats.is_symlink():
        image_formats.symlink_to(libraries / "qt6/plugins/imageformats", target_is_directory=True)
    print(f"Qt WebEngine is ready in {destination}")


if __name__ == "__main__":
    main()
