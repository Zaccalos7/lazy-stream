# Verifica di parità — Linux C++ 0.0.12

Riferimento: checkout locale `lazy-stream-windows`, inventario di 272 file e 62 route. **TikTok è escluso su richiesta**: non è una funzione da riportare nel port. Questa verifica riguarda l'implementazione Qt e il motore C++, insieme alle risorse distribuite per la modalità headless.

## Correzioni emerse dal confronto

- Canali: il sender YouTube era bloccato su FFmpeg; ora il selettore è modificabile e la scelta viene conservata anche riaprendo l'editor. Gli altri canali continuano a usare il trasporto previsto dalla piattaforma.
- Facebook: il preset ingest ora corrisponde esattamente alla costante Windows `rtmps://live-api-s.facebook.com:443/rtmp`.
- Kick/Facebook: aggiunto l'aiuto degli ingest personali mostrato dal riferimento Windows.
- Preset leggeri: titolo localizzato nelle impostazioni e nel wizard, con le stesse chiavi `SettingTitleView` Windows. Un titolo personalizzato resta quello dell'utente.
- Profili che seguono la sorgente: il wizard omette dimensioni/FPS non specificati, come `_StartWizardSetting`; non mostra più `0×0` o `0 fps`.
- Marchi: path SVG identici a `PageHelpers.cs`, con colori distinti per tema chiaro/scuro.
- Manuale: dipendenza Qt per WebP dichiarata e preparabile senza installazione di sistema; verifica di decodifica reale. Le 42 figure di wizard/playlist sono catture Qt aggiornate nelle 21 lingue. Sincronizzati anche il capitolo e le figure dei rapporti d'errore.
- Client headless: la chiusura del dialogo non invia il form, quindi i campi obbligatori vuoti non bloccano l'annullamento.
- Editor profili headless: dimensioni e FPS vuoti restano nulli, come nel profilo Twitch Windows; la scelta del preset trasferisce anche le opzioni encoder. La prova browser verifica il salvataggio effettivo.

## Controlli riproducibili

| Ambito | Evidenza |
|---|---|
| Otto preset delle quattro piattaforme | Fixture estratta dalle dichiarazioni `DefaultSpec` Windows e confronto dei valori esposti dall'API: dimensioni, FPS, bitrate video/audio, preset, profilo e parametri x264 |
| Quattro ingest e marchi | `parity-scope` confronta direttamente sorgenti Windows e C++ |
| 21 cataloghi UI, messaggi e manuali | Confronto completo con le risorse Windows dopo la sola esclusione richiesta |
| Navigazione e pagine | `native-gui`: menu, impostazioni, canali/profili, layout, wizard avanti/indietro, dettagli, deck, storico/rapporti QR, anteprima/mixer, manuale e lingue |
| Nuove differenze dei canali | Editor con cinque scelte (quattro piattaforme più Custom), chiave persistente visibile, aiuti condizionali e riapertura del sender YouTube disabilitato |
| Streaming e preview | `streaming-engine`, `delivery`, `integration`, `platform-preview`: pacing, cambi producer, codec, decodifica completa, audio, alpha/loop, parametri, RTMP locale e player incorporato |
| Ciclo di vita e pacchetto | `native-launch`, le due prove di shutdown e `package_smoke.py` dal Debian estratto |
| Client headless | `browser.mjs`: navigazione, filtri, CRUD, wizard, RTMP, storico e pulizia; fixture in cartella temporanea |

Il confronto statico non certifica il comportamento di ogni metodo Windows: `PORTING_MAP.md` distingue codice correlato, risorse adattate, esclusioni e test Windows ancora da trasferire. I test della GUI verificano i flussi e i controlli esercitati; non costituiscono una misura dell'identità di ogni pixel.

Risultati finali: **11/11 gruppi CTest passati in 55,82 secondi**; `browser.mjs` passato con playlist e scena su RTMP locale; cattura dei 42 screenshot nelle 21 lingue passata. Debian estratto senza installazione: avvio/seconda istanza, risorse senza la piattaforma rimossa, dipendenza Qt WebP, API/SQLite e streaming completo con entrambi i trasporti tutti passati, inclusi cambi producer HEVC/VP9/AV1 e WebP animato con alpha.

Pacchetto: `build/packages/orbis-stream_0.0.12_amd64.deb`, 29.207.022 byte; SHA-256 `64bf225ce37701d96234d4c1acc6a92865feb2bc2862c1bb2e47fd94bf7a286f`.

## Rimozione della piattaforma esclusa

Rimossi enum e riconoscimento ingest, profilo di consegna/orientamento, due preset, marchio/colori, link pubblico, selettori/filtri, chiave temporanea nel wizard e nei riavvii, servizio `live_stream_keys`, UI headless, traduzioni e istruzioni dedicate. La sincronizzazione Windows applica nuovamente il filtro e conserva le figure Qt.

La migrazione SQLite 1 → 2 sposta canali/profili dedicati in `retired_records`, fuori dagli elenchi applicativi; conserva integralmente i dati nel database privato. Riconosce anche vecchi canali Custom con ingest regionali. Lo storico resta leggibile e i profili verticali personalizzati restano utilizzabili. Gli ID archiviati non sono riassegnati; un canale rimasto attivo che puntava a un profilo archiviato torna alla selezione del profilo. API e import rifiutano una piattaforma esplicita non supportata. Il publisher conserva il normale RTMP generico, senza logica dedicata alla piattaforma rimossa.

I riferimenti al nome della piattaforma restano nell'inventario originale Windows, nella migrazione dei dati e nei test che verificano la rimozione; non sono opzioni dell'applicazione.

## Differenze ancora aperte

- Le 62 API Windows e lo schema SQLite non sono compatibili uno a uno con il contratto Linux; i percorsi GUI coperti dai test sono una verifica distinta.
- Le icone di azione sono disegnate da Qt; i marchi delle quattro piattaforme condividono gli stessi path SVG Windows. Font, dialoghi di sistema e metriche dipendono dall'ambiente Linux.
- Camera, microfono, desktop/GPU e ingesti RTMPS su account pubblici richiedono prove sull'hardware e sugli account effettivi; le prove locali non verificano permessi embed o attivazione della trasmissione pubblica.
- Il relay live usa FLV; il catalogo codec non implica supporto di ogni combinazione da parte di ogni piattaforma.
- Rimangono casi dei test Windows da trasferire, elencati nella mappa. Non viene dichiarata parità totale soltanto in base alla compilazione o al numero di test passati.
