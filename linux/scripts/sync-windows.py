#!/usr/bin/env python3
"""Copy redistributable resources and inventory the actual Windows source tree."""
import argparse
import hashlib
import json
import pathlib
import re
import shutil

parser=argparse.ArgumentParser()
parser.add_argument('--source',type=pathlib.Path,default=pathlib.Path(__file__).resolve().parents[3]/'lazy-stream-windows')
parser.add_argument("--inventory-only", action="store_true", help="Refresh source hashes without replacing Linux resources")
args=parser.parse_args()
source=args.source.resolve()
root=pathlib.Path(__file__).resolve().parents[1]
core=source/'src/Orbis.Stream.Core'
if not core.is_dir():
    parser.error('Cartella Orbis.Stream.Core non trovata')
resources=root/'web/resources'
if not args.inventory_only:
    for name,folder in [('ui',core/'Ui'),('messages',core/'Messages'),('flags',source/'src/Orbis.Stream.App/wwwroot/flags'),('manual',source/'src/Orbis.Stream.App/wwwroot/manual')]:
        (resources/name).mkdir(parents=True,exist_ok=True)
        shutil.copytree(folder,resources/name,dirs_exist_ok=True)
    shutil.copyfile(source/'LICENSE',resources/'LICENSE-windows.txt')
inventory=[]
routes=[]
for path in sorted(source.rglob('*')):
    if not path.is_file() or any(x in path.parts for x in ['.git','bin','obj','artifacts']):
        continue
    if path.suffix not in ['.cs','.cshtml','.css','.js','.json','.properties','.csproj','.wxs','.nsi','.ps1','.sh'] and path.name not in ['README.md','LICENSE']:
        continue
    raw=path.read_bytes()
    text=raw.decode('utf-8',errors='replace')
    entry={'path':str(path.relative_to(source)),'sha256':hashlib.sha256(raw).hexdigest(),'bytes':len(raw)}
    if path.suffix=='.cs':
        entry['types']=re.findall(r'\b(?:class|enum|record|interface)\s+(\w+)',text)
        entry['properties']=re.findall(r'public\s+[\w<>?,\[\].]+\s+(\w+)\s*\{\s*get',text)
        entry['methods']=sorted(set(re.findall(r'public\s+(?:static\s+)?(?:async\s+)?[\w<>?,\[\].]+\s+(\w+)\s*\(',text)))
    if path.name=='OrbisEndpoints.cs':
        group=''
        for line in text.splitlines():
            m=re.search(r'var group = app.MapGroup\("([^"]+)"\)',line)
            if m:group=m[1]
            m=re.search(r'(group|app)\.Map(Get|Post|Put|Delete)\("([^"]*)"',line)
            if m:routes.append({'method':m[2].upper(),'path':(group if m[1]=='group' else '')+m[3]})
        # MapOverlays receives the /scene group from its caller.
        for route in routes:
            if route['path'].startswith('/overlays'):
                route['path']='/scene'+route['path']
    inventory.append(entry)
(root/'docs').mkdir(exist_ok=True)
(root/'docs/windows-inventory.json').write_text(json.dumps({'source':'lazy-stream-windows','files':inventory,'routes':routes},ensure_ascii=False,indent=2)+'\n')
lines=['# Inventario dei sorgenti Windows','',f'{len(inventory)} file censiti, {len(routes)} route API. Snapshot SHA-256 disponibile in `windows-inventory.json`.','',
       'Il censimento riguarda il checkout locale di lazy-stream-windows; non deriva dalle funzioni aggiunte soltanto alla versione macOS. La parità funzionale è descritta in [MIGRAZIONE.md](MIGRAZIONE.md).','',
       '## Route API Windows','', '| Metodo | Percorso originale |','|---|---|']
lines += [f"| {r['method']} | `{r['path']}` |" for r in routes]
lines += ['', '## File, tipi, proprietà e metodi pubblici', '']
for entry in inventory:
    lines += [f"### `{entry['path']}`", '']
    for label,key in [('Tipi','types'),('Proprietà','properties'),('Metodi pubblici','methods')]:
        if entry.get(key):lines += [label+': '+', '.join('`'+v+'`' for v in entry[key])+'.','']
    lines += ['SHA-256: `'+entry['sha256']+'`.','']
(root/'docs/INVENTARIO_WINDOWS.md').write_text('\n'.join(lines)+'\n')
print(f'{len(inventory)} file, {len(routes)} API; ' + ('solo inventario aggiornato' if args.inventory_only else f'risorse copiate in {resources}'))
