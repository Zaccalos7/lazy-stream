#!/usr/bin/env python3
"""Import the selectable codec labels/IDs from the Windows reference catalog."""
import json
import re
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = Path(sys.argv[1]) if len(sys.argv) > 1 else root.parent.parent / 'lazy-stream-windows/src/Orbis.Stream.Core/Domain/FfmpegCodecCatalog.cs'
text = source.read_text()
catalog = {}
for key in ('VideoCodecs', 'AudioCodecs'):
    body = text.split(key + ' =', 1)[1].split('];', 1)[0]
    catalog[key] = [{'id': int(number), 'label': label, 'name': name} for number, label, name in re.findall(r'new\((\d+), "([^"]+)", "([^"]+)"\)', body)]
body = text.split('PixelFormats = BuildPixelFormats(', 1)[1].split(');', 1)[0]
catalog['PixelFormats'] = [{'id': i, 'label': name.upper(), 'name': name} for i, name in enumerate(re.findall(r'"([^"]+)"', body))]
body = text.split('VideoCodecNames =', 1)[1].split('public static readonly', 1)[0]
catalog['VideoCodecNames'] = [{'label': f'Codec {vendor} ({name})', 'name': name} for vendor, names in re.findall(r'\("([^"]+)", \[([^\]]+)\]\)', body) for name in re.findall(r'"([^"]+)"', names)]
(root / 'web/resources/codec-catalog.json').write_text(json.dumps(catalog, ensure_ascii=False, indent=2) + '\n')
