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
aliases.update({
    'Avvia': 'startLiveButton', 'Avvia da questo video': 'playlistStartFrom',
    'Dalla posizione': 'sceneResume', 'Dall’inizio': 'restartLiveIconLabel',
    'Apri regia': 'livePreviewTitle', 'Apri Canali': 'channelSettings',
    'Apri Video e audio': 'videoAndAudio', 'Aggiungi video…': 'addVideoFile',
    'Aggiungi sorgente': 'addToCanvas', 'Anteprima sorgente': 'preview',
    'Anteprima composizione': 'preview', 'Applica layout': 'selectLayout', 'Usa layout': 'selectLayout',
    'Bitrate audio (bit/s)': 'audioBitrate', 'Bitrate video (bit/s)': 'bitrate',
    'Canale abilitato': 'activeConfigurationLabel', 'Cartella video': 'playlistFolder',
    'Chiave di streaming': 'streamKey', 'Componi nuova scena…': 'composeScene',
    'Contenitore': 'videoFormat',
    'Controlli video e audio': 'videoAndAudio', 'Dalla libreria': 'overlays',
    'Descrizione': 'description', 'Dettagli diretta': 'seeDetails', 'Dirette': 'runningLives',
    'Disattivati': 'deactive', 'Attivi': 'active', 'Elimina video': 'deleteLiveRow',
    'Etichetta': 'sceneButtonName', 'Frame rate': 'frameRate', 'Includi nel mix': 'soundOn',
    'Intermezzo…': 'playSpot', 'Larghezza canvas': 'width', 'Altezza canvas': 'height',
    'Attesa del segnale': 'previewWaiting', 'Media': 'sceneMedia', 'Metti in onda': 'startStreaming',
    'Mixer audio': 'sourceVolumes', 'Modifica composizione…': 'composeScene',
    'Modifica scena…': 'composeScene', 'Nessuna diretta': 'noLiveRunning',
    'Nessuna sessione da recuperare': 'noLockedVideo', 'Nome canale': 'channelName',
    'Nome pubblico / ID trasmissione': 'channelName', 'Nuovo pulsante': 'sceneAdd',
    'Opzioni FFmpeg aggiuntive': 'additionalVideoOptions', 'Pagina piattaforma': 'liveOnPlatform',
    'Posizione': 'scenePlacement', 'Precedente': 'back', 'Successivo': 'next',
    'Profilo': 'videoSettings', 'Profilo video': 'videoSettings', 'Profilo video e audio': 'videoAndAudioSettings',
    'Pulisci storico': 'runCleanupNow', 'Pulizia automatica per canale': 'autoCleanup',
    'Pulizia dello storico': 'autoCleanup', 'Pulsante scena': 'sceneDeck', 'Qualità': 'encoderQuality',
    'Registra tasti': 'sceneKeySet', 'Premi una combinazione…': 'sceneKeyPress',
    'Riempi canvas': 'fillCanvas', 'Rileva dispositivi': 'refreshSources',
    'Rimuovi dalla libreria': 'overlayDelete', 'Risoluzione canvas': 'resolution',
    'Salva come layout': 'saveLayout', 'Salva impostazioni': 'save', 'Scegli file…': 'sceneMediaChoose',
    'Server RTMP/RTMPS': 'streamUrl',
    'Sovrapponi alla scena': 'sceneModeInScene', 'Scorciatoia': 'sceneKey',
    'Svuota canvas': 'blankLayout', 'Torna alla playlist': 'sceneResume',
    'Trasmetti playlist': 'startPlaylistButton', 'Video successivo': 'next',
    'Volume': 'sourceVolumes', 'Importa overlay…': 'uploadOverlay',
    'CPU': 'cpu', 'Spazio disponibile': 'diskFree', 'Informazioni su OrbisStream': 'about',
})

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
    text = path.read_text().split('/* Native workflow labels, from the shared language catalog. */', 1)[0].rstrip() + '\n'
    keys = {json.loads(match[1]) for match in re.finditer(r'^("(?:[^"\\]|\\.)*")\s*=', text, re.M)}
    additions = [(label, upstream[key]) for label, key in aliases.items() if label not in keys]
    if additions:
        text = text.rstrip() + '\n\n/* Native workflow labels, from the shared language catalog. */\n'
        text += ''.join(f'{json.dumps(label, ensure_ascii=False)} = {json.dumps(value, ensure_ascii=False)};\n' for label, value in sorted(additions))
        path.write_text(text)
        count += len(additions)
print(f'{count} native labels synchronized across 21 language catalogs')
