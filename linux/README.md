# OrbisStream Linux · C++20

Port Linux del progetto `lazy-stream-windows`, contenuto nella cartella `linux/` del repository macOS. Il motore e il server sono compilati in C++20; FFmpeg esegue composizione e codifica, SQLite conserva i dati. L'interfaccia usa il browser locale, senza framework JavaScript e senza runtime .NET, Swift o Rust.

Il confronto funzionale è in [docs/MIGRAZIONE.md](docs/MIGRAZIONE.md). La [mappa file per file](docs/PORTING_MAP.md) associa ciascuno dei **257 file Windows** al codice Linux o al lavoro mancante; l'[inventario](docs/INVENTARIO_WINDOWS.md) conserva hash, simboli e 61 route. Il [versionamento](docs/VERSIONAMENTO.md) collega binario, API e pacchetto Debian.

**Stato:** port eseguibile con test su Linux x86_64. La parità completa con Windows non è ancora certificata: i controlli aggiunti per Linux non sono tutti tradotti, il contratto HTTP è stato sostituito da `/api/*`, e la cattura/hardware e i servizi pubblici richiedono prove dedicate. Il documento di migrazione indica le differenze precise.

## Compilare e avviare

Su Debian/Ubuntu:

```sh
sudo apt install build-essential cmake pkg-config libboost-dev libjson-c-dev libsqlite3-dev ffmpeg curl xdg-utils
cd linux
bash scripts/build-linux.sh
./build/orbis-stream
```

Il programma apre il browser sull'indirizzo locale completo di token. La porta predefinita è 1200. Per un avvio senza apertura del browser:

```sh
./build/orbis-stream --no-browser --port 1200
```

Apri **l'indirizzo completo stampato nel terminale**. Il token crea una sessione locale; la pagina `/` da sola non abilita l'accesso.

Per creare l'installer Debian, dalla cartella `linux/`:

```sh
bash scripts/create-deb.sh
```

Il comando compila, esegue i test e produce `linux/build/packages/orbis-stream_<versione>_<arch>.deb`. Su Debian/Ubuntu puoi installarlo con `sudo apt install ./build/packages/orbis-stream_0.0.4_amd64.deb` se quella è la versione e architettura mostrate dal comando. Per generare anche l'archivio `.tar.gz`, usa `bash scripts/build-linux.sh --package`.

Il `.deb` installa l'app, il suo helper, le risorse e una voce nel menu delle applicazioni; FFmpeg è una dipendenza della distribuzione. Un pacchetto compilato su Ubuntu 24.04 richiede versioni compatibili di glibc e delle librerie. Ricompila sulla distribuzione di destinazione se necessario. Il codice non impone x86_64, ma ARM64 non è stato verificato.

## Utilizzo

1. **Canali:** aggiungi nome, piattaforma, server RTMP/RTMPS e chiave; i server iniziali sono quelli previsti da Windows, ma puoi inserire l'ingest del tuo account. Il nome canale viene usato anche per il link pubblico; nelle opzioni Linux puoi impostare un nome pubblico alternativo, cartella, scena e profilo. TikTok richiede una nuova chiave ad ogni avvio.
2. **Profili video:** imposta H.264, risoluzione, fps, bitrate, GOP in secondi, audio AAC, preset e bassa latenza. “Qualità Auto” esegue un benchmark x264. NVENC/QSV richiedono hardware e driver adeguati.
3. **Playlist:** importa una cartella locale o aggiungi percorsi assoluti. Abilita, riordina, associa un profilo al singolo video e imposta la posizione iniziale.
4. **Scene e layout:** aggiungi file, desktop X11, camera V4L2, microfono PulseAudio, slot e overlay. Trascina/ridimensiona; usa frecce, Shift e Canc. Puoi trascinare file dal desktop nel canvas: vengono copiati nella libreria locale. “Usa layout” crea una scena distinta dagli slot salvati.
5. **Overlay e pulsanti scena:** carica i media, scegli modalità a schermo intero o sovrapposta, posizione, durata e scorciatoia. I tasti funzionano quando il pannello di regia è aperto e chiedono conferma.
6. **Avvia diretta:** scegli canale/profilo/scena, loop e posizione iniziale; il countdown è annullabile.
7. **Regia:** controlla anteprima, bitrate di uscita, mixer 0–200%, mute, salto, cambio profilo, intermezzi e spot in coda. Il publisher FFmpeg resta aperto durante i cambi dei produttori; la sostituzione può introdurre una breve pausa di codifica.
8. **Storico:** consulta snapshot/esito, riavvia e cancella; esporta JSON senza chiavi e pulisci lo storico manualmente o per canale.
9. **Attività:** CPU, memoria, swap, rete, disco, uptime, sensori disponibili e processi FFmpeg. `—` indica il primo campione o un contatore assente.
10. **Manuale:** consulta le 21 versioni del manuale originale, con screenshot Windows. Stampa o salva in PDF dal browser.

## Dati e opzioni

| Opzione | Valore predefinito |
|---|---|
| `--data-dir DIR` / `LAZY_STREAM_DATA_DIR` | `$XDG_DATA_HOME/orbis-stream` o `~/.local/share/orbis-stream` |
| `--port N` / `LAZY_STREAM_PORT` | `1200` |
| `--no-browser` | apertura automatica con `xdg-open` |
| `--web-root DIR` | risorse installate oppure `linux/web` durante lo sviluppo |
| `FFMPEG_PATH` | `ffmpeg` da PATH |
| `FFPROBE_PATH` | `ffprobe` da PATH |

Il database è `stream.db`; `overlays/`, `scene-media/`, `proxies/`, `previews/`, `images/` e `logs/` sono nella cartella dati. La cartella dati è privata (`0700`), il database e i file importati sono `0600`. Le chiavi dei canali sono nel database locale, come nel progetto Windows; non sono cifrate con Secret Service. Export e storico escludono la chiave. L'importazione JSON sostituisce canali/profili/scene/pulsanti/video/preferenze in una transazione; conserva lo storico locale e richiede di reinserire le chiavi.

Il database Windows originale ha uno schema diverso: **non sostituire `stream.db` Linux con quello Windows**. La migrazione automatica del database originale non è inclusa.

I processi sono avviati con argomenti separati, senza shell. `orbis-stream-runner` deve restare accanto a `orbis-stream`: lega FFmpeg alla vita dell'applicazione anche in caso di crash. INT/TERM fermano le sessioni; dopo un'interruzione anomala lo storico viene recuperato come `INTERRUPTED`.

## Cattura Linux

- **Desktop:** `x11grab`, con DISPLAY come `:0.0`. Il portale desktop Wayland/PipeWire non è implementato; su Wayland usa una sorgente V4L2 virtuale alimentata da un'app compatibile.
- **Camera:** `/dev/videoN` via V4L2.
- **Microfono/audio desktop:** sorgenti PulseAudio, comprese le sorgenti monitor. PipeWire funziona attraverso `pipewire-pulse` e `pactl`.
- **Sensori:** `/sys/class/thermal`; statistiche NVIDIA tramite `nvidia-smi` se presente. Nessun valore GPU o temperatura viene inventato.

L'uscita RTMP usa FLV/H.264/AAC. HEVC, altri contenitori e VAAPI non sono opzioni operative di questo motore. `h264_nvenc` e `h264_qsv` sono disponibili come scelta, ma devono superare una prova sul computer reale.

## Verifica e risorse Windows

La formattazione usa **Prettier 3.9.10** per JavaScript, HTML e CSS e **clang-format** per C++ (verificato con 23.1.3). I commenti delle funzioni descrivono il comportamento e le ragioni delle scelte di implementazione. Le risorse originali Windows e gli inventari generati sono esclusi dalla formattazione.

Dal percorso `linux/`, con Node/npm e clang-format disponibili:

```sh
npm ci --ignore-scripts --no-audit --no-fund
npm run format
npm run format:check
```

```sh
ctest --test-dir build --output-on-failure
node tests/browser.mjs build/orbis-stream /tmp/orbis-linux-preview.png
```

Il secondo comando richiede Node 22+ e Google Chrome; usa un profilo temporaneo senza finestra. I test principali richiedono FFmpeg/FFprobe, usano cartelle temporanee e non pubblicano su piattaforme esterne. Verificano SQLite/export/import, parser FLV a blocchi, timestamp continui, server/token, playlist reale, audio/video, mixer, takeover/ripresa, RTMP locale, proxy, benchmark e riproduzione con Range. Il test browser verifica CRUD canali, trascinamento/salvataggio canvas e manuale.

Per aggiornare il censimento e le risorse dal checkout Windows:

```sh
python3 scripts/sync-windows.py --source ../../lazy-stream-windows
```

Le risorse Windows sono incluse e redistribuite con la licenza originale in `web/resources/LICENSE-windows.txt`. L'app usa la [licenza MIT del repository](../LICENSE); le librerie di sistema e FFmpeg conservano le proprie licenze.
