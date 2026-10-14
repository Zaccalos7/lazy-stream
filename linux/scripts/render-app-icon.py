#!/usr/bin/env python3
"""Render the desktop monitor icon with the original sidebar logo inside its screen."""
import base64
import shutil
import subprocess
from pathlib import Path

linux = Path(__file__).resolve().parents[1]
resources = linux / "web/resources"
icons = linux / "packaging/icons/hicolor"
logo = resources / "favicon.png"
# The final ICO frame is the original 256px artwork, suitable for high density displays.
subprocess.run(["convert", str(resources / "favicon.ico") + "[-1]", str(logo)], check=True)
embedded = base64.b64encode(logo.read_bytes()).decode("ascii")
svg = icons / "scalable/apps/orbis-stream.svg"
svg.parent.mkdir(parents=True, exist_ok=True)
svg.write_text(f'''<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="256" height="256" viewBox="0 0 256 256">
  <title>Orbis Stream</title>
  <defs>
    <linearGradient id="frame" x1="0" y1="0" x2="0" y2="1">
      <stop stop-color="#dce3eb"/><stop offset="1" stop-color="#8994a6"/>
    </linearGradient>
    <linearGradient id="stand" x1="0" y1="0" x2="0" y2="1">
      <stop stop-color="#6b778a"/><stop offset="1" stop-color="#c7cfdb"/>
    </linearGradient>
    <linearGradient id="screen" x1="0" y1="0" x2="1" y2="1">
      <stop stop-color="#171c31"/><stop offset="1" stop-color="#090d19"/>
    </linearGradient>
  </defs>
  <path d="M110 177h36l8 37h-52z" fill="url(#stand)" stroke="#667286" stroke-width="2"/>
  <path d="M87 214h82l12 13v7H75v-7z" fill="url(#frame)" stroke="#738095" stroke-width="2" stroke-linejoin="round"/>
  <rect x="17" y="25" width="222" height="166" rx="15" fill="url(#frame)" stroke="#566278" stroke-width="2"/>
  <rect x="26" y="34" width="204" height="140" rx="8" fill="url(#screen)" stroke="#465166" stroke-width="2"/>
  <image x="69" y="45" width="118" height="118" xlink:href="data:image/png;base64,{embedded}"/>
  <circle cx="128" cy="182" r="3" fill="#465166"/>
</svg>
''')
for size in (16, 24, 32, 48, 64, 128, 256, 512):
    png = icons / f"{size}x{size}/apps/orbis-stream.png"
    png.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["rsvg-convert", "--width", str(size), "--height", str(size),
                    "--output", str(png), str(svg)], check=True)
shutil.copyfile(icons / "256x256/apps/orbis-stream.png", resources / "app-icon.png")
