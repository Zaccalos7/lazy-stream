# Diagnosi Twitch del 18 ottobre 2026

Esaminati `/var/log/syslog` e i registri privati dell'app in `~/.local/share/orbis-stream/logs/`. I timestamp dei log applicativi sono UTC: la sessione iniziata il 17 ottobre alle 23:19:51 UTC corrisponde al 18 ottobre alle 01:19:51 in Italia.

La sessione 4 mostra:

- FFmpeg 9.0.2 già presente nel pacchetto installato, H.264/AAC prodotti senza errori di codifica.
- Alle 23:19:54 UTC, connessione accettata dal publisher libavformat e primo timestamp consegnato.
- Il player pubblico cerca il canale `ciclovisione`; Twitch restituisce `MasterPlaylist:11`, `ErrorNotAvailable code 404`, `Failed to load playlist`.
- Alle 23:21:28 UTC la sessione termina come `STOPPED`, senza errori di pubblicazione. `Immediate exit requested` durante la chiusura segue la richiesta di arresto.

La verifica pubblica dell'identità di `ciclovisione` e il confronto locale con l'ID account nella chiave salvata mostrano **due account diversi**. La chiave è sintatticamente valida e non include `bandwidthtest=true`. La chiave completa non è stata inviata al servizio di verifica né riportata in questo documento.

Questo spiega la discrepanza tra invio accettato e player offline: la connessione viene aperta usando le credenziali di un account diverso dal canale mostrato. Per trasmettere su `ciclovisione`, apri la configurazione del canale nell'app e inserisci la chiave attuale ottenuta dalla dashboard Twitch di **quell'account**. Se vuoi usare l'account della chiave esistente, imposta invece il nome pubblico corretto. Salva e avvia una nuova diretta; verifica sia il publisher sia il player pubblico.

Non è stata avviata una trasmissione pubblica durante questa diagnosi e il database reale non è stato modificato. L'accettazione RTMP e l'avanzamento dell'invio confermano il trasporto; da soli non certificano che il canale cercato dall'anteprima sia online.

Riferimento ufficiale: [Twitch Video Broadcast](https://dev.twitch.tv/docs/video-broadcast/) descrive la chiave del broadcaster e la modalità `bandwidthtest=true`, che disabilita la visione pubblica.
