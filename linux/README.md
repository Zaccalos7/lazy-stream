# OrbisStream Linux · C++20

Port Linux del progetto `lazy-stream-windows`, contenuto nella cartella `linux/` del repository macOS. Il motore e l’interfaccia sono compilati in **C++20 con Qt 6 Widgets, Qt Multimedia e Qt WebEngine**. L’avvio normale apre una finestra Linux nativa: controlli, dialoghi, canvas e player sono widget Qt, collegati direttamente al motore. FFmpeg esegue composizione e codifica; SQLite conserva i dati. Le pagine dell’app sono widget C++: non viene avviato un server HTTP né un browser esterno. Solo il player pubblico Twitch/YouTube nell’Anteprima usa un QWebEngineView, controllato da C++, con il frame FFmpeg disponibile in fallback.

Il confronto funzionale è in [docs/MIGRAZIONE.md](docs/MIGRAZIONE.md). La [mappa file per file](docs/PORTING_MAP.md) associa ciascuno dei **272 file Windows** al codice Linux o al lavoro mancante; l'[inventario](docs/INVENTARIO_WINDOWS.md) conserva hash, simboli e 62 route. Il [versionamento](docs/VERSIONAMENTO.md) collega binario, API e pacchetto Debian.

**Stato:** port eseguibile con test su Linux x86_64. La parità completa con Windows non è ancora certificata: la cattura/hardware e i servizi pubblici richiedono prove dedicate; alcuni comportamenti Windows restano parziali o mancanti. I test nativi verificano le schermate nelle 21 lingue originali, ma non certificano ogni funzione Windows. Il documento di migrazione indica le differenze precise.

## Compilare e avviare

È richiesto Qt 6.4 o successivo. Se non puoi installare i pacchetti di sviluppo a livello di sistema, `python3 scripts/prepare-webengine.py` scarica le dipendenze Qt WebEngine dai repository APT configurati e le estrae in `build/qt-webengine/`; CMake le rileva automaticamente. Questa preparazione non richiede sudo e non modifica il sistema. Il pacchetto Debian finale dichiara invece le dipendenze Qt WebEngine della distribuzione.

Su Debian/Ubuntu:

```sh
sudo apt install build-essential cmake pkg-config libboost-dev libjson-c-dev libsqlite3-dev ffmpeg curl gnupg libxcb1-dev gstreamer1.0-libav gstreamer1.0-plugins-good qt6-base-dev qt6-multimedia-dev qt6-webengine-dev qt6-image-formats-plugins qt6-translations-l10n fonts-noto-core fonts-noto-cjk
cd linux
python3 scripts/prepare-ffmpeg.py  # la build lo esegue se manca il relativo SDK
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

## Cambiare l'IP del server per usarlo in LAN

Sul computer che esegue OrbisStream, copia `.env.example` in `.env` e imposta **l'IP locale di quel computer**, per esempio:

```dotenv
LAZY_STREAM_HOST=192.168.1.50
LAZY_STREAM_PORT=1200
```

Su Linux puoi leggere gli indirizzi con `hostname -I`; scegli quello della rete domestica. L'indirizzo deve appartenere al server, non al telefono o al computer da cui vuoi collegarti.

Avvia il servizio web dalla cartella `linux/`:

```sh
./build/orbis-stream --headless --env-file .env
```

Se hai installato il pacchetto `.deb`, modifica `LAZY_STREAM_HOST` e `LAZY_STREAM_PORT` in **`/etc/orbis-stream/orbis-stream.env`**. Il pacchetto include il file e ne conserva le modifiche durante gli aggiornamenti. Per una configurazione personale senza modificare `/etc`, puoi invece conservare il file in `~/.config/orbis-stream/.env` (crea prima la cartella). Verrà letto automaticamente:

```sh
orbis-stream --headless
```

Dal telefono o da un altro computer collegato alla stessa LAN, apri **l'indirizzo completo stampato dal server**, per esempio `http://192.168.1.50:1200/?token=...`. Usa il token generato all'avvio. Le chiamate del browser usano automaticamente lo stesso server. Se il firewall del server blocca la connessione, consenti la porta TCP scelta dalla rete domestica.

In LAN tramite HTTP, per vedere la diretta pubblica Twitch usa **Apri la pagina della live**: il player incorporato di Twitch richiede HTTPS, con eccezione dello sviluppo locale su localhost/127.0.0.1. L'anteprima locale della composizione e i comandi di regia restano disponibili. Vedi i [requisiti ufficiali del player Twitch](https://dev.twitch.tv/docs/embed/).

**Dopo ogni modifica dell'IP o della porta nel file env, chiudi completamente e riavvia l'applicazione.** Il file viene letto solo all'avvio: ricaricare la pagina del browser non basta. Dopo il riavvio, usa il nuovo indirizzo completo stampato dal server, con il nuovo token. Chiudi anche un'eventuale istanza desktop aperta sulla stessa cartella dati prima di avviare `--headless`.

Senza un IP configurato il servizio ascolta su `127.0.0.1`, accessibile solo dal server. `LAZY_STREAM_HOST` configura il servizio web in modalità `--headless`; l'avvio normale continua ad aprire la finestra Qt. Il server accetta un IP concreto IPv4/IPv6, non un nome host o `0.0.0.0`.

Le opzioni della riga di comando hanno precedenza sulle variabili esportate nella shell. I file env sono letti, in ordine di precedenza: `--env-file`, `.env` nella cartella di avvio, `~/.config/orbis-stream/.env` (o `$XDG_CONFIG_HOME/orbis-stream/.env`), `/etc/orbis-stream/orbis-stream.env`. Le variabili già esportate prevalgono sui file. Sono supportati valori semplici e tra virgolette; il contenuto non viene eseguito come script shell.

Per creare l'installer Debian, dalla cartella `linux/`:

```sh
bash scripts/create-deb.sh
```

Il comando compila, esegue i test e produce `linux/build/packages/orbis-stream_<versione>_<arch>.deb`. Su Debian/Ubuntu puoi installarlo con `sudo apt install ./build/packages/orbis-stream_0.0.13_amd64.deb` se quella è la versione e architettura mostrate dal comando. Per generare anche l'archivio `.tar.gz`, usa `bash scripts/build-linux.sh --package`.

Il `.deb` installa l'app, il suo helper, le risorse e una voce nel menu delle applicazioni; FFmpeg/FFprobe 9.0.2 e le relative librerie sono inclusi in percorsi privati. Le librerie esterne degli encoder e Qt sono dipendenze APT; il FFmpeg della distribuzione resta disponibile come fallback esplicito. Un pacchetto compilato su Ubuntu 24.04 richiede versioni compatibili di glibc e delle librerie. Ricompila sulla distribuzione di destinazione se necessario.

Le build supportano `amd64` (x64) e `arm64` (AArch64), ciascuna con applicazione, FFmpeg e librerie della stessa architettura. Il pacchetto usa automaticamente il proprio FFmpeg privato; puoi forzare un altro eseguibile con `FFMPEG_PATH`. Le opzioni Intel QSV/NVIDIA della compilazione FFmpeg sono abilitate solo su x64; i codec software sono inclusi anche nella build ARM64.

Per distribuire entrambe le architetture, metti i due `.deb` e `scripts/install-deb.sh` nella stessa cartella, poi esegui:

```sh
bash install-deb.sh
```

Lo script rileva `dpkg --print-architecture`, verifica i metadati e installa con APT il pacchetto nativo più recente presente nella cartella. Se manca il pacchetto adatto, segnala l'errore. Puoi verificare la selezione senza installare con `bash install-deb.sh . --print-path`. Anche l'installazione diretta di un `.deb` viene controllata da Debian, che ne verifica l'architettura.

`ORBIS_BUILD_DIR` permette build separate, per esempio `ORBIS_BUILD_DIR="$PWD/build-arm64" bash scripts/create-deb.sh` su una macchina ARM64. Per una build isolata Ubuntu 24.04 è disponibile `scripts/build-container.sh`; per eseguirla su un PC di un'altra architettura occorre configurare prima l'emulazione QEMU/binfmt del sistema Docker.

## Utilizzo

1. **Canali:** aggiungi nome, piattaforma, server RTMP/RTMPS e chiave; i server iniziali sono quelli previsti da Windows, ma puoi inserire l'ingest del tuo account. Il nome canale viene usato anche per il link pubblico; la cartella si sceglie avviando una playlist e le regole di pulizia sono nello Storico.
2. **Profili video:** imposta H.264, risoluzione, fps, bitrate, GOP in secondi, audio AAC, preset e bassa latenza. “Qualità Auto” misura l’encoder alla risoluzione e al frame rate della live e conserva il risultato per la durata del processo. NVENC richiede hardware e driver adeguati.
3. **Playlist:** il wizard trasmette una cartella locale in ordine naturale. La libreria consente di aggiungere percorsi, abilitare video e impostare ordine, profilo e posizione iniziale.
4. **Scene e layout:** aggiungi file, desktop X11, camera V4L2, microfono PulseAudio, slot e overlay. Trascina/ridimensiona; usa frecce, Shift e Canc. Puoi trascinare file dal desktop nel canvas: gli overlay vengono copiati nella libreria locale; i video mantengono il percorso originale. “Usa layout” crea una scena distinta dagli slot salvati.
5. **Overlay e pulsanti scena:** carica i media, scegli modalità a schermo intero o sovrapposta, posizione, durata e scorciatoia. Aprire il pannello di regia arma i tasti per quella diretta anche in Live/Anteprima; chiedono conferma.
6. **Avvia Live:** scegli un profilo attivo, poi una configurazione canale attiva e componi la scena, nello stesso ordine Windows. **Avvia playlist** usa gli stessi pannelli e una cartella locale; sono disponibili ripetizione e countdown annullabile.
7. **Gestione Live:** tabella con filtri Stato/Canale/Piattaforma e dieci righe per pagina, come `MainLive.cshtml`. La riga mostra stato, titolo (con pillola della playlist/scena e provenienza), canale, data di avvio e tutte le azioni come icone. Dettagli apre i video della playlist o le sorgenti della scena; Riprendi usa video e timestamp interrotti, Ricomincia parte dall'inizio. Il profilo può essere collegato anche a una diretta già salvata. La ricerca testuale resta sulla pagina Storico.
8. **Regia:** controlla anteprima, bitrate di uscita, mixer 0–200%, mute, salto, cambio profilo, intermezzi e spot in coda. Il publisher resta aperto durante i cambi dei produttori, usando libavformat in C++ o il sender FFmpeg secondo la piattaforma; la sostituzione può introdurre una breve pausa di codifica.
9. **Storico:** filtra per canale, stato e titolo, consulta snapshot/esito, riavvia e cancella. Esporta JSON senza chiavi; la pulizia manuale o programmata mostra i conteggi e conserva le dirette attive.
10. **Attività:** CPU, memoria, swap, rete, disco, uptime, sensori disponibili e processi FFmpeg. `—` indica il primo campione o un contatore assente.
11. **Manuale:** consulta le 21 versioni del manuale originale, con screenshot Windows. Stampa o salva in PDF con i dialoghi Qt.

## Dati e opzioni

| Opzione | Valore predefinito |
|---|---|
| `--data-dir DIR` / `LAZY_STREAM_DATA_DIR` | `$XDG_DATA_HOME/orbis-stream` o `~/.local/share/orbis-stream` |
| `--port N` / `LAZY_STREAM_PORT` | `1200`, usata solo con `--headless` |
| `--host IP` / `LAZY_STREAM_HOST` | `127.0.0.1`; imposta l'IP del server per l'accesso LAN |
| `--env-file FILE` | file env esplicito, letto prima dei file automatici |
| `--headless` / `--no-browser` | disattiva la GUI e avvia il servizio API |
| `--web-root DIR` | risorse installate oppure `linux/web` durante lo sviluppo |
| `FFMPEG_PATH` | FFmpeg 9.0.2 privato; fallback a PATH solo senza SDK/installazione |
| `FFPROBE_PATH` | FFprobe accanto al FFmpeg scelto; fallback a PATH |

Il database è `stream.db`; `overlays/`, `scene-media/`, `proxies/`, `previews/`, `images/` e `logs/` sono nella cartella dati. La cartella dati è privata (`0700`), il database e i file importati sono `0600`. Le chiavi dei canali sono nel database locale, come nel progetto Windows; non sono cifrate con Secret Service. Export e storico escludono la chiave. L'importazione JSON sostituisce canali/profili/scene/pulsanti/video/preferenze in una transazione; conserva lo storico locale e richiede di reinserire le chiavi.

Nei **Dettagli** di una diretta sono visibili i log aggiornati durante l'esecuzione: `<id>.session.log` contiene comandi, PID, segmenti, primo output, errori con fase ed exit code e riconnessioni; `<id>.log` contiene la diagnostica del producer FFmpeg; `<id>.publisher.log` quella del publisher. Le chiavi e gli URL RTMP sono oscurati prima della scrittura su disco; i file sono privati (`0600`) e i tentativi restano nello stesso registro. `<id>.preview.log` registra ricerca della diretta pubblica, caricamento del player, errori console e uscita del renderer. Il pannello e `GET /api/logs/<id>` mostrano gli ultimi 64 KiB di ciascun file. FFmpeg analizza fino a 4 MiB/due secondi per raccogliere le intestazioni H.264/AAC prima di pubblicare, come il sender Windows, e conserva integralmente la chiave Facebook passando app e playpath separati. Il publisher segue timeout e riconnessioni della piattaforma (20/15 secondi per Twitch/Kick, 30/20 per le piattaforme con buffering); un fallimento durante la finalizzazione viene segnalato come errore.

Il database Windows originale ha uno schema diverso: **non sostituire `stream.db` Linux con quello Windows**. La migrazione automatica del database originale non è inclusa.

Il compositore ha un’anteprima JPEG a 2 fps generata dallo stesso grafo FFmpeg, senza codifica video o ascolto audio. I processi sono avviati con argomenti separati, senza shell. `orbis-stream-runner` deve restare accanto a `orbis-stream`: lega FFmpeg alla vita dell'applicazione anche in caso di crash. INT/TERM fermano le sessioni; dopo un'interruzione anomala lo storico viene recuperato come `INTERRUPTED`.

L’anteprima della diretta arriva fino a 30 fps, con lato maggiore limitato a 640 pixel e timer dell’immagine distinto dall’aggiornamento dei controlli. I file JPEG sono sostituiti atomicamente; la GUI conserva l’ultima immagine completa e decodifica solo le immagini nuove. Il primo video fissa il formato delle piattaforme che richiedono un’unica risoluzione: senza dimensioni nel profilo, un clip piccolo o verticale parte da 1080p, come nel riferimento Windows.

Il primo avvio crea otto preset attivi: standard e leggero per Twitch, YouTube, Kick e Facebook Gaming. I preset modificati vengono conservati. Il controllo YouTube prosegue in background, con 90 secondi di tolleranza e riscontri ripetuti. La GUI riceve notifiche dal motore; in headless `GET /updates?watch=rows,stats,live` offre SSE autenticato e ripresa con `Last-Event-ID`.

I file `application-AAAA-MM-GG.log` raccolgono startup, versione effettiva di FFmpeg, errori UI/Qt e fasi di invio. La coda asincrona è limitata a 10.000 voci e 8 MiB, con conservazione di tre giorni; `composition-preview.log` conserva gli errori FFmpeg dell’anteprima del canvas.

## Cattura Linux

- **Desktop:** `x11grab`, con DISPLAY come `:0.0`. Il portale desktop Wayland/PipeWire non è implementato; su Wayland usa una sorgente V4L2 virtuale alimentata da un'app compatibile.
- **Camera:** `/dev/videoN` via V4L2.
- **Microfono/audio desktop:** sorgenti PulseAudio, comprese le sorgenti monitor. PipeWire funziona attraverso `pipewire-pulse` e `pactl`.
- **Sensori:** `/sys/class/thermal`; statistiche NVIDIA tramite `nvidia-smi` se presente. Nessun valore GPU o temperatura viene inventato.

L'uscita RTMP usa FLV, comprese le estensioni FFmpeg 9 per HEVC, VP9 e AV1 verificate con decodifica completa in locale. L’ingest deve supportare i codec scelti; H.264/AAC rimane il formato dei preset delle piattaforme. Gli altri contenitori del catalogo non cambiano il relay FLV. NVENC, QSV e VAAPI sono selezionabili; la pipeline VAAPI carica i fotogrammi nel dispositivo, ma l’esecuzione degli encoder hardware richiede i relativi driver e una prova sul computer reale.

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

TikTok è escluso dal port C++ dalla versione 0.0.12. L’aggiornamento archivia le configurazioni dedicate nella tabella privata `retired_records` di `stream.db`, senza cancellare lo storico. Gli ID archiviati non sono riutilizzati; i profili verticali personalizzati restano disponibili. Le risorse Windows sono filtrate durante la sincronizzazione per mantenere questa esclusione.
