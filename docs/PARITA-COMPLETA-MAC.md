# Confronto funzionale Windows / macOS

Revisione del 18 ottobre 2026. Il riferimento è il progetto Windows disponibile accanto a questo repository. Il primo intervento sulla navigazione non costituiva il porting completo: questa revisione comprende anche sorgenti, media, motore, gestione dei dati e controlli delle singole dirette.

L’[inventario verificabile](windows-parity-audit.json) associa **tutti i 62 endpoint e i 35 handler Razor** ai simboli Swift che eseguono le operazioni corrispondenti. Lo script `scripts/audit-macos-parity.py --check` fallisce se compare un’azione Windows non censita, se un riferimento Swift scompare o se l’inventario cambia. Questo controllo dimostra la copertura delle azioni censite; non sostituisce le prove su macOS.

| Area | Implementazione Mac |
|---|---|
| Percorsi | Panoramica, Live, Gestione Live, Anteprima, Scene e layout, Attività, Manuale, Impostazioni → Video e audio / Canali; pulsante Indietro e scorciatoie della Panoramica |
| Avvio | Wizard distinti Live/playlist, prima profilo poi canale e infine composizione/cartella; countdown annullabile; scene temporanee e uso dei layout senza aggiungere la live alla libreria |
| Canali e profili | Creazione, modifica, attivazione, filtri, duplicazione profili, selezione piattaforma, ingest personali, Portachiavi; dieci preset Apple con bitrate, dimensioni, fps e profilo H.264 standard/leggero; validazione prima della sostituzione in onda |
| Gestione Live | Filtri Stato/Canale/Piattaforma, dieci righe per pagina, titolo del video raggiunto, dettagli, anteprima, link pubblico, profilo, pulsanti scena, spot, stop, ripresa, riavvio ed eliminazione |
| Dettagli playlist | Video e posizioni, anteprime con controlli e metadati, avvio da un video mantenendo quelli successivi, profilo per singolo video, salto durante la live ed eliminazione dei video fermati |
| Scene e layout | File, camera, schermo, microfono, overlay; catalogo dispositivi; trascinamento e sostituzione negli slot; ridimensionamento, livelli, aggancio, tastiera, cambio risoluzione e orientamento; anteprima della composizione prodotta dallo stesso grafo FFmpeg |
| Regia | Modifica della composizione corrente, cambio scena, ritorno alla playlist nella posizione precedente, mixer per sorgente e playlist, mute/volume 0–200%, parametri video/audio e passaggio tra anteprima locale e pagina pubblica |
| Media e overlay | PNG/JPEG/BMP/WebP statici, GIF/APNG/WebP animati, WebM con decoder alpha, MOV/MP4; importazione controllata, copie private, deduplicazione, anteprime native o convertite, proxy per file pesanti e rimozione degli import abbandonati |
| WebP animati | Riconoscimento tramite probe/RIFF, copia temporanea del solo contatore ANIM per il loop nativo FFmpeg 9, frame/durate/alpha conservati e pulizia delle copie a fine sessione |
| Pulsanti scena | Creazione/modifica/duplicazione, media e anteprima, modalità intera o sovrapposta, posizione/durata, registrazione dei tasti, controllo dei conflitti, conferma configurabile, ripresa del programma e coda spot |
| Motore | Publisher persistente e produttori sostituibili, pacing, buffer limitato, timestamp FLV continui, audio stereo/silenzio, riconnessione, watchdog, qualità Auto sulla risoluzione effettiva e bitrate adattivo; scene con catture non terminate dalla fine di un file |
| Facebook | Query della chiave conservata integralmente nel playpath RTMP, diagnostica del publisher e separazione app/playpath per il sender FFmpeg |
| Storico e recupero | Snapshot, filtri e paginazione, ripresa e riavvio distinti, recupero delle sessioni precedenti senza checkpoint, posizione delle scene e della playlist sospesa, export, pulizia progressiva e automatica per canale dalla pagina Storico |
| Registri e report | Stderr progressivo con ruolo/PID, comandi ed exit code, errori del publisher propagati, registri giornalieri e per sessione leggibili dopo il riavvio; report testo/QR Unicode, copia/export e mascheramento di URL/chiavi/token |
| Sistema | CPU, memoria, compressione, swap, rete, dischi, uptime, energia e stato termico tramite API Apple; PID, CPU, memoria e thread dei processi FFmpeg |
| Preferenze e guida | Tema, sfondo, 21 cataloghi linguistici condivisi, layout arabo RTL, import/export, aggiornamenti all’avvio o manuali con pacchetti Mac arm64, indice e stampa/PDF del manuale con istruzioni Mac, informazioni e link del progetto |
| Distribuzione | Build arm64/macOS 14+, test prima del pacchetto, FFmpeg/FFprobe e librerie incluse, verifica del supporto WebP animato, firme, pkg e notarizzazione opzionale |

## Verifica eseguita

Il [risultato dei controlli](validation-macos-core.json) distingue il test del nucleo dalla prova dell’app nativa. Il nucleo e il relay vengono compilati realmente con Swift su Linux; le prove usano FFmpeg/FFprobe 9.0.2. Sono inclusi:

- composizione e decodifica di output FFmpeg reali, con audio e anteprima;
- sostituzione del produttore mantenendo lo stesso PID del publisher, decodifica finale e timestamp DTS ordinati per audio e video;
- loop WebP oltre la durata originale verificato confrontando gli hash dei frame ripetuti;
- trasparenza dei pixel e del padding nella composizione;
- errori del publisher, cancellazione e svuotamento delle pipe;
- ripresa/riavvio, avvio da un video, dati precedenti senza checkpoint, preset, profili RTMP, import/deduplicazione, registri, aggiornamenti e redazione dei report.

Per ripetere: `SWIFT_BIN=/percorso/swift ORBIS_FFMPEG_DIR=/percorso/ffmpeg9/bin bash scripts/test-macos-core.sh`. Su Mac esegue anche i test nativi; su Linux esegue il nucleo. Il prodotto macOS rimane esclusivamente Apple Silicon. `./scripts/build-macos.sh` esegue i test e genera app/pkg sul Mac.

## Differenze dichiarate e prove ancora necessarie

Le operazioni dell’interfaccia usano servizi Swift diretti: il server HTTP Windows non è riprodotto come API per client esterni. Le API di acquisizione e codifica Windows sono sostituite da AVFoundation e VideoToolbox; il trasporto usa il sender FFmpeg persistente. L’anteprima locale della composizione usa fotogrammi JPEG; la piattaforma pubblica si apre in WebKit. Le temperature numeriche CPU/GPU restano indicate come non disponibili e non vengono stimate dallo stato termico. Le descrizioni specifiche Mac senza una traduzione condivisa restano in italiano.

Su questo host Linux non sono stati compilati o eseguiti SwiftUI/AppKit, AVFoundation, VideoToolbox, WebKit, Mach/IOKit o l’installer. Il parsing del compilatore Swift per il target arm64 macOS verifica la grammatica dei sorgenti, non il collegamento con gli SDK Apple. Apertura delle schermate, catture autorizzate, streaming verso gli account reali, lettura QR, stampa e installazione devono essere provati su un Mac. L’inventario mantiene esplicitamente queste esecuzioni nello stato `pending`.
