# Cronologia del port Linux

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
