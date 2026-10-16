# Cronologia del port Linux

## 0.0.12 — 2026-10-16

- TikTok rimosso da motore C++, piattaforme, preset, marchi, wizard, player, headless, traduzioni e manuale. Otto preset per le quattro piattaforme rimanenti.
- Migrazione SQLite 2: configurazioni della piattaforma rimossa archiviate privatamente, storico e profili verticali personalizzati conservati, ID archiviati non riutilizzati.
- Corretta la parità dei canali: sender YouTube modificabile e conservato al salvataggio, ingest Facebook identico al riferimento Windows, istruzioni per gli ingest personali Kick/Facebook.
- Localizzati i titoli dei preset leggeri, omesse dimensioni/FPS non specificati nel wizard, allineati i colori chiari dei marchi; dipendenza WebP Qt esplicita.
- Corretta la chiusura del dialogo headless con campi obbligatori vuoti; la “×” annulla senza inviare il form.
- Sincronizzazione risorse con filtro persistente e verifica automatica di cataloghi, ingest, marchi SVG e assenza della piattaforma esclusa; figure del wizard e playlist catturate dalla GUI Qt nelle 21 lingue.

## 0.0.11 — 2026-10-16

- FFmpeg/FFprobe 9.0.2 compilati da sorgenti ufficiali con firma verificata e inclusi nel pacchetto; x264/x265, VP8/VP9, AV1, WebP e codec audio, NVENC/QSV/VAAPI secondo hardware e driver.
- Trasporto C++ nativo mediante libavformat, coda di tag compressi limitata, invio su thread dedicato e attese interrompibili; sender FFmpeg conservato per le piattaforme con buffering. Stato e posizione seguono il contenuto consegnato al publisher.
- Profili di consegna Windows per piattaforma, formato uniforme e audio, preroll, VBV, timeout e riconnessioni; bitrate adattivo basato su avanzamento e backlog, con recupero e protezione dalla lentezza dell'encoder.
- Dieci preset iniziali attivi, aggiornamento riconoscibile dei vecchi template senza sovrascrivere configurazioni modificate. Cataloghi selezionabili, risoluzione codec/pixel e verifica delle AVOption tramite FFmpeg; compatibilità con opzioni NVENC rimosse nella versione 9 e CBR x264 conservato insieme ai parametri manuali.
- Anteprima del compositore generata in C++ con lo stesso grafo, senza codifica H.264 né riproduzione audio; finestre e monitor X11 selezionabili. Catalogo sorgenti raggruppato e scorrevole, intestazioni e font avvicinati al riferimento Windows.
- Anteprima live fino a 30 fps con aggiornamento immagine separato dai controlli, limite di 640 pixel anche in verticale e conservazione dell’ultimo JPEG completo. Prima risoluzione delle connessioni condivise allineata a Windows; letture del relay più piccole per ridurre il ritardo dei comandi sulle scene statiche.
- Overlay statici decodificati una volta, decoder libvpx per alpha WebM, copie private WebP con ANIM infinito e ignore_loop=0. Eliminati metadati ripetuti e configurazioni AAC identiche che alteravano il rilevamento a 48 kHz nei cambi di produttore.
- Controllo YouTube in background con cache, tolleranza iniziale, avvisi e chiusura solo dopo due riscontri di fine della trasmissione pubblica. Richieste esterne cancellabili alla chiusura.
- Notifiche di modifica verso Qt e SSE autenticato per headless; log applicativi giornalieri asincroni con coda limitata e conservazione di tre giorni, diagnostica Qt e anteprima inclusa.

- Anteprima Twitch/YouTube incorporata con Qt WebEngine, controllata da C++; test con decodifica H.264 reale e sandbox attiva. Registro aggiuntivo della risoluzione del player, caricamento, console e arresti del renderer.
- Storico con stati/esiti separati per video, messaggio completo e rapporto QR multipagina; aggiornati i 21 cataloghi dai sorgenti Windows. Dettagli playlist con azioni sui singoli video e log espandibili.
- Canali e profili in schede con pulsanti, toggle e duplicazione; editor con griglie equivalenti a Windows e unità di bitrate coerenti. Impostazioni, indicatori attività, manuale continuo e pulsanti scena allineati ai percorsi Windows.
- Corretto il parsing dei valori JSON scalari usati dai cataloghi codec e il salvataggio dei layout dal compositore ampio.
- Qualità encoder applicata ai preset x264/NVENC/QSV come su Windows; Auto misura la velocità e conserva il risultato per encoder, risoluzione e FPS. Diagnostica persistita prima di pubblicare lo stato di errore, con protezione dello storico durante la riconnessione.
- Gestione Live con filtri compatti e pagine numerate; wizard profilo → canale → compositore ampio, con schede adattive fino a cinque colonne e comandi di navigazione distinti.
- Avvio scena sopra il canvas, sorgenti usate/disponibili e overlay a sinistra, livelli a destra. Trascinamento dal catalogo al canvas e aggiunta di un video tramite percorso.
- Sender FFmpeg allineato a Windows per analisi delle intestazioni AVC/AAC e app/playpath Facebook; keyframe periodici, flush FLV e CBR x264 sulle piattaforme che lo richiedono.
- Registri separati per sessione, producer e publisher, consultabili dai dettagli della diretta e dall'API. Comandi, PID, exit code, errori e riconnessioni conservati con credenziali oscurate prima della scrittura; watchdog per avvio e progresso del publisher.
- Test di navigazione del wizard, trasmissione RTMP da playlist e scena, errori di connessione, oscuramento dei log e conservazione dell'exit code.

## 0.0.10 — 2026-10-14

- Lo stato Live richiede progresso effettivo del publisher FFmpeg; durante il collegamento resta Connessione/Riconnessione. Ridotti i tempi di analisi FLV e aggiunto un timeout della connessione di uscita.
- Gli errori RTMP vengono raccolti nel registro della sessione e mostrati nei dettagli e sullo stato della riga, con chiave e URL oscurati. Gli avvisi GNOME non sono usati come diagnostica della trasmissione.
- L'avvio di FFprobe/FFmpeg continua a funzionare quando un aggiornamento sostituisce il binario dell'app aperta. Storico e pulizia proteggono anche le sessioni in riconnessione.
- Wizard playlist su un unico foglio, mixer e parametri nell'anteprima, deck e compositore a tre pannelli, storico con filtri e pulizia. Catalogo codec Windows importato con opzioni non supportate disabilitate.
- Prove RTMP locali, handshake sospeso, connessione rifiutata, oscuramento credenziali e avvio dopo sostituzione del binario.

## 0.0.9 — 2026-10-14

- La pagina Gestione Live riprende `MainLive.cshtml` e `_LiveRows.cshtml`: intestazioni Stato/Titolo/Canale/Data, pillole per stato, posizione della playlist e numero di sorgenti della scena, e provenienza sotto il titolo.
- Le azioni della riga non passano più dal menu aggiuntivo: marchio della piattaforma (con la pagina pubblica del canale), dettagli, collegamento profilo, deck delle scene, anteprima, spot, stop, avvio, riavvio ed eliminazione sono icone sempre visibili, nell'ordine Windows e abilitate secondo lo stato.
- L'intestazione mostra Sblocca video (solo con video bloccati), Avvia playlist con l'icona dell'elenco e Avvia Live con quella di riproduzione; i marchi delle piattaforme sono disegnati dai percorsi SVG di Windows, non da icone Qt.
- La ricerca testuale resta solo sulla pagina Storico; `live_rows.cpp` espone inoltre URL di ingresso, piattaforma riconosciuta, presenza di un profilo collegato e fermo immagine in onda.

## 0.0.8 — 2026-10-13

- La chiusura confermata della finestra principale termina esplicitamente l'app anche con finestre secondarie aperte.
- Le sessioni vengono arrestate dopo l'attesa dei lavori pendenti, includendo eventuali dirette avviate da quei lavori durante l'uscita.
- Test di chiusura con una seconda finestra e una trasmissione reale: verifica uscita del ciclo Qt e terminazione di producer e publisher FFmpeg.

## 0.0.5 — 2026-10-12

- Avvio grafico predefinito riscritto in C++/Qt 6 Widgets: nessun browser, WebView o listener HTTP. `--headless` mantiene l’API per test e automazioni.
- Finestre native per navigazione, canali, profili, wizard, dirette/storico, mixer, deck, scene/layout e librerie media. Canvas QGraphicsView con spostamento, ridimensionamento, livelli e file dal desktop.
- Player Qt Multimedia con scelta del backend FFmpeg disponibile, decoder GStreamer nel pacchetto e prova di fotogrammi MP4 reali; manuale in widget Qt con stampa/esportazione PDF.
- Tutti i 21 cataloghi Windows e le 439 chiavi originali riusati senza modifiche, selezione persistente, lingua iniziale dal sistema, formati locali e arabo RTL. Cataloghi standard Qt per i dialoghi di sistema.
- Test Qt di navigazione multilingua, CRUD e geometria del canvas, playlist reale dal wizard; controllo avvio nativo senza HTTP anche dal pacchetto estratto.
- Dipendenze Qt, traduzioni e font Unicode nel `.deb`; codice C++ formattato con clang-format.

Il port non dichiara ancora parità completa Windows: le differenze sono elencate in `docs/INTERFACCIA_NATIVA.md` e `docs/MIGRAZIONE.md`.

## 0.0.4 — 2026-10-12

- `MainLiveHistory.cshtml` e il suo modello ripresi nella pagina Storico: filtri Canale/Stato/Titolo combinati, ordinamento per avvio e pagine da 20 righe, con scelte mantenute nell'URL.
- Il polling conserva il testo e il focus dei filtri; una ricerca ritardata non riapre lo storico dopo una navigazione.
- Nome e piattaforma del canale salvati nello snapshot della diretta, indipendentemente dalle modifiche successive al canale.
- `LiveHistoryCleanupService.cs` portato in un servizio C++ dedicato: worker, progresso, conteggi video/dirette, un lavoro alla volta e protezione dei record LIVE/STARTING.
- Soglie “ieri” e “tutto” e mesi di calendario; controlli di pulizia automatica della prima configurazione attiva e scheduler per canale. Disabilitare la pulizia conserva i periodi scelti.
- Export dal browser autenticato con chiavi oscurate; test C++, API/FFmpeg e Chrome per storico, pulizia e download.
- Vincoli `keepInside` di `composer.js` ripresi per drag, ridimensionamento e frecce; test del salvataggio effettivo del layout dopo il trascinamento.
- Stop manuale distinto dalla fine naturale (`STOPPED`/`ENDED`); indice e timestamp persistiti per riprendere la playlist dal video interrotto.
- `MainLive.cshtml`, `_LiveRows.cshtml` e i due partial del wizard hanno file HTML dedicati: tabella di dieci dirette per pagina, filtri Stato/Canale/Piattaforma e azioni per playlist/scene persistite anche dopo il riavvio.
- `live_rows.cpp` unisce snapshot storici e sessioni correnti; dettagli playlist con riproduzione locale, ripresa da un video, ricomincia e collegamento profilo.
- Avvio scena nell'ordine profilo attivo → configurazione attiva → compositore; avvio playlist con cartella e countdown annullabile. Errori mostrati nel dialogo e scelte conservate durante Indietro o un avvio rifiutato.
- Test Chrome con FFmpeg in ascolto su localhost: avvio reale di playlist e scena dal wizard, dettagli/riproduzione e stop della scena; avvio da scena senza obbligo di cartella playlist.

Il modello dati Linux conserva un record per diretta con la playlist nello snapshot: il progresso conta i video inclusi, ma la rimozione è atomica per diretta. I media originali e le scene salvate nella libreria restano disponibili; la cancellazione delle scene temporanee Windows richiede ancora un modello equivalente.

## 0.0.3 — 2026-10-12

- Regole di `SceneService.cs` portate in C++: layout con slot, canvas valido, sorgenti presenti, file e overlay nella libreria, duplicati e rettangoli entro i bordi.
- Modificare una scena usata da una diretta crea una nuova scena e conserva lo snapshot originale; la cancellazione di scene e overlay usati viene rifiutata.
- Test C++ per questi vincoli e mappa Windows → Linux aggiornata.
- Struttura di `MainPreview.cshtml` ripresa in `MainPreview.html`: scelta della diretta, anteprima, pulsanti scena, mixer e fatti di uscita nello stesso pannello.
- Parametri applicabili durante la diretta editabili dall'Anteprima e player pubblico aperto su richiesta.
- Elenco e filtri di `MainChannelSetting.cshtml` ripresi in una pagina HTML dedicata, con dettagli e stato attivo nella riga.
- Preset ingest di Twitch, YouTube, Kick, Facebook e TikTok dal modello Windows; chiave TikTok nascosta perché richiesta a ogni avvio.
- `platformStreamName` torna a indicare la piattaforma; il player usa il nome del canale, mantenendo compatibilità con i vecchi handle Linux.
- `MainVideoSetting.cshtml` ha una pagina HTML per profili personalizzati, duplicazione, stato attivo e caricamento dei preset dalla stessa libreria seed.

## 0.0.2 — 2026-10-12

- Versione unica in `VERSION`, esposta dal binario, dall'API e dal pacchetto Debian.
- Rotte e gerarchia iniziale Pannello → Live/Anteprima/Layout e Pannello → Impostazioni riprese dalle pagine Windows.
- Struttura e collegamenti delle prime tre pagine di accesso ripresi in file HTML separati da `MainMenu.cshtml`, `MainLiveMenu.cshtml` e `MainSetting.cshtml`.
- Filtri Piattaforma/Stato e attivazione dei canali ripresi da `MainChannelSetting.cshtml`.
- Elenco Layout in `MainLayoutList.html`, con creazione, apertura ed eliminazione.
- Mappa dei 257 file Windows con hash e stato di porting esplicito.
- Normalizzazione dell'ingest YouTube RTMP in RTMPS derivata da `StreamPlatform.cs`.
- Rilevamento degli endpoint Twitch, YouTube, Kick, Facebook e TikTok secondo i domini del file Windows.
- Chiavi TikTok temporanee riconosciute anche dal nome pubblico e dai domini ingest regionali, come in `LiveStreamKeys.cs`.
- Installer `.deb` verificato dopo estrazione nei percorsi di installazione.
- Schema SQLite versionato: rifiuta database Windows e versioni Linux future senza modificarli.

## 0.0.1 — 2026-10-12

- Primo port C++20 Linux con FFmpeg, SQLite e interfaccia browser locale.
- Funzioni implementate e differenze note elencate in `docs/MIGRAZIONE.md`.
