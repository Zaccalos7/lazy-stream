# Censimento e riscrittura Swift

Data: 12 ottobre 2026. Destinazione: macOS 14+, esclusivamente Apple Silicon/arm64.

**Stato: codice presente, validazione macOS ancora da eseguire.** La presenza di una funzione in questa tabella non certifica che la compilazione, il test o la cattura reale siano riusciti. Su questo host Linux sono stati eseguiti il parsing della sintassi Swift, i controlli dei documenti e la sintassi Bash; i test Swift richiedono il Mac.

Il censimento parte dai sorgenti originali disponibili: 228 file applicativi/test/risorse, 62 route HTTP e 126 identificatori HTML. `original-inventory.json` conserva file, SHA-256, simboli, proprietà, route e controlli; `parity.json` rende questa tabella consultabile da strumenti. Le route sono sostituite da chiamate dirette ai servizi Swift.

211 voci funzionali censite, oltre alla corrispondenza delle route e dei controlli. L’[aggiornamento del 18 ottobre](PERCORSI-MAC.md) allinea i percorsi di navigazione e le azioni alla versione Windows disponibile.

## Elenco punto per punto

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|

### Canali

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M001 | Elenco e ricerca | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M002 | Creazione, modifica ed eliminazione | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M003 | Nome e descrizione | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M004 | Server RTMP/RTMPS e chiave | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M005 | Nome pubblico o ID trasmissione | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M006 | Abilitazione del canale | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M007 | Cartella video | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M008 | Scena e profilo associati | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M009 | Regole di pulizia per canale | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M010 | Normalizzazione host YouTube e query della chiave | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M011 | Riconoscimento Twitch, YouTube, Kick, Facebook, TikTok e ingest personalizzato | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M012 | Chiave TikTok richiesta per ogni diretta | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M013 | Chiavi nel Portachiavi ed esclusione da export e storico | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |

### Profili

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M014 | Elenco, ricerca, creazione e modifica | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M015 | Duplicazione ed eliminazione con controllo dei riferimenti | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M016 | Attivazione e profilo predefinito | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M017 | Predefinito per piattaforma e preset di partenza | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M018 | Codec video e lista dei codec effettivamente disponibili | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M019 | Formato pixel e contenitore | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M020 | Bitrate video | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M021 | Codec e bitrate audio | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M022 | GOP in secondi e keyframe periodici | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M023 | Risoluzione e frequenza della sorgente tramite 0×0 e 0 fps | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M024 | Qualità Auto, Leggero, Bilanciato e Alta | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M025 | Preset e tune manuali | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M026 | Bassa latenza | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M027 | Interruttore bitrate adattivo | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M028 | Opzioni FFmpeg aggiuntive | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M029 | Validazione e opzioni riservate | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M030 | Associazione del profilo a un singolo video | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |
| M031 | Modifica dei parametri durante la diretta | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [CodecCatalog.swift](../Sources/OrbisCore/CodecCatalog.swift) · [FFmpegCapabilities.swift](../Sources/OrbisCore/FFmpegCapabilities.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) |

### Avvio e playlist

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M032 | Procedura di avvio a passi | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M033 | Scelta di canale e profilo | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M034 | Scelta o composizione di una scena durante l’avvio | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M035 | Conto alla rovescia annullabile | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M036 | Importazione di file e cartelle | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M037 | Solo file video, cartelle e duplicati esclusi | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M038 | Ordinamento naturale dei nomi | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M039 | Riordino e rimozione dei video | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M040 | Abilitazione dei singoli video | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M041 | Posizione di ripresa | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M042 | Loop o termine della playlist | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M043 | Avvio di playlist e dirette concorrenti | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M044 | Blocco del doppio avvio sullo stesso canale | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M045 | Salto al video successivo o a un video scelto | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M046 | Stop della singola diretta e stop generale | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M047 | Ripresa dopo errore e riconnessione | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |

### Scene e layout

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M048 | Elenco, ricerca, salvataggio, duplicazione ed eliminazione | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M049 | Nome e descrizione della scena | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M050 | Layout riutilizzabili e slot senza sorgenti | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M051 | Decorazioni mantenute nei layout | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M052 | Applicazione di un layout a sorgenti esistenti | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M053 | File, schermo, fotocamera e microfono | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M054 | Catalogo e indici dispositivi FFmpeg/AVFoundation | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M055 | Permessi camera, microfono e schermo | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M056 | Controllo di dispositivi duplicati | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M057 | Canvas con anteprime locali | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M058 | Trascinamento dei riquadri | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M059 | Ridimensionamento con maniglia e campi numerici | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M060 | Aggancio a bordi e centri | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M061 | Spostamento da tastiera e cancellazione della selezione | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M062 | Trascinamento di file sul canvas | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M063 | Coordinate X/Y, larghezza e altezza | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M064 | Riordino dei livelli | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M065 | Risoluzioni canvas e cambio orientamento | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M066 | Risoluzione ricavata dal file sorgente | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M067 | Ridimensionamento proporzionale degli elementi | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M068 | Svuotamento del canvas | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M069 | Protezione della chiusura accidentale con modifiche non salvate | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M070 | Audio per sorgente e volume 0–200% | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M071 | Composizione scale/pad/overlay e pixel quadrati | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M072 | Durata della scena pari al file più lungo e ultimo frame mantenuto | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M073 | Mix stereo e audio riempito con silenzio fino al termine della scena | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [CanvasMediaPreview.swift](../Sources/OrbisMac/CanvasMediaPreview.swift) · [SceneGeometry.swift](../Sources/OrbisCore/SceneGeometry.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |

### Media e overlay

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M074 | Libreria overlay e importazione | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M075 | PNG/JPEG/WebP e immagini statiche | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M076 | GIF/APNG animati | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M077 | WebM/MOV animati e trasparenza | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M078 | Decoder libvpx per alpha VP8/VP9 | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M079 | Loop degli overlay | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M080 | Anteprima e fotogramma di una sorgente | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M081 | Anteprima delle fotocamere e degli schermi | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M082 | Riproduzione locale con AVPlayer | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M083 | Conversione dei formati non riproducibili | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M084 | Copie leggere per sorgenti oltre Full HD | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M085 | Confronto dei decoder CPU/VideoToolbox | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M086 | Tone mapping PQ/HLG con zscale | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M087 | Proxy usato solo quando copre la dimensione del riquadro | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M088 | Preparazione dei proxy in coda, un lavoro alla volta | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M089 | Cache identificata da percorso, dimensione e data del file | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M090 | Rimozione delle copie inutilizzate oltre 30 giorni | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |
| M091 | Protezione dell’eliminazione di overlay ancora referenziati | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [MediaPreviewService.swift](../Sources/OrbisMac/MediaPreviewService.swift) · [MediaOptimization.swift](../Sources/OrbisMac/MediaOptimization.swift) · [FFmpeg.swift](../Sources/OrbisCore/FFmpeg.swift) |

### Pulsanti e spot

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M092 | Creazione, modifica, eliminazione e ordine del deck | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M093 | Etichetta, media e scorciatoia | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M094 | Scorciatoie macOS normalizzate e tasti riservati | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M095 | Funzionamento delle scorciatoie fuori dai campi di testo | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M096 | Intermezzo a schermo intero | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M097 | Immagine mantenuta fino alla ripresa | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M098 | Clip che torna automaticamente al programma | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M099 | Spot da file e coda per singola diretta | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M100 | Pulsante che interrompe lo spot e svuota la coda | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M101 | Ripresa che elimina gli spot in attesa | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M102 | Overlay nella scena e toggle dello stesso pulsante | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M103 | Posizioni angoli, centro e personalizzata | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M104 | Dimensioni e durata dell’overlay | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M105 | Timer protetto da richieste successive | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M106 | Ripresa dal cursore del programma e non dallo spot | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |
| M107 | Protezione dei media attivi o in coda | [SceneViews.swift](../Sources/OrbisMac/SceneViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeckHotkey.swift](../Sources/OrbisCore/DeckHotkey.swift) |

### Regia e trasporto

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M108 | Elenco delle sessioni e selezione della diretta | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M109 | Filtri canale, piattaforma, ricerca e dirette attive | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M110 | Anteprima JPEG della composizione codificata | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M111 | Dimensioni dell’anteprima corrispondenti al segnale reale | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M112 | Pagina pubblica della piattaforma in WebKit e browser esterno | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M113 | Mixer per sorgente, mute e volume | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M114 | Cambio scena durante la diretta | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M115 | FPS, bitrate, velocità, byte e posizione | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M116 | Parser FLV incrementale con validazione dei tag | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M117 | Un solo header FLV e sequence header preservati | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M118 | Timeline continua tra produttori | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M119 | Tagli ribasati sull’ultimo tag scritto, esclusi i frame scartati | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M120 | Intervallo tra segmenti derivato dal frame rate | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M121 | Publisher FFmpeg persistente | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M122 | Buffer limitato per durata e memoria | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M123 | Backpressure e risveglio su interruzione o cancellazione | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M124 | Cuscino iniziale del relay | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M125 | Recupero del ritardo a 2× senza burst incontrollati | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M126 | Pacing nel sender e preroll per YouTube/Facebook/TikTok | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M127 | Margini, sample rate, CBR e timeout specifici per piattaforma | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M128 | Formato uniforme dove richiesto | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M129 | Orientamento verticale TikTok | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M130 | Bitrate adattivo basato su drift, backlog e throughput | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M131 | Distinzione tra rete lenta, encoder lento e rete ferma | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M132 | Riduzione con margine, gradini di recupero e attesa raddoppiata | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M133 | Qualità automatica alleggerita dopo 10 secondi senza margine | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M134 | Benchmark qualità con cache e soglia 2× | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M135 | Watchdog e riconnessioni con limite consecutivo | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M136 | Chiusura naturale del publisher con flush | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M137 | Stop forzato dei processi bloccati | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M138 | Cancellazione delle operazioni asincrone FFmpeg | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M139 | Controllo YouTube con tolleranza iniziale e conferme consecutive | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M140 | Stato YouTube non verificato per consenso/errori/pagine sconosciute | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |
| M141 | Avviso YouTube chiudibile | [LiveViews.swift](../Sources/OrbisMac/LiveViews.swift) · [RelayEngine.swift](../Sources/OrbisCore/RelayEngine.swift) · [LiveSession.swift](../Sources/OrbisMac/LiveSession.swift) · [DeliveryBuffer.swift](../Sources/OrbisCore/DeliveryBuffer.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) · [StreamingPolicy.swift](../Sources/OrbisCore/StreamingPolicy.swift) · [EncoderTuner.swift](../Sources/OrbisCore/EncoderTuner.swift) · [YouTubeAir.swift](../Sources/OrbisCore/YouTubeAir.swift) |

### Storico e recupero

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M142 | Snapshot di canale, profilo, scena e playlist | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M143 | Data iniziale, data finale ed errore | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M144 | Ricerca per canale, scena o titolo del video | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M145 | Filtri di canale e stato e ordinamento cronologico | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M146 | Dettagli dei video di una diretta | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M147 | Riavvio della diretta | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M148 | Replay di un video dall’inizio o dalla posizione | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M149 | Eliminazione di una riga video o di una diretta | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M150 | Esportazione dello storico | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M151 | Pulizia di tutto, prima di ieri o per mesi | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M152 | Avanzamento della pulizia e conteggio dei video/dirette | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M153 | Protezione delle sessioni attive durante la pulizia | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M154 | Pulizia periodica per canale | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M155 | Checkpoint delle sessioni ogni cinque secondi | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M156 | Elenco delle sessioni interrotte | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |
| M157 | Sblocco e ripresa dopo chiusura inattesa | [ConfigurationViews.swift](../Sources/OrbisMac/ConfigurationViews.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [HistoryCleanup.swift](../Sources/OrbisCore/HistoryCleanup.swift) · [Models.swift](../Sources/OrbisCore/Models.swift) |

### Statistiche Apple

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M158 | CPU del Mac | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M159 | RAM attiva/wired/compressa e capacità fisica | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M160 | Memoria compressa | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M161 | Swap usato e totale | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M162 | Capacità totale e disponibile del volume | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M163 | Rete sulle interfacce fisiche en* | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M164 | I/O aggregato dei driver disco | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M165 | CPU e memoria dell’app | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M166 | Processi FFmpeg con ruolo e PID | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M167 | CPU, memoria e thread dei processi FFmpeg | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M168 | Uptime | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M169 | Stato termico e risparmio energetico | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |
| M170 | Primo campione e contatori non disponibili mostrati come assenti | [SystemMetrics.swift](../Sources/OrbisCore/SystemMetrics.swift) · [ProcessMetrics.swift](../Sources/OrbisCore/ProcessMetrics.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) |

### Applicazione e dati

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M171 | Sidebar, toolbar, menu e pannelli file nativi | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M172 | Tema di sistema, chiaro e scuro | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M173 | Sfondo personalizzato della panoramica | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M174 | Selezione della lingua e 21 risorse linguistiche | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M175 | Guida alle funzioni | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M176 | Informazioni e versione dell’app | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M177 | JSON atomico con permessi 0600 | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M178 | Import/export della configurazione | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M179 | Lettura dei documenti senza i campi aggiunti in questa revisione | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M180 | Validazione degli ID duplicati e dei dati importati | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M181 | Copia di recupero per configurazione corrotta | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M182 | Media importati in Application Support | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M183 | Registri giornalieri persistenti | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M184 | Registri con coda limitata e conservazione di tre giorni | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M185 | Mascheramento di server, chiave e token | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M186 | Esportazione del registro della sessione | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M187 | Protezione del sonno durante la diretta | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |
| M188 | Conferma di chiusura quando esistono dirette attive | [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) · [AppModel.swift](../Sources/OrbisMac/AppModel.swift) · [AppleServices.swift](../Sources/OrbisMac/AppleServices.swift) · [NativeLocalization.swift](../Sources/OrbisMac/NativeLocalization.swift) · [SessionLog.swift](../Sources/OrbisCore/SessionLog.swift) · [FLVRelay.swift](../Sources/OrbisCore/FLVRelay.swift) |

| M201 | Verifica aggiornamenti tramite manifest HTTPS di installer macOS arm64 | [UpdateSettingsView.swift](../Sources/OrbisMac/UpdateSettingsView.swift) · [OrbisApp.swift](../Sources/OrbisMac/OrbisApp.swift) |

### Distribuzione

| ID | Funzione originale o equivalente Apple | Codice Swift |
|---|---|---|
| M189 | Compilazione Swift release solo arm64 | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M190 | Rifiuto di Linux, Intel e macOS antecedente a 14 | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M191 | FFmpeg e FFprobe inclusi nell’app | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M192 | Dipendenze dinamiche ricorsive incluse e rilocate | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M193 | Eliminazione delle sezioni Intel dei binari universali | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M194 | Verifica di codec alpha e readrate_initial_burst | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M195 | Prova di codifica H.264 VideoToolbox | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M196 | Installer pkg per Applications con requisito arm64/macOS 14 | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M197 | Firma dei componenti e del bundle | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M198 | Firma Developer ID Installer opzionale | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M199 | Notarizzazione e allegato del ticket opzionali | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |
| M200 | Test Core e test del relay/sessione nativa prima del pacchetto | [build-macos.sh](../scripts/build-macos.sh) · [bundle-ffmpeg.swift](../scripts/bundle-ffmpeg.swift) · [Package.swift](../Package.swift) · [Info.plist](../macOS/Info.plist) |

## Adattamenti espliciti

- WPF, Razor, server HTTP, SQLite, browser locale e aggiornamenti SSE diventano SwiftUI, servizi Swift, Combine e persistenza JSON. Non si conserva la struttura .NET.
- NVENC/QSV/AMF diventano VideoToolbox. La pubblicazione RTMP passa sempre da FFmpeg, come richiesto: il selettore del vecchio publisher .NET è eliminato.
- Camera e microfono si selezionano come sorgenti AVFoundation distinte nel mixer; lo schermo usa AVFoundation. Per audio del sistema è necessario un dispositivo virtuale. Non si dichiara una cattura ScreenCaptureKit implementata.
- Le temperature numeriche CPU/GPU e il carico GPU non vengono inventati: i controlli Apple mostrano stato termico e risparmio energetico.
- Le liste native sostituiscono le pagine del server; filtri e ordinamento restano controlli dell’app.
- La verifica aggiornamenti usa un manifest macOS configurabile nelle impostazioni; il feed Windows originale non viene usato. Il manifest HTTPS ha campi `version`, `architecture` (`arm64`) e `url` (installer HTTPS `.pkg`). È necessario pubblicare questo manifest per distribuire nuove versioni.
- Le risorse delle 21 lingue sono trasferite. Le nuove diciture Apple senza traduzione usano l’italiano; non si dichiara completa la traduzione di ogni nuova frase. I manuali italiano/inglese sono riscritti per i controlli nativi; gli altri manuali trasferiti richiedono ancora revisione delle istruzioni web. Le figure della vecchia guida web non sono screenshot dell’app Mac e non vengono riutilizzate.
- I contenitori/codec non RTMP restano nel modello e nel command builder per output su file. L’avvio dalla regia è limitato a RTMP/FLV/H.264: non è presente una UI completa per registrazione in altri formati.

## Prove di regressione predisposte

- `OrbisCoreTests`: URL e piattaforme, validazione, composizione/mix, portrait, parsing progress, FLV, persistenza e output FFmpeg reale.
- `MigrationParityTests`: durata della scena, pacing, bitrate/rete/encoder, backpressure e cancellazione, pulizia protetta, compatibilità JSON, tagli FLV, alpha WebM, audio pad, scorciatoie e ID duplicati.
- `OrbisMacTests/LiveSessionTests`: programma e due spot, stesso PID del publisher, ripresa del programma e decodifica del file catturato. Il test scrive in una cartella temporanea e non pubblica su Internet.

Queste prove vengono eseguite da `./scripts/build-macos.sh`; non sono state eseguite su questo host Linux.

## Corrispondenza delle route originali

| Operazione originale | Servizio/vista nativa |
|---|---|
| `GET /scene/layouts` | SceneViews / SceneGeometry / AppModel |
| `GET /scene/{pkid:long}` | SceneViews / SceneGeometry / AppModel |
| `POST /scene/save` | SceneViews / SceneGeometry / AppModel |
| `DELETE /scene/{pkid:long}` | SceneViews / SceneGeometry / AppModel |
| `GET /preview/sources` | AppleCapture / SourceInspector / MediaPreviewService / MediaOptimization |
| `GET /preview/sources/snapshot` | AppleCapture / SourceInspector / MediaPreviewService / MediaOptimization |
| `POST /preview/sources/prepare` | AppleCapture / SourceInspector / MediaPreviewService / MediaOptimization |
| `GET /preview/sources/probe` | AppleCapture / SourceInspector / MediaPreviewService / MediaOptimization |
| `GET /scene/overlays` | SceneViews / SceneGeometry / AppModel |
| `POST /scene/overlays` | SceneViews / SceneGeometry / AppModel |
| `GET /scene/overlays/{name}` | SceneViews / SceneGeometry / AppModel |
| `GET /scene/overlays/{name}/still` | SceneViews / SceneGeometry / AppModel |
| `DELETE /scene/overlays/{name}` | SceneViews / SceneGeometry / AppModel |
| `POST /live/start-scene-live` | AppModel.start / LiveSession |
| `POST /live/start-live` | AppModel.start / LiveSession |
| `POST /live/start-video-live` | AppModel.start / LiveSession |
| `PUT /live/stop-live` | AppModel.start / LiveSession |
| `GET /live/{pkid:int}/scene` | AppModel.start / LiveSession |
| `POST /live/{pkid:int}/scene/resume` | AppModel.start / LiveSession |
| `POST /live/{pkid:int}/scene/{button:long}` | AppModel.start / LiveSession |
| `PUT /scene-buttons/{pkid:long}` | SceneViews / DeckEditor / LiveSession |
| `DELETE /scene-buttons/{pkid:long}` | SceneViews / DeckEditor / LiveSession |
| `POST /scene-buttons/media` | SceneViews / DeckEditor / LiveSession |
| `GET /scene-buttons/media/{name}/still` | SceneViews / DeckEditor / LiveSession |
| `GET /video/getPage` | PlaylistView / HistoryView / RecoveryView / HistoryCleanup |
| `GET /video/getAllVideo` | PlaylistView / HistoryView / RecoveryView / HistoryCleanup |
| `GET /video/getAllChannelWithVideoLive` | PlaylistView / HistoryView / RecoveryView / HistoryCleanup |
| `PUT /video/unlockVideo` | PlaylistView / HistoryView / RecoveryView / HistoryCleanup |
| `DELETE /video/live-history/{pkid:long}` | PlaylistView / HistoryView / RecoveryView / HistoryCleanup |
| `DELETE /video/live-history/older-than/{months:int}` | PlaylistView / HistoryView / RecoveryView / HistoryCleanup |
| `POST /video/live-history/cleanup` | PlaylistView / HistoryView / RecoveryView / HistoryCleanup |
| `GET /video/live-history/cleanup` | PlaylistView / HistoryView / RecoveryView / HistoryCleanup |
| `POST /settings/save` | ChannelsView / ChannelEditor / AppModel |
| `PUT /settings/change` | ChannelsView / ChannelEditor / AppModel |
| `GET /settings/retrive` | ChannelsView / ChannelEditor / AppModel |
| `GET /settings/retrive-channels` | ChannelsView / ChannelEditor / AppModel |
| `GET /settings/retrive-directories-path` | ChannelsView / ChannelEditor / AppModel |
| `DELETE /settings/delete` | ChannelsView / ChannelEditor / AppModel |
| `GET /video-setting/getVideoSettings` | ProfileEditor / ProfileFields / PlaylistItem.profileID |
| `POST /video-setting/save` | ProfileEditor / ProfileFields / PlaylistItem.profileID |
| `POST /video-setting/link` | ProfileEditor / ProfileFields / PlaylistItem.profileID |
| `PUT /video-setting/edit` | ProfileEditor / ProfileFields / PlaylistItem.profileID |
| `DELETE /video-setting/delete` | ProfileEditor / ProfileFields / PlaylistItem.profileID |
| `POST /image/upload` | PreferencesView / DashboardView / WorkspaceStore.importMedia |
| `GET /image/loadimage` | PreferencesView / DashboardView / WorkspaceStore.importMedia |
| `GET /taskManager/statistics/allInfo` | SystemSampler / ProcessSampler / TasksView |
| `GET /taskManager/statistics/cpu` | SystemSampler / ProcessSampler / TasksView |
| `GET /taskManager/statistics/ram` | SystemSampler / ProcessSampler / TasksView |
| `GET /taskManager/statistics/swap` | SystemSampler / ProcessSampler / TasksView |
| `GET /taskManager/statistics/cpu/temperature` | SystemSampler / ProcessSampler / TasksView |
| `GET /taskManager/statistics/gpu/temperature` | SystemSampler / ProcessSampler / TasksView |
| `GET /preview/live` | LiveViews / LiveSession / YouTubeAir |
| `GET /preview/live/embed` | LiveViews / LiveSession / YouTubeAir |
| `POST /preview/live/{pkid:int}/air/dismiss` | LiveViews / LiveSession / YouTubeAir |
| `GET /preview/live/{pkid:int}/video` | LiveViews / LiveSession / YouTubeAir |
| `GET /preview/live/{pkid:int}/frame` | LiveViews / LiveSession / YouTubeAir |
| `PUT /preview/live/{pkid:int}/parameters` | LiveViews / LiveSession / YouTubeAir |
| `PUT /preview/live/{pkid:int}/volume` | LiveViews / LiveSession / YouTubeAir |
| `GET /updates` | ObservableObject / @Published / Combine |

## Identificatori dei controlli originali

Gli identificatori seguenti sono riferimenti al censimento HTML, non nomi imposti al nuovo modello Swift. Le viste native corrispondenti sono indicate per ogni pagina.

### Countdown.cshtml → LiveViews / OrbisApp

`viewport`

### MainChannelSetting.cshtml → ConfigurationViews / OrbisApp

`@configuration.Id`, `@editing.Id`, `active`, `autoCleanupEnabled`, `autoCleanupIntervalMonths`, `autoCleanupOlderThanMonths`, `channelName`, `current`, `description`, `ffmpegSender`, `id`, `platform`, `platformStreamName`, `streamKey`, `streamUrl`

### MainLayout.cshtml → SceneViews

`_SceneComposer`

### MainLayoutList.cshtml → SceneViews

`@layout.Pkid`

### MainLive.cshtml → LiveViews / OrbisApp

`_LiveRows`, `_PlaylistVideos`, `_SceneComposer`, `_SceneDeck`, `_StartWizardConfiguration`, `_StartWizardSetting`, `_VideoSettingFields`, `channelName`, `compose-dialog`, `configurationId`, `live-key-dialog`, `liveKey`, `liveStatus`, `platform`, `playlist-details`, `playlist-dialog`, `scenePkid`, `settingId`, `spot-dialog`, `spotPath`, `start-dialog`, `streamKey`, `unlock-dialog`, `videoFolder`, `videoKey`

### MainLiveHistory.cshtml → LiveViews / OrbisApp

`@pick.Key`, `_Pager`, `autoCleanupEnabled`, `autoCleanupFields`, `autoCleanupIntervalMonths`, `autoCleanupOlderThanMonths`, `channelName`, `cleanupProgress`, `enabled`, `intervalMonths`, `liveStatus`, `olderThanMonths`, `runCleanupBtn`, `runCleanupLabel`, `titleSearch`

### MainManual.cshtml → ConfigurationViews / OrbisApp

`@headingId`, `@part.Id`

### MainPreview.cshtml → LiveViews / OrbisApp

`@T[`, `@live.VideoPkid`, `_SceneDeck`, `_SceneDeckIcon`, `_SpotIcon`, `spot-dialog`, `spotPath`, `videoKey`

### MainTaskManager.cshtml → ConfigurationViews / OrbisApp

`_AppStats`, `_Stats`

### MainVideoSetting.cshtml → ConfigurationViews / OrbisApp

`@Model.Form.Id`, `@setting.Id`, `_VideoSettingFields`

### _Layout.cshtml → SceneViews

`about-button`, `about-dialog`, `confirm-dialog`, `orbis-sidebar`, `sidebar-toggle`, `update-bell`, `update-dialog`, `update-dialog-text`, `viewport`

### _SpotIcon.cshtml → ConfigurationViews / OrbisApp

`@mask`

### _VideoSettingFields.cshtml → ConfigurationViews / OrbisApp

`@T[`, `AdaptiveBitrate`, `AudioBitrate`, `AudioCodec`, `FrameRate`, `GopSize`, `Id`, `IsActive`, `LowLatency`, `PixelFormat`, `Preset`, `Quality`, `Title`, `Tune`, `VideoBitrate`, `VideoCodec`, `VideoCodecName`, `VideoFormat`, `VideoHeight`, `VideoWidth`

### _AppStats.cshtml → ConfigurationViews / OrbisApp

`@label`, `ffmpeg-stats-container`

### _LiveRows.cshtml → LiveViews / OrbisApp

`_Pager`, `_SceneDeckIcon`, `_SpotIcon`, `streamKey`

### _PlaylistVideos.cshtml → ConfigurationViews / OrbisApp

`_SpotIcon`, `streamKey`

### _SceneComposer.cshtml → SceneViews

`@T[`

### _SceneDeck.cshtml → SceneViews

`_SceneDeckIcon`, `displayMode`, `durationSeconds`, `height`, `label`, `placement`, `scene-deck`, `width`, `x`, `y`

### _StartWizardConfiguration.cshtml → LiveViews / OrbisApp

`configurationId`, `streamKey`

### _StartWizardSetting.cshtml → LiveViews / OrbisApp

`settingId`

### _Stats.cshtml → ConfigurationViews / OrbisApp

`@label`


### Percorsi e parità Windows — 18 ottobre 2026

| ID | Funzione | Codice Swift |
|---|---|---|
| M202 | Hub Live/Impostazioni e navigazione Indietro | `NavigationViews.swift`, `OrbisApp.swift`, `AppModel.swift` |
| M203 | Wizard Live/playlist: profilo → canale → sorgente | `StartWizard.swift` |
| M204 | Gestione Live persistente, filtri e paginazione | `NavigationViews.swift` |
| M205 | Dettagli, anteprima, profilo, deck, spot, stop ed eliminazione dalla riga | `NavigationViews.swift`, `AppModel.swift` |
| M206 | Ripresa dal video/timestamp e riavvio con seek azzerati | `PlatformPresets.swift`, `Models.swift`, `AppModel.swift` |
| M207 | Dieci preset di uscita per piattaforma e VideoToolbox | `PlatformPresets.swift`, `ConfigurationViews.swift` |
| M208 | Selettore piattaforma e server iniziali | `PlatformPresets.swift`, `ConfigurationViews.swift` |
| M209 | Mixer audio della playlist | `LiveSession.swift`, `LiveViews.swift` |
| M210 | Report testo/QR, copia/export e mascheramento segreti | `ErrorReport.swift`, `ErrorReportView.swift`, `LiveSession.swift` |
| M211 | Indice, stampa/PDF, etichette condivise e layout RTL | `ManualPrint.swift`, `NativeLocalization.swift`, `OrbisApp.swift` |
