# Interfaccia Linux nativa — 0.0.10

## Architettura

1. `main.cpp` avvia normalmente `QApplication` e `NativeWindow`, senza avviare `serve()` o processi browser. `--headless`/`--no-browser` abilita esplicitamente il servizio precedente per test e automazioni.
2. `native_window.cpp` riprende navigazione e sezioni Windows: Pannello, Gestione Live, Anteprima, Layout, Gestore attività, Manuale e Impostazioni. Tabelle e filtri sono widget Qt.
3. `native_live_pages.cpp` contiene Gestione Live, anteprima con parametri/mixer/deck integrati e storico con filtri e pulizia. `native_dialogs.cpp` contiene configurazioni, avvio profilo → canale → composer, avvio playlist con profilo/canale/cartella sullo stesso foglio, dettagli, collegamento profilo, deck e import/export. Gli ID sono separati dalle diciture tradotte.
4. `native_composer.cpp` sostituisce il canvas JavaScript con QGraphicsView: sorgenti selezionabili, spostamento, angolo per ridimensionare, Shift, frecce/Canc, livelli, slot, overlay, file dal file manager e inspector. Il salvataggio richiama lo stesso `scene_service.cpp`.
5. `native_media.cpp` contiene player Qt Multimedia, librerie, playlist e manuale. Testo formattato e immagini del manuale sono resi da QTextDocument, senza motore web; stampa e PDF usano QPrinter. Il player sceglie il backend FFmpeg di Qt quando è disponibile e conserva gli override `QT_MEDIA_BACKEND`; il `.deb` include i decoder GStreamer per le versioni Qt che ne dipendono.
6. `native_forms.hpp` mantiene i campi sconosciuti del record originale, evitando di perdere opzioni importate durante una modifica del form.
7. I lavori pesanti e il campionamento usano QtConcurrent; i risultati aggiornano i widget nel thread grafico. La chiusura attende i lavori e ferma le dirette.

## Traduzioni

- I 21 cataloghi `web/resources/ui/ui.*.json` riusano i testi Windows, incluse le voci Connessione/Riconnessione importate dal checkout di riferimento.
- Le nuove diciture specifiche delle operazioni native (import/export configurazione, ripetizione playlist, video successivo, libreria video, modifica e altre azioni) sono in `native_i18n.cpp`, in italiano e inglese; nelle altre lingue queste sole diciture usano l'inglese.
- Il primo avvio sceglie la lingua del sistema se disponibile, altrimenti inglese. Il selettore mostra autonimi e salva la scelta nelle preferenze SQLite.
- Cambio lingua immediato; arabo RTL e formattazione QLocale di numeri e date. Le lingue artificiali mantengono i cataloghi Windows.
- Qt usa anche le traduzioni standard `qtbase_<lingua>.qm` per i propri dialoghi. Dove Qt non offre un catalogo, i pulsanti principali usano le chiavi Windows.
- Unicode supporta anche altri alfabeti; una nuova traduzione richiede cataloghi completi dell’interfaccia e del manuale. Non sono incluse traduzioni per ogni lingua del mondo.
- Nomi dei dispositivi, codec, unità, scorciatoie e marchi delle piattaforme restano dati tecnici. I messaggi applicativi sono tradotti; i dettagli delle eccezioni mantengono il testo diagnostico del motore.

## Verifiche

- `native-gui`: tutte le schermate nelle 21 lingue, identità delle chiavi, RTL, persistenza lingua, creazione canale/profilo, geometria del layout salvato e trasmissione FFmpeg da una playlist avviata nel wizard Qt e fotogrammi decodificati nel player nativo.
- `native-launch`: processo reale avviato nella modalità normale, database inizializzato, assenza del listener HTTP e del bootstrap browser, SIGTERM con uscita pulita.
- `core` e `integration`: SQLite, scene, pulizia, timestamp FLV, autentificazione dell’API headless e streaming reale continuano a usare le prove precedenti.
- `package_smoke.py`: verifica versione, avvio Qt e API/streaming dal `.deb` estratto senza installazione globale.
- `native-window.png` nella cartella di build è una cattura dei widget reali, non un mockup.
- `native-live.png`, `native-wizard.png` e `native-composer.png` documentano anche gestione live, scelta del profilo e modifica del layout. I test controllano che le schede non si sovrappongano a 960×640 e che i comandi dell'anteprima vuota siano disabilitati.

## Riferimento grafico e comandi

Il confronto con `guida-2.pdf` e con le pagine del checkout `lazy-stream-windows` mantiene la sequenza profilo → canale → composizione/cartella. La dashboard riprende le sezioni e le descrizioni di `MainMenu.cshtml`; navigazione e accento blu seguono le schermate della guida. `MainLive` usa un avvio principale, un avvio playlist distinto, l'eventuale sblocco dei video, filtri etichettati e uno stato vuoto; le righe hanno stato e titolo come pillole, la data di avvio e tutte le azioni come icone sempre visibili, ciascuna con il proprio marchio di piattaforma, senza il menu aggiuntivo delle altre tabelle.

Il compositore riprende da `_SceneComposer.cshtml` la separazione tra sorgenti/sagome, canvas e livelli. Le sagome restano selezionabili dal canvas e dalla lista; il pulsante finale indica se salva un layout oppure avvia la diretta. Il pannello scena mostra pulsanti in griglia, ripresa della live e spiegazione delle scorciatoie. Sono widget Qt: le immagini del manuale Windows restano un riferimento, non una certificazione di parità grafica o funzionale completa.

L'anteprima contiene il mixer in percentuale (0–200%, reset a 100%) e modifica i parametri dell'encoder senza cambiare l'identità della live. Il GOP resta di sola lettura. Il layout dell'anteprima si dispone in verticale sulle finestre strette. La selezione dei layout avviene nel composer; gli overlay caricati entrano nella libreria e si aggiungono al canvas selezionandoli. La chiusura del composer riporta alle scelte di avvio, conservandole. Lo storico espande i video della playlist nelle otto colonne Windows, conserva insieme canale/stato/titolo, pagina a 20 righe e mostra il progresso della pulizia.

Le prove native trasmettono a un server RTMP locale e verificano aggiornamenti reali di volume e bitrate audio, oltre a filtri e pulizia dello storico. L'anteprima locale usa i fotogrammi FFmpeg; il player della piattaforma si apre tramite il collegamento pubblico. Il catalogo delle liste codec Windows è importato in `web/resources/codec-catalog.json`, rigenerabile con `scripts/sync-codec-catalog.py`. Le opzioni native selezionabili restano limitate ai formati supportati dal motore Linux (H.264, yuv420p, AAC); le altre sono visibili e disabilitate.

## Differenze ancora da trasferire o verificare

Lo stato Live viene impostato dal progresso del publisher, dopo l'avvio dell'uscita RTMP. Durante l'handshake resta Connessione/Riconnessione e il bitrate non indica traffico inviato al server. Gli errori FFmpeg di uscita sono conservati separatamente in `logs/<id>.publisher.log`; i dettagli e lo storico oscurano chiave e URL. La diagnostica è visibile sullo stato della riga e nei dettagli della live. Le prove includono un handshake sospeso, una porta RTMP che rifiuta la connessione e un binario sostituito mentre è aperto.

La conversione nativa non certifica parità completa con Windows. Restano da confrontare proprietà per proprietà i form avanzati, i conflitti delle scorciatoie, i riquadri delle sorgenti e il comportamento delle finestre su un desktop reale. Aprire il deck arma le scorciatoie per quel live anche nelle schermate Live/Anteprima; possono essere disattivate dal deck. Il manuale conserva le immagini Windows originali.

Cattura Wayland tramite portale, monitoraggio automatico YouTube, publisher RTMP interno e notifiche push sono ancora i limiti del motore già riportati in MIGRAZIONE.md. Servizi di piattaforme pubbliche, camera/microfoni, driver GPU, stampa fisica e rendering di ogni alfabeto devono essere verificati sul computer di destinazione. I cataloghi non sono stati sottoposti a revisione linguistica.

I vecchi HTML/CSS restano nel repository e nel servizio headless come riferimento file per file; non vengono mostrati né usati dalla finestra Qt.
