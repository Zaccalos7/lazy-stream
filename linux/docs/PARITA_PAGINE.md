# Contratto delle pagine Windows e implementazione Qt

Riferimento: checkout locale `lazy-stream-windows`, pagine in `src/Orbis.Stream.Core/Pages`, confrontate il 16 ottobre 2026. Le schermate Linux sono controlli C++/Qt. Solo il player pubblico incorporato richiede Qt WebEngine. Il canvas dispone inoltre di un’anteprima FFmpeg locale, gestita da C++.

| Pagina Windows | Percorso Linux | Aperture e controlli |
|---|---|---|
| MainMenu | Pannello | Schede nelle sezioni Streaming, Impostazioni e Sistema, sfondo, navigazione laterale, lingua, tema e informazioni |
| MainLiveMenu | Live | Gestione live, Anteprima, Layout e Storico |
| MainSetting | Impostazioni | Due schede: Video e audio, Configurazioni canale |
| MainChannelSetting | Canali | Schede con marchio, descrizione, ingest, stato, modifica e interruttore; filtri piattaforma e attivazione; editor con piattaforme, chiave rivelabile, descrizione e istruzioni TikTok/Facebook |
| MainVideoSetting | Video e audio | Schede di profili personalizzati, modifica, duplicazione e attivazione; editor con preset piattaforma, catalogo codec, qualità, bitrate in Mbit/s, dimensioni/FPS opzionali, audio e opzioni di latenza/adattamento |
| MainLive / _LiveRows | Gestione live | Filtri Stato/Canale/Piattaforma, tabella con pillole e azioni sempre visibili, pagine numerate; dettagli, profilo, deck, anteprima, spot, stop, ripresa, riavvio ed eliminazione secondo lo stato |
| _StartWizardSetting / Configuration | Avvia Live | Profilo → canale → composizione; griglia di schede, radio, icone di piattaforma; Indietro conserva la scelta; Chiudi annulla |
| _SceneComposer | Compositore | Sorgenti usate/disponibili e overlay a sinistra, canvas al centro e livelli a destra; layout/nome/risoluzione e avvio sopra il canvas; drag/drop, selezione, inspector e ridimensionamento |
| _PlaylistVideos | Dettagli | Numero, stato per video, titolo/percorso, timestamp interrotto, segnalazione, anteprima, spot, stop, avvio da un video, riavvio ed eliminazione; log FFmpeg espandibili |
| MainLiveHistory | Storico | Otto colonne, filtri combinati Canale/Stato/Titolo, pagine da 20 video, messaggio completo, rapporto QR in più parti, eliminazione e pulizia per canale; progresso separato dalla tabella |
| MainPreview | Anteprima | Player, identità e posizione/durata a sinistra; scene e mixer sotto; sei dati sorgente/uscita e parametri a destra; Twitch/YouTube incorporati, collegamento pubblico, avviso Facebook; disposizione verticale su finestre strette |
| _SceneDeck | Pulsanti scena | Sei posizioni, media, nome, scorciatoia, modalità, posizione e durata; coordinate condizionali; conferma prima della messa in onda, ripresa e disattivazione scorciatoie |
| MainLayoutList / MainLayout | Layout | Elenco, apertura del compositore, slot e overlay, salvataggio ed eliminazione |
| MainTaskManager | Gestore attività | Indicatori circolari sistema/app, processi FFmpeg espandibili e dati del sistema Linux |
| MainManual | Manuale | Tutti i capitoli nello stesso documento, indice con ancore, immagini originali, stampa e PDF |

## Verifica

`native-gui` attraversa tutte le pagine nelle 21 lingue, crea canali/profili, duplica profili, controlla i dialoghi di errore e QR, segue avanti/indietro nel wizard, salva un layout e trasmette playlist e scena a un ingest RTMP locale. Salva schermate dei widget in `build/native-*.png`.

`platform-preview` usa un server HTTP locale e un MP4 H.264 generato con FFmpeg. Verifica che il tempo del video avanzi nel player incorporato. La sandbox WebEngine resta abilitata. Questa prova non verifica le autorizzazioni di incorporamento imposte da Twitch o YouTube sui singoli account.

## Confini della verifica

Il contratto sopra descrive i percorsi implementati; non attesta identità di ogni pixel su tutti i desktop. Qt usa font e dialoghi del sistema. Le informazioni hardware e i dispositivi provengono da Linux. Il catalogo Windows è selezionabile; disponibilità di encoder, formati e driver è verificata dal FFmpeg incluso. Il trasporto delle live usa FLV; non tutte le combinazioni arbitrarie del catalogo sono formati validi per un ingest RTMP. Le differenze del motore e le funzioni Windows esterne a queste pagine sono documentate in `MIGRAZIONE.md`. Servono prove su account reali per autenticazione ingest, stato pubblico, eventuali restrizioni del player e attivazione Facebook Live Producer.

`streaming-engine` verifica pacing, backpressure, cancellazione, adattamento, preset, anteprima del canvas, opzioni encoder, loop WebP e stato YouTube. `delivery` decodifica l’intero risultato di entrambi i trasporti, controlla formato uniforme, sorgenti senza audio, continuità dei DTS, ricorrenza dell’animazione e trasparenza reale. `integration` verifica anche SSE autenticato.
