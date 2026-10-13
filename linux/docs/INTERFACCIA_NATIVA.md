# Interfaccia Linux nativa — 0.0.5

## Architettura

1. `main.cpp` avvia normalmente `QApplication` e `NativeWindow`, senza avviare `serve()` o processi browser. `--headless`/`--no-browser` abilita esplicitamente il servizio precedente per test e automazioni.
2. `native_window.cpp` riprende navigazione e sezioni Windows: Pannello, Gestione Live, Anteprima, Layout, Gestore attività, Manuale e Impostazioni. Tabelle e filtri sono widget Qt.
3. `native_dialogs.cpp` contiene configurazioni canali/profili, wizard profilo → canale → scena/cartella, dettagli storico, collegamento profilo, mixer, deck e import/export. Gli ID sono separati dalle diciture tradotte.
4. `native_composer.cpp` sostituisce il canvas JavaScript con QGraphicsView: sorgenti selezionabili, spostamento, angolo per ridimensionare, Shift, frecce/Canc, livelli, slot, overlay, file dal file manager e inspector. Il salvataggio richiama lo stesso `scene_service.cpp`.
5. `native_media.cpp` contiene player Qt Multimedia, librerie, playlist e manuale. Testo formattato e immagini del manuale sono resi da QTextDocument, senza motore web; stampa e PDF usano QPrinter. Il player sceglie il backend FFmpeg di Qt quando è disponibile e conserva gli override `QT_MEDIA_BACKEND`; il `.deb` include i decoder GStreamer per le versioni Qt che ne dipendono.
6. `native_forms.hpp` mantiene i campi sconosciuti del record originale, evitando di perdere opzioni importate durante una modifica del form.
7. I lavori pesanti e il campionamento usano QtConcurrent; i risultati aggiornano i widget nel thread grafico. La chiusura attende i lavori e ferma le dirette.

## Traduzioni

- Tutti i 21 cataloghi originali `web/resources/ui/ui.*.json` sono riusati byte per byte; ciascuno contiene 439 chiavi.
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

## Differenze ancora da trasferire o verificare

La conversione nativa non certifica parità completa con Windows. Restano da confrontare proprietà per proprietà i form avanzati, i conflitti delle scorciatoie, i riquadri delle sorgenti e il comportamento delle finestre su un desktop reale. Aprire il deck arma le scorciatoie per quel live anche nelle schermate Live/Anteprima; possono essere disattivate dal deck. Il manuale conserva le immagini Windows originali.

Cattura Wayland tramite portale, monitoraggio automatico YouTube, publisher RTMP interno e notifiche push sono ancora i limiti del motore già riportati in MIGRAZIONE.md. Servizi di piattaforme pubbliche, camera/microfoni, driver GPU, stampa fisica e rendering di ogni alfabeto devono essere verificati sul computer di destinazione. I cataloghi non sono stati sottoposti a revisione linguistica.

I vecchi HTML/CSS restano nel repository e nel servizio headless come riferimento file per file; non vengono mostrati né usati dalla finestra Qt.
