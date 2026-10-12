# Cronologia del port Linux

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
