# Migrazione Windows → Linux: elenco punto per punto

**Interfaccia corrente (0.0.11):** C++/Qt Widgets nativa. Le righe V storiche possono riferirsi ai test del motore o al precedente client di riferimento; non attestano automaticamente parità del nuovo front end. La verifica Qt e le differenze sono in [INTERFACCIA_NATIVA.md](INTERFACCIA_NATIVA.md). Tutti i 21 cataloghi Windows sono disponibili nella GUI.

Riferimento: checkout locale `lazy-stream-windows`, 16 ottobre 2026. Destinazione: `lazy-stream-mac/linux`, C++20 + Qt 6 Widgets + FFmpeg + SQLite. Questo elenco parte dal progetto Windows, non dalle aggiunte specifiche di macOS.

L'[inventario completo](INVENTARIO_WINDOWS.md) riporta 272 file, 62 route API, tipi, proprietà, metodi pubblici e hash SHA-256. È riproducibile con `scripts/sync-windows.py`; [windows-inventory.json](windows-inventory.json) contiene i dati per strumenti automatici.

## Significato degli stati

- **V:** verificato dai test Linux con Qt, server, FFmpeg o client di riferimento reali.
- **P:** codice presente; la singola funzione non ha una prova dedicata oppure richiede hardware/servizi esterni.
- **D:** equivalente Linux con comportamento o architettura differente.
- **M:** parità ancora mancante. Non considerare queste voci completate.

## Canali e piattaforme

Originali: `Domain/Entities.cs`, `Services/SettingService.cs`, `Streaming/StreamPlatform.cs`, `Streaming/LiveStreamKeys.cs`, `Pages/MainChannelSetting.cshtml`.

| ID | Funzione Windows | Linux | Stato |
|---|---|---|---|
| W001 | Elenco canali e ricerca | `web/app.js`, pagina Canali | V |
| W002 | Creazione e modifica | form e SQLite in `core.cpp` | V |
| W003 | Eliminazione | API e conferma | P |
| W004 | Nome e descrizione | campi canale | V |
| W005 | URL RTMP/RTMPS e chiave | validazione e publisher FFmpeg | V RTMP; P RTMPS |
| W006 | Piattaforma della configurazione e nome del canale | `platformStreamName` salva la piattaforma come su Windows; il player usa `channelName` o un handle Linux precedente | V link Twitch e compatibilità DB; P altre piattaforme |
| W007 | Attivazione/disattivazione canale | filtro stato e pulsante nel client, `isActive` persistito e controllato all'avvio | V toggle browser; P blocco avvio dedicato |
| W008 | Cartella locale associata | scansione automatica o playlist importata | V |
| W009 | Scena associata | `scenePkid` | P |
| W010 | Profilo associato | `profileId` | V |
| W011 | Twitch | ingest RTMP e player pubblico | P, serve account reale |
| W012 | YouTube | ingest RTMP, ricerca live e player | P, serve account reale |
| W013 | Kick | ingest configurabile e pagina pubblica | P, serve account reale |
| W014 | Facebook | ingest configurabile e pagina pubblica | P, serve account reale |
| W015 | TikTok | `live_stream_keys.cpp` riconosce nome pubblico o domini ingest regionali, salva un segnaposto e richiede una chiave nuova ad ogni diretta | V per riconoscimento, persistenza e rifiuto chiave mancante; P per piattaforma reale |
| W016 | TikTok verticale | orientamento del profilo e fitting con bande | P |
| W017 | RTMP personalizzato | nessun vincolo sul provider | V con ingest locale |
| W018 | Regole di pulizia per canale | mesi, intervallo e ultimo controllo persistito | P |
| W019 | Scelta nativo RTMP / FFmpeg sender | API libavformat in C++ senza secondo processo, sender per le piattaforme con buffering e selezione YouTube | V trasporti locali e decodifica completa; P RTMPS reale |
| W020 | Normalizzazioni speciali URL/chiave YouTube | `stream_platform.cpp` converte schema e host ingest RTMP YouTube in RTMPS conservando chiave, porta e percorso | V per primary/backup e host estranei; altre regole da confrontare |

## Profili video e audio

Originali: `Domain/VideoSetting*.cs`, `Domain/FfmpegCodecCatalog.cs`, `Services/VideoSettingService.cs`, `Services/EncoderTuningService.cs`, `Streaming/FfmpegCommandBuilder.cs`.

| ID | Funzione Windows | Linux | Stato |
|---|---|---|---|
| W021 | Elenco e ricerca profili | `MainVideoSetting.html`: i seed sono template, i profili personalizzati compaiono nelle righe | V browser per elenco e ricerca di base |
| W022 | Creazione, modifica, duplicazione | form e API SQLite | V creazione; P modifica/duplicazione |
| W023 | Eliminazione e riferimenti | rifiuto se associato ad un canale | P |
| W024 | Preset per piattaforma | Dieci seed standard/leggeri con valori Windows attuali; upgrade dei template Linux riconoscibili, senza sovrascrivere modifiche | V nuovi seed e persistenza; P upgrade su database utente |
| W025 | Attivo/predefinito per piattaforma | Seed attivi, flag predefinito solo sui cinque standard, selezione e toggle Qt; default della piattaforma nel server | V wizard Qt e selezione API |
| W026 | Elenco encoder FFmpeg installati | comando `-encoders` | P |
| W027 | H.264 software | libx264 | V |
| W028 | Accelerazione hardware | NVENC/QSV, da verificare con driver reali | P |
| W029 | Encoder/codec extra del catalogo Windows | Cataloghi selezionabili e risoluzione tramite libavcodec; build con x265, AOM/SVT-AV1, libvpx e percorso VAAPI con upload | P combinazioni e hardware; vedi test delivery |
| W030 | Bitrate video | kbps e rate control | V |
| W031 | Audio | AAC stereo 48 kHz, bitrate configurabile | V |
| W032 | Catalogo codec audio/formati pixel/contenitori | Scelte Windows esposte, codec audio e pixel risolti dal motore; il relay delle live usa FLV | D per i contenitori non FLV e i codec incompatibili con RTMP; P combinazioni extra |
| W033 | Risoluzione | dimensioni pari, fitting della scena | V su profilo test |
| W034 | Frame rate | 1–120 fps | V su profilo test |
| W035 | GOP in secondi | convertito in numero di frame | V su profilo test |
| W036 | Risoluzione/FPS mantieni sorgente | Valori null risolti dalla scena/media; prima uscita mantenuta sulle piattaforme che richiedono formato uniforme | V builder e consegna con sorgenti di dimensione/FPS diversi |
| W037 | Preset x264 | selezione manuale | V |
| W038 | Tune/bassa latenza | `zerolatency` quando abilitato | V |
| W039 | Qualità Auto | benchmark x264 a risoluzione/fps del profilo | V |
| W040 | Qualità Leggera/Bilanciata/Alta | Preset x264/x265, NVENC e QSV come riferimento; Auto misura risoluzione e FPS effettivi | V selezione preset; P hardware e confronto del benchmark |
| W041 | Bitrate adattivo | Finestre, deriva, throughput e backlog; riduzione limitata e recupero, senza confondere encoder lento e rete lenta | V simulazioni deterministiche; P rete fisica degradata |
| W042 | Opzioni FFmpeg aggiuntive | Opzioni specifiche verificate tramite AVOption dell’encoder, preset e alias NVENC 9, parametri x264 uniti al CBR | V opzioni e protezione input/output; P ogni encoder |
| W043 | Rifiuto opzioni che cambiano input/output | opzioni non consentite rifiutate | P |
| W044 | Profilo per singolo video | `videoProfiles` durante la playlist | P |
| W045 | Modifica parametri in diretta | nuovo produttore sul publisher persistente | P |

## Avvio e playlist

Originali: `Services/StreamingService.cs`, `Services/FfmpegVideoPlaylistStreamer.cs`, `Services/NaturalFileNameComparer.cs`, `Services/SceneRows.cs`, `Pages/_StartWizard*.cshtml`, `Pages/_PlaylistVideos.cshtml`.

| ID | Funzione Windows | Linux | Stato |
|---|---|---|---|
| W046 | Procedura di avvio | `_StartWizardSetting.html` → `_StartWizardConfiguration.html` → compositore; avvio playlist distinto con cartella, solo profili e canali attivi | V Chrome con scena e playlist su ingest RTMP locale, Indietro e chiave TikTok |
| W047 | Countdown annullabile | conto alla rovescia nella playlist, generazione del dialogo controllata per impedire un avvio dopo l'annullamento | V Chrome |
| W048 | Avvio da cartella | elenco video riconosciuti | V |
| W049 | Aggiunta singolo file | percorso assoluto con FFprobe | P |
| W050 | Ordinamento naturale | confronto numerico senza overflow | V |
| W051 | Esclusione cartelle e duplicati | scansione solo file media e controllo per canale | P |
| W052 | Riordino e rimozione | pulsanti su/giù e delete | P |
| W053 | Abilitazione per video | checkbox, esclusione all'avvio | P |
| W054 | Ripresa da timestamp | `startIndex` e `offsetSeconds`, indice persistito; replay, ricomincia e riparti da un video nei dettagli | V motore FFmpeg; P controlli browser |
| W055 | Ripeti playlist o termina | loop e stato ENDED | V |
| W056 | Dirette concorrenti | sessione/processi/relay per diretta | P, test attuale sequenziale |
| W057 | Blocco doppio avvio canale | controllo in `Studio::start` | P |
| W058 | Successivo e salto ad indice | comandi next/jump | P |
| W059 | Stop singolo e generale | arresto dei gruppi FFmpeg, stato STOPPED per stop manuale distinto da ENDED | V singolo; P generale |
| W060 | Riconnessione e watchdog | Timeout di consegna per piattaforma; 5 tentativi YouTube/Facebook e 3 sugli altri percorsi; I/O cancellabile | V errore/rifiuto/handshake sospeso/stop; P riconnessioni su servizi pubblici |
| W061 | Recupero dopo crash | record INTERRUPTED, helper parent-death | P |

## Scene, sorgenti, layout e overlay

Originali: `Services/SceneService.cs`, `Services/SourceCatalogService.cs`, `Services/SourceSnapshotService.cs`, `Services/MediaProxyService.cs`, `Services/OverlayLibrary.cs`, `Streaming/OverlayMedia.cs`, `Pages/_SceneComposer.cshtml`.

| ID | Funzione Windows | Linux | Stato |
|---|---|---|---|
| W062 | Elenco/salvataggio/duplicazione/eliminazione scene | pagina Scene e layout | V salvataggio; P altre azioni |
| W063 | Nome, descrizione e canvas | dati scena | V |
| W064 | Layout con slot vuoti e overlay conservati | `scene_service.cpp`: almeno uno slot, target puliti solo per non-overlay, overlay verificati nella libreria | V regole di salvataggio; P editor completo |
| W065 | Creazione scena distinta dal layout | “Usa layout”, copia con nuovo ID; SceneService duplica scene già in onda o se cambia il tipo | V copia su modifica in storico; P flusso UI completo |
| W066 | Posizione, dimensioni e ordine dei layer | compositor e filter graph | V trascinamento; P resize/layer |
| W067 | Posizionamento del canvas | Coordinate, trascinamento e frecce; il riferimento attuale non espone un pulsante Centra separato | V interazioni Qt di base |
| W068 | Comandi dei livelli del riferimento attuale | Avanti/indietro, riempi canvas, rimozione e ordine dei livelli come composer.js | V controlli e geometria; P confronto di tutte le sequenze |
| W069 | Scorciatoie frecce/Shift/Canc | eventi tastiera del canvas | P |
| W070 | Drag-and-drop dal desktop | upload nella libreria scena | P |
| W071 | Sorgente file | FFmpeg e FFprobe | V |
| W072 | Acquisizione schermo | x11grab, DISPLAY selezionato | P, richiede sessione X11 |
| W073 | Selezione finestra/schermo specifico | Catalogo XCB di desktop e finestre, monitor RandR, target distinti e input FFmpeg condiviso con snapshot | V parsing e limiti input; P desktop/hardware reali |
| W074 | Acquisizione camera | V4L2 `/dev/videoN` | P |
| W075 | Microfono e audio desktop | PulseAudio/PipeWire Pulse, sorgenti monitor | P |
| W076 | Catalogo sorgenti disponibili | `/dev`, `/sys`, DISPLAY e pactl | P |
| W077 | Thumbnail file/camera/schermo | `/api/snapshot`, JPEG | V file; P hardware |
| W078 | Probe risoluzione/codec/durata | FFprobe JSON | V |
| W079 | Proxy media leggeri | job FFmpeg cancellabile, cache MP4 e progresso | V |
| W080 | Libreria overlay | upload, elenco e cancellazione; rifiuto di overlay fuori libreria o ancora referenziati | V controllo dei riferimenti; P upload/media avanzati |
| W081 | PNG/JPEG statici | Decodifica singola e ripetizione dell’ultimo fotogramma nel filtro; nessun loop del decoder | V classificazione e anteprima reale PNG |
| W082 | GIF/WebP animati e WebM alpha | Classificazione FFprobe, libvpx per VP8/VP9 alpha, WebP ANIM infinito in copia privata e ignore_loop=0 | V WebP reale: colori, alpha e tre cicli; P combinazioni GIF/WebM |
| W083 | Audio separato per sorgente | volume 0–200% e mute, `amix` | V su singola sorgente; P multi-sorgente |
| W084 | Silenzio per sorgenti senza audio | `anullsrc` | P |
| W085 | Anteprima composizione live e compositore | Live fino a 30 fps e 640 pixel sul lato maggiore; CompositionPreview a 2 fps usa lo stesso grafo senza H.264/audio in ascolto nel canvas | V fotogrammi reali e lifecycle; P dispositivi di cattura |
| W086 | Scena di ogni live conservata nello storico | snapshot del request, distinto dal layout; scene già usate non sovrascritte o cancellate | V vincolo di modifica/cancellazione; P flusso completo di riavvio |
| W087 | Cattura Wayland | portale PipeWire assente | M; funzione Linux aggiuntiva |

## Pulsanti scena, intermezzi e spot

Originali: `Domain/SceneButton.cs`, `Services/SceneButtonService.cs`, `Services/LiveTakeovers.cs`, `Streaming/FlvPacedRelay.cs`, `Pages/_SceneDeck.cshtml`.

| ID | Funzione Windows | Linux | Stato |
|---|---|---|---|
| W088 | Sei pulsanti condivisi | posizioni 0–5, label e media | P |
| W089 | Upload media pulsanti | libreria `scene-media` | P |
| W090 | Video a schermo intero con ritorno automatico | takeover del produttore e resume | V |
| W091 | Immagine/animazione fino a ripresa | loop e comando resume | P |
| W092 | Overlay nella scena e durata | layer aggiunto, timer e programma che avanza | P |
| W093 | Posizioni predefinite e rettangolo personalizzato | placement/custom e x/y/width/height | P |
| W094 | Ripresa dal punto interrotto | offset del programma separato dal takeover | V |
| W095 | Scorciatoie confermate soltanto nel pannello attivo | deckActive e KeyboardEvent.code | P |
| W096 | Hotkey riservate e validazione Windows | pattern, blacklist browser e controllo duplicati | P |
| W097 | Spot in coda al termine del video | coda per sessione | P |
| W098 | Spot immediato su un programma immagine | takeover immediato se il programma è un'immagine | P |
| W099 | RTMP persistente durante cambi | publisher unico e parser FLV C++ | V |
| W100 | Ricomposizione tag ricevuti a blocchi | buffer e validazione dimensioni/tag | V |
| W101 | Timestamp continui e pacing | timeline unica tra produttori | V |

## Anteprima, piattaforme, storico e attività

Originali: `Services/LivePreviewService.cs`, `Streaming/LivePreviewFrames.cs`, `Streaming/LivePlatformEmbeds.cs`, `Services/YouTubeAirWatch.cs`, `Services/LiveHistoryCleanupService.cs`, `Services/SystemInfoService.cs`, `SystemInfo/SystemInfoProviders.cs`.

| ID | Funzione Windows | Linux | Stato |
|---|---|---|---|
| W102 | Selezione della diretta osservata | `MainPreview.html`: selettore delle sessioni e `?live=ID` | V selezione browser senza live multipla; P con dirette concorrenti |
| W103 | Frame con stamp/ETag | 304 sul frame già posseduto | P |
| W104 | Long polling frame / notifiche SSE | ChangeNotifier verso Qt, SSE autenticato con eventi rows/stats/live e Last-Event-ID; ETag frame conservato | V notifica e cancellazione; D forma snapshot live rispetto al contratto Windows |
| W105 | Riproduzione file con seek | Range e lettura a blocchi | V |
| W106 | Player pubblico Twitch/YouTube | iframe e pagina pubblica | P |
| W107 | YouTube stato in onda e avviso | Ricerca streams/watch verificata prima di assegnare un embed; avvisi in Gestione Live e Anteprima | V parsing/stato e GUI; P risposte reali della piattaforma |
| W108 | Monitoraggio automatico YouTube con cache | Worker 30 s, tolleranza 90 s, cache 45/10 s, assenze/errori distinti, stop solo dopo due prove positive di fine | V stato e richieste cancellabili; P account pubblico |
| W109 | Chiudi avviso per live | Stato condiviso per ID; chiusura mantenuta fino al successivo riscontro della live pubblica | V stato C++ |
| W110 | Storico e snapshot | `MainLiveHistory.html`: filtri Canale/Stato/Titolo, pagine da 20 righe e URL persistente; nome/piattaforma conservati nello snapshot | V API/FFmpeg e browser |
| W111 | Riavvio storico | richiesta salvata, indice video e offset; replay/riparti/ricomincia nella Gestione Live, chiave corrente del canale o fresca TikTok | V motore FFmpeg; P singoli controlli replay |
| W112 | Eliminazione storico con protezione live | sessione e stato persistito LIVE/STARTING ricontrollati prima della cancellazione | V API e C++ |
| W113 | Pulizia manuale/per canale/automatica | soglie -1=tutto, 0=ieri, mesi di calendario; toggle/periodi salvati sulla prima configurazione attiva, scheduler per canale | V C++/API/browser e scheduler al riavvio |
| W114 | Pulizia progressiva dello storico | `history_cleanup_service.cpp`: worker, progresso video/dirette e barra; rimozione atomica dello snapshot per diretta, senza eliminare scene di libreria | V C++/API/browser; D modello record e scene temporanee Windows |
| W115 | Export dati/storico senza chiave | JSON con chiavi vuote | V |
| W116 | CPU, RAM e swap | `/proc/stat`, `/proc/meminfo` | P |
| W117 | Rete | contatori `/proc/net/dev`, escluso loopback | P |
| W118 | Spazio disco e I/O | statvfs e `/proc/diskstats` | P |
| W119 | Uptime e CPU logiche | `/proc/uptime`, hardware_concurrency | P |
| W120 | CPU/RAM/thread app e FFmpeg | `/proc/<pid>/stat`, percentuale per core | P |
| W121 | Sensori CPU/GPU | thermal e nvidia-smi quando disponibili | P |
| W122 | Dati hardware WMI/Windows | contatori Linux sostitutivi | D |

## Applicazione, lingue e distribuzione

Originali: `I18n/*`, `Ui/*`, `Messages/*`, `Pages/*`, `Logging/FileLoggerProvider.cs`, `Configuration/OrbisRuntimeOptions.cs`, shell WPF/WebView2 e `Installer/*`.

| ID | Funzione Windows | Linux | Stato |
|---|---|---|---|
| W123 | Navigazione desktop | finestra Qt nativa e navigazione nelle sette sezioni; HTML conservati come riferimento | V avvio nativo e navigazione in 21 lingue; P resa/contratto Windows |
| W124 | Tema sistema/chiaro/scuro | CSS e preferenza locale | P |
| W125 | 21 lingue UI/messaggi | bundle originali inclusi, navigazione localizzata | P |
| W126 | Traduzione dei controlli Windows | 21 cataloghi originali nella GUI; diciture aggiuntive native in italiano/inglese, dettagli diagnostici conservati | V chiavi e navigazione; D per diciture native extra |
| W127 | Manuale in 21 lingue con figure | originale incluso, figure Windows dichiarate | V rendering |
| W128 | Stampa/salvataggio PDF manuale | stampa/PDF con QTextDocument e QPrinter | P |
| W129 | Immagini del manuale | Figure originali Windows incluse, come nel riferimento; screenshot Qt prodotti dai test | D: screenshot Linux per ogni lingua sono documentazione aggiuntiva, non funzione Windows |
| W130 | Sfondo personalizzato dashboard | upload immagini e preferenza SQLite | P |
| W131 | Log FFmpeg consultabile/esportabile | file per sessione e API testo | P |
| W132 | Log applicazione/rotazione quotidiana | AsyncLogger limitato a 10.000 voci/8 MiB, file giornalieri e conservazione 3 giorni; include Qt, errori UI, preview e sessioni | V coda/flush/retention; P tutti gli errori dei driver |
| W133 | Percorsi runtime configurabili | XDG, env e CLI | V |
| W134 | Sicurezza media upload/path | librerie private, nome file validato, token locale | V token; P upload/path |
| W135 | Server Kestrel esposto in rete | server C++ solo loopback con sessione/token | D |
| W136 | API Windows / Swagger | `/api/*`; 61 route originali censite ma non compatibili | D/M per compatibilità API |
| W137 | SQLite e migrazioni | schema C++ separato, versione 1 | V |
| W138 | Import database Windows | Database Linux separato e protezione contro apertura di uno schema Windows; import diretto non implementato | D/M: migrazione tra OS aggiuntiva, non funzione dell’app Windows |
| W139 | Import/export Linux transazionale | validazione ID e rollback | V |
| W140 | Verifica aggiornamenti | manifest HTTPS Linux e link download | P |
| W141 | Controllo chiusura durante live | Conferma Qt, cancellazione richieste e stop di processi/worker; nessuna API Windows di blocco sospensione nel riferimento censito | V chiusura con entrambi i trasporti e finestre secondarie |
| W142 | Compilazione Release | CMake C++20 | V su Linux x86_64 |
| W143 | Installer NSIS/MSI | pacchetti DEB/TGZ e desktop entry | D |
| W144 | FFmpeg incluso offline nell’installer | FFmpeg/FFprobe e librerie 9.0.2 privati, sorgenti firmati e build riproducibile, dipendenze encoder tramite APT | V Debian 0.0.11 estratto: versione, GUI, API e decodifica completa con entrambi i trasporti |
| W145 | Test automatici | Core + FFmpeg/server/RTMP + Chrome headless | V |

## Validazione eseguita

- Compilazione con GCC 13, CMake, Boost 1.83, SQLite 3.45, json-c 0.17, FFmpeg 9.0.2 su Linux x86_64.
- Test Core: JSON, persistenza, import/export, rollback, ordinamento naturale, avvio processi senza shell, parser FLV a frammenti e timestamp tra segmenti.
- Test integrazione: server reale/token, playlist con audio/video, timestamp continui, mixer/takeover/ripresa/stop, ingest RTMP locale, thumbnail JPEG, proxy, benchmark x264 e riproduzione Range.
- Test Chrome: dashboard, CRUD canale, slot canvas, trascinamento, salvataggio scena e manuale.
- Le verifiche non provano driver hardware, servizi pubblici, degrado di una rete fisica, Wayland, ARM64 o tutte le combinazioni del catalogo codec. Le prove di pacing/adattamento, trasporti, WebP e canvas sono in streaming-engine e delivery.

## Parità ancora da completare

Le voci **M** sono una lista di lavoro concreta. La presenza del port Linux funzionante e dei test superati non equivale alla conclusione della richiesta “tutte le funzionalità”. Prima di dichiarare parità completa servono queste implementazioni, prove hardware/rete, traduzioni dei nuovi controlli e confronto operativo con il progetto Windows.

Il riesame del 16 ottobre è in [VERIFICA_2026-10-16.md](VERIFICA_2026-10-16.md). Le voci storiche riferite al browser non vanno interpretate come prova di identità di ogni widget Qt. Import di un database Windows, portale Wayland e screenshot Linux localizzati sono estensioni del port e non funzioni mancanti del programma Windows. La compatibilità binaria delle 62 API originali resta distinta dai flussi della GUI nativa.
