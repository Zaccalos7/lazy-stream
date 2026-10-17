#!/usr/bin/env python3
"""Add native navigation aliases using the translations maintained by Windows."""
import argparse
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source', type=Path, default=ROOT.parent / 'lazy-stream-windows/src/Orbis.Stream.Core/Ui')
args = parser.parse_args()
italian = json.loads((args.source / 'ui.it.json').read_text())
aliases = {
    'Live / Anteprima': 'livePreviewTitle', 'Riprendi': 'sceneResume',
    'Riprendi programma': 'sceneResume', 'Riprendi dalla posizione': 'sceneResume',
    'Ricomincia': 'restartLiveIconLabel', 'Ricomincia dall’inizio': 'restartLiveIconLabel',
    'Dettagli': 'seeDetails', 'Apri segnalazione': 'reportOpenIssue',
    'Indice': 'manualToc', 'Stampa / Salva PDF': 'manualPrint',
    'Configura pulsanti scena': 'sceneDeck', 'Termina': 'stopLiveIconLabel',
}
# Exact Italian labels already in the upstream bundle need no new translation.
sources = '\n'.join(path.read_text() for path in (ROOT / 'Sources/OrbisMac').glob('*.swift'))
for key, value in italian.items():
    if json.dumps(value, ensure_ascii=False) in sources:
        aliases.setdefault(value, key)
count = 0
for folder in sorted((ROOT / 'Sources/OrbisMac/Resources').glob('*.lproj')):
    language = folder.stem
    upstream = json.loads((args.source / f'ui.{language}.json').read_text())
    path = folder / 'Localizable.strings'
    text = path.read_text()
    keys = {json.loads(match[1]) for match in re.finditer(r'^("(?:[^"\\]|\\.)*")\s*=', text, re.M)}
    additions = [(label, upstream[key]) for label, key in aliases.items() if label not in keys]
    if additions:
        text = text.rstrip() + '\n\n/* Native workflow labels, from the shared language catalog. */\n'
        text += ''.join(f'{json.dumps(label, ensure_ascii=False)} = {json.dumps(value, ensure_ascii=False)};\n' for label, value in sorted(additions))
        path.write_text(text)
        count += len(additions)
print(f'{count} native labels added across 21 language catalogs')
