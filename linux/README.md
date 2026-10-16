# OrbisStream Linux · C++20

Port Linux del progetto `lazy-stream-windows`, contenuto nella cartella `linux/` del repository macOS. Il motore e l’interfaccia sono compilati in **C++20 con Qt 6 Widgets e Qt Multimedia**. L’avvio normale apre una finestra Linux nativa: controlli, dialoghi, canvas e player sono widget Qt, collegati direttamente al motore. FFmpeg esegue composizione e codifica; SQLite conserva i dati. Non vengono avviati browser, WebView o server HTTP in modalità grafica.

Il confronto funzionale è in [docs/MIGRAZIONE.md](docs/MIGRAZIONE.md). La [mappa file per file](docs/PORTING_MAP.md) associa ciascuno dei **257 file Windows** al codice Linux o al lavoro mancante; l'[inventario](docs/INVENTARIO_WINDOWS.md) conserva hash, simboli e 61 route. Il [versionamento](docs/VERSIONAMENTO.md) collega binario, API e pacchetto Debian.

**Stato:** port eseguibile con test su Linux x86_64. La parità completa con Windows non è ancora certificata: la cattura/hardware e i servizi pubblici richiedono prove dedicate; alcuni comportamenti Windows restano parziali o mancanti. I test nativi verificano le schermate nelle 21 lingue originali, ma non certificano ogni funzione Windows. Il documento di migrazione indica le differenze precise.

## Compilare e avviare

Su Debian/Ubuntu:

```sh
sudo apt install build-essential cmake pkg-config libboost-dev libjson-c-dev libsqlite3-dev ffmpeg curl gstreamer1.0-libav gstreamer1.0-plugins-good qt6-base-dev qt6-multimedia-dev qt6-translations-l10n fonts-noto-core fonts-noto-cjk
cd linux
bash scripts/build-linux.sh
./build/orbis-stream
```

Il programma apre l’app nativa. Dopo l’installazione è disponibile anche nel menu applicazioni come **OrbisStream**.

Chiudere la finestra principale termina l'applicazione, comprese le sessioni FFmpeg e i lavori in background. Se una diretta è attiva viene chiesta conferma. La chiusura non lascia l'app in esecuzione nell'area di notifica; i lavori già avviati vengono attesi prima di liberare la cartella dati.

Un secondo avvio sulla stessa cartella dati richiama la finestra già aperta tramite un socket locale, senza aprire un secondo database. Il gestore finestre può limitare il passaggio in primo piano; l'app richiede anche una notifica di attenzione. Per usare questa funzione, anche l'istanza già avviata deve essere aggiornata: una vecchia versione o un processo headless mantiene il blocco e viene segnalato senza interromperlo.

L’interfaccia desktop segue le schermate della guida Windows: superfici grigio scuro, accento blu, navigazione laterale e schede con titolo e descrizione divise in Streaming, Impostazioni e Sistema. In Gestione Live, **Avvia Live** è il pulsante principale e **Avvia playlist** è separato, con **Sblocca video** quando ce ne sono di bloccati. I filtri Stato/Canale/Piattaforma hanno etichette visibili, come nella pagina Windows; le righe mostrano stato e titolo come pillole, la data di avvio e tutte le azioni come icone sempre visibili (marchio della piattaforma, dettagli, collegamento profilo, deck delle scene, anteprima, spot, stop, avvio, riavvio ed eliminazione) secondo lo stato. Le schermate vuote spiegano cosa manca e i comandi della regia si abilitano solo con una live attiva.

Il wizard Live mostra prima i profili audio/video e poi i canali in schede con dettagli, fino a cinque colonne. Il compositore dispone sorgenti usate, sorgenti disponibili e overlay a sinistra, il canvas al centro e i livelli a destra. **Vai in live con questa scena** è sopra il canvas; **Indietro** riapre la scelta del canale e **Chiudi** annulla. La scena della diretta non viene aggiunta ai layout. Il tema è implementato in `src/native_theme.cpp` e le schermate in `src/native_window.cpp`, `src/native_dialogs.cpp` e `src/native_composer.cpp`, in C++20 con Qt 6 Widgets.

La lingua viene scelta dal sistema al primo avvio e può essere cambiata dal selettore nella barra laterale. La scelta viene salvata nel database. Sono disponibili tutte le **21 lingue del progetto Windows**: arabo, danese, tedesco, greco, inglese, spagnolo, filippino, francese, hindi, Hodor, italiano, giapponese, coreano, latino, olandese, portoghese, rumeno, russo, Klingon, turco e cinese. L’arabo usa la disposizione da destra a sinistra. Qt gestisce Unicode; ulteriori lingue si aggiungono tramite cataloghi `ui.<codice>.json` e `manual.<codice>.json`. Non si dichiarano traduzioni per lingue che non hanno ancora un catalogo.

Per test o automazioni è disponibile un servizio esplicito senza GUI:

```sh
./build/orbis-stream --headless --port 1200
```

`--no-browser` è un alias compatibile di `--headless`. Solo questa modalità avvia l’API locale e stampa il token. Le vecchie pagine HTML/CSS restano come riferimento del porting e per i test precedenti; non sono l’interfaccia dell’app nativa.

Per creare l'installer Debian, dalla cartella `linux/`:

```sh
bash scripts/create-deb.sh
```

Il comando compila, esegue i test e produce `linux/build/packages/orbis-stream_<versione>_<arch>.deb`. Su Debian/Ubuntu puoi installarlo con `sudo apt install ./build/packages/orbis-stream_0.0.5_amd64.deb` se quella è la versione e architettura mostrate dal comando. Per generare anche l'archivio `.tar.gz`, usa `bash scripts/build-linux.sh --package`.

Il `.deb` installa l'app, il suo helper, le risorse e una voce nel menu delle applicazioni; FFmpeg è una dipendenza della distribuzione. Un pacchetto compilato su Ubuntu 24.04 richiede versioni compatibili di glibc e delle librerie. Ricompila sulla distribuzione di destinazione se necessario. Il codice non impone x86_64, ma ARM64 non è stato verificato.

## Utilizzo

1. **Canali:** aggiungi nome, piattaforma, server RTMP/RTMPS e chiave; i server iniziali sono quelli previsti da Windows, ma puoi inserire l'ingest del tuo account. Il nome canale viene usato anche per il link pubblico; puoi scegliere la cartella e le regole di pulizia. TikTok richiede una nuova chiave ad ogni avvio.
2. **Profili video:** imposta H.264, risoluzione, fps, bitrate, GOP in secondi, audio AAC, preset e bassa latenza. “Qualità Auto” esegue un benchmark x264. NVENC richiede hardware e driver adeguati.
3. **Playlist:** il wizard trasmette una cartella locale in ordine naturale. La libreria consente di aggiungere percorsi, abilitare video e impostare ordine, profilo e posizione iniziale.
4. **Scene e layout:** aggiungi file, desktop X11, camera V4L2, microfono PulseAudio, slot e overlay. Trascina/ridimensiona; usa frecce, Shift e Canc. Puoi trascinare file dal desktop nel canvas: gli overlay vengono copiati nella libreria locale; i video mantengono il percorso originale. “Usa layout” crea una scena distinta dagli slot salvati.
5. **Overlay e pulsanti scena:** carica i media, scegli modalità a schermo intero o sovrapposta, posizione, durata e scorciatoia. Aprire il pannello di regia arma i tasti per quella diretta anche in Live/Anteprima; chiedono conferma.
6. **Avvia Live:** scegli un profilo attivo, poi una configurazione canale attiva e componi la scena, nello stesso ordine Windows. **Avvia playlist** usa gli stessi pannelli e una cartella locale; sono disponibili ripetizione e countdown annullabile.
7. **Gestione Live:** tabella con filtri Stato/Canale/Piattaforma e dieci righe per pagina, come `MainLive.cshtml`. La riga mostra stato, titolo (con pillola della playlist/scena e provenienza), canale, data di avvio e tutte le azioni come icone. Dettagli apre i video della playlist o le sorgenti della scena; Riprendi usa video e timestamp interrotti, Ricomincia parte dall'inizio. Il profilo può essere collegato anche a una diretta già salvata. La ricerca testuale resta sulla pagina Storico.
8. **Regia:** controlla anteprima, bitrate di uscita, mixer 0–200%, mute, salto, cambio profilo, intermezzi e spot in coda. Il publisher FFmpeg resta aperto durante i cambi dei produttori; la sostituzione può introdurre una breve pausa di codifica.
9. **Storico:** filtra per canale, stato e titolo, consulta snapshot/esito, riavvia e cancella. Esporta JSON senza chiavi; la pulizia manuale o programmata mostra i conteggi e conserva le dirette attive.
10. **Attività:** CPU, memoria, swap, rete, disco, uptime, sensori disponibili e processi FFmpeg. `—` indica il primo campione o un contatore assente.
11. **Manuale:** consulta le 21 versioni del manuale originale, con screenshot Windows. Stampa o salva in PDF con i dialoghi Qt.

## Dati e opzioni

| Opzione | Valore predefinito |
|---|---|
| `--data-dir DIR` / `LAZY_STREAM_DATA_DIR` | `$XDG_DATA_HOME/orbis-stream` o `~/.local/share/orbis-stream` |
| `--port N` / `LAZY_STREAM_PORT` | `1200`, usata solo con `--headless` |
| `--headless` / `--no-browser` | disattiva la GUI e avvia il servizio API |
| `--web-root DIR` | risorse installate oppure `linux/web` durante lo sviluppo |
| `FFMPEG_PATH` | `ffmpeg` da PATH |
| `FFPROBE_PATH` | `ffprobe` da PATH |

Il database è `stream.db`; `overlays/`, `scene-media/`, `proxies/`, `previews/`, `images/` e `logs/` sono nella cartella dati. La cartella dati è privata (`0700`), il database e i file importati sono `0600`. Le chiavi dei canali sono nel database locale, come nel progetto Windows; non sono cifrate con Secret Service. Export e storico escludono la chiave. L'importazione JSON sostituisce canali/profili/scene/pulsanti/video/preferenze in una transazione; conserva lo storico locale e richiede di reinserire le chiavi.

Nei **Dettagli** di una diretta sono visibili i log aggiornati durante l'esecuzione: `<id>.session.log` contiene comandi, PID, segmenti, primo output, errori con fase ed exit code e riconnessioni; `<id>.log` contiene la diagnostica del producer FFmpeg; `<id>.publisher.log` quella del publisher. Le chiavi e gli URL RTMP sono oscurati prima della scrittura su disco; i file sono privati (`0600`) e i tentativi restano nello stesso registro. Il pannello e `GET /api/logs/<id>` mostrano gli ultimi 64 KiB di ciascun file. FFmpeg analizza fino a 4 MiB/due secondi per raccogliere le intestazioni H.264/AAC prima di pubblicare, come il sender Windows, e conserva integralmente la chiave Facebook passando app e playpath separati. Avvio e progresso del publisher hanno un watchdog di 30 secondi; un fallimento durante la finalizzazione viene segnalato come errore.

Il database Windows originale ha uno schema diverso: **non sostituire `stream.db` Linux con quello Windows**. La migrazione automatica del database originale non è inclusa.

I processi sono avviati con argomenti separati, senza shell. `orbis-stream-runner` deve restare accanto a `orbis-stream`: lega FFmpeg alla vita dell'applicazione anche in caso di crash. INT/TERM fermano le sessioni; dopo un'interruzione anomala lo storico viene recuperato come `INTERRUPTED`.

## Cattura Linux

- **Desktop:** `x11grab`, con DISPLAY come `:0.0`. Il portale desktop Wayland/PipeWire non è implementato; su Wayland usa una sorgente V4L2 virtuale alimentata da un'app compatibile.
- **Camera:** `/dev/videoN` via V4L2.
- **Microfono/audio desktop:** sorgenti PulseAudio, comprese le sorgenti monitor. PipeWire funziona attraverso `pipewire-pulse` e `pactl`.
- **Sensori:** `/sys/class/thermal`; statistiche NVIDIA tramite `nvidia-smi` se presente. Nessun valore GPU o temperatura viene inventato.

L'uscita RTMP usa FLV/H.264/AAC. HEVC, altri contenitori e VAAPI non sono opzioni operative di questo motore. `h264_nvenc` è disponibile nel dialogo Qt; `h264_qsv` è accettato dal motore tramite un profilo importato, ma entrambi, ma devono superare una prova sul computer reale.

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
# Solo per controllare il vecchio client di riferimento:
node tests/browser.mjs build/orbis-stream /tmp/orbis-linux-preview.png
```

I test `native-gui` usano Qt offscreen, attraversano i dialoghi nativi e avviano una playlist reale su un ingest RTMP locale. `native-launch` verifica che l’avvio predefinito non ascolti una porta HTTP e termini correttamente. La prova del pacchetto estratto ripete avvio Qt e streaming.

Il secondo comando richiede Node 22+ e Google Chrome; usa un profilo temporaneo senza finestra. I test principali richiedono FFmpeg/FFprobe, usano cartelle temporanee e un ingest RTMP locale. Verificano SQLite/export/import, parser FLV a blocchi, timestamp continui, server/token, playlist reale, audio/video, mixer, takeover/ripresa, proxy, benchmark e riproduzione con Range. Il browser verifica le pagine Windows portate, i wizard, avvio playlist via RTMP, countdown annullabile, storico/filtri/paginazione, pulizia, download dell'export, salvataggio canvas e manuale.

Per aggiornare il censimento e le risorse dal checkout Windows:

```sh
python3 scripts/sync-windows.py --source ../../lazy-stream-windows
```

Le risorse Windows sono incluse e redistribuite con la licenza originale in `web/resources/LICENSE-windows.txt`. L'app usa la [licenza MIT del repository](../LICENSE); le librerie di sistema e FFmpeg conservano le proprie licenze.
