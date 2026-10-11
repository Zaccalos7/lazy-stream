# OrbisStream · macOS Apple Silicon

Applicazione Swift e SwiftUI per macOS 14+, esclusivamente arm64. FFmpeg compone e codifica le sorgenti; VideoToolbox utilizza l’accelerazione Apple. Sidebar, toolbar, SF Symbols, Portachiavi, pannelli file, AVFoundation e WebKit sono nativi.

Il layout usa una toolbar unificata e una sidebar divisa tra studio, libreria, configurazione e monitoraggio. La panoramica offre accessi rapidi e schede adattive; superfici, testo e accento seguono i colori di sistema. Nella regia il pannello video/audio è richiudibile dalla toolbar e conserva la preferenza. I dettagli del Mac sono espandibili, con tutte le statistiche ancora accessibili.

## Compilazione e installer

Esegui su un Mac Apple Silicon con Xcode o Command Line Tools e FFmpeg/FFprobe arm64:

```sh
./scripts/build-macos.sh
```

Lo script esegue i test, compila in Release, incorpora FFmpeg/FFprobe e le dipendenze dinamiche, conserva soltanto le sezioni arm64, firma i componenti e genera:

- `dist/OrbisStream.app`
- `dist/OrbisStream-3.0.0-arm64.pkg`, che installa l’app in `/Applications`.

Il pacchetto richiede macOS 14 e arm64. Il Mac destinatario non deve installare FFmpeg separatamente. Il percorso predefinito dei binari per il build è `/opt/homebrew/bin`; puoi cambiarlo:

```sh
ORBIS_FFMPEG_DIR=/percorso/bin ./scripts/build-macos.sh
```

Usa una distribuzione FFmpeg con AVFoundation, VideoToolbox, AAC, FLV, RTMP e i filtri/encoder richiesti dai tuoi profili. Le dipendenze mancanti o senza arm64 interrompono il confezionamento. Licenza e configurazione FFmpeg sono incluse nelle risorse; `ORBIS_FFMPEG_LICENSE_DIR` permette di aggiungere licenze, avvisi e materiale sorgente della distribuzione scelta. Rispetta le condizioni delle librerie che distribuisci.

La firma predefinita è locale. Per una distribuzione firmata e notarizzata:

```sh
ORBIS_APP_SIGN_IDENTITY='Developer ID Application: Nome (TEAMID)' \
ORBIS_INSTALLER_SIGN_IDENTITY='Developer ID Installer: Nome (TEAMID)' \
ORBIS_NOTARY_PROFILE='profilo-notarytool' \
./scripts/build-macos.sh
```

Il profilo deve essere già configurato nel Portachiavi per `notarytool`. Lo script invia il pacchetto, attende la notarizzazione e allega il ticket. Apri `Package.swift` con Xcode per sviluppare.

## Censimento della migrazione

[Elenco punto per punto](docs/MIGRAZIONE.md): 228 file originali, 62 route e 126 identificatori di controlli, con corrispondenze al codice Swift. [Inventario originale](docs/original-inventory.json) e [matrice delle funzioni](docs/parity.json) sono consultabili anche da strumenti. Le voci indicano codice presente; la parità operativa richiede ancora le prove su Mac.

## Funzionalità

- Canali multipli, chiavi nel Portachiavi, configurazione di cartelle, scene e profili.
- Procedura di avvio con conto alla rovescia, start/stop e selezione della diretta osservata.
- Playlist ordinabili, abilitazione, profilo per video, ripresa da posizione, loop, salto al video successivo.
- Scene e layout riutilizzabili, canvas con trascinamento, ridimensionamento, allineamento, comandi da tastiera, drop dei file, anteprima video e acquisizione delle sorgenti.
- File, schermi, fotocamere e microfoni attraverso **AVFoundation**, usando gli indici rilevati dallo stesso FFmpeg.
- Overlay statici e animati, libreria locale, pulsanti scena, intermezzi, posizione, durata, scorciatoie macOS e ripresa del programma.
- Mixer audio per sorgente, mute e volume 0–200%, audio stereo/silenzio quando necessario.
- Profili video/audio con bitrate, GOP in secondi, risoluzione, fps, formato pixel, contenitore, qualità, preset, tune, bassa latenza, adattamento del bitrate e opzioni FFmpeg.
- **VideoToolbox H.264/HEVC** e alternativa software x264/x265. RTMP è limitato a FLV/H.264 con audio AAC/MP3; HEVC, Opus e altri contenitori sono conservati nei profili ma non vengono accettati per una diretta RTMP.
- Relay FLV Swift con timestamp continui, un processo FFmpeg publisher persistente e produttori sostituibili per playlist, scene, intermezzi e modifiche del mixer.
- Anteprima della composizione, avanzamento di FFmpeg, pagina pubblica della piattaforma in WebKit, registri con server mascherato, riconnessione e watchdog.
- Storico con snapshot, riavvio, esportazione, cancellazione, pulizia progressiva per canale e recupero delle sessioni interrotte.
- Spot in coda, registri giornalieri, risorse in 21 lingue, verifica aggiornamenti da manifest macOS configurabile e tema di sistema/chiaro/scuro.
- Panoramica/attività con CPU, RAM, compressione, swap, rete, spazio e I/O disco, uptime, risparmio energetico e stato termico macOS.

## Motore e dati

Il nucleo dello streaming è un relay FLV in Swift: un publisher FFmpeg mantiene la connessione RTMP, mentre i produttori vengono sostituiti durante cambi di file, scene, intermezzi e mixer. Il parser ricompone i tag ricevuti a blocchi, valida le dimensioni e riscrive i timestamp per mantenere una timeline continua. Il pacing segue l’uptime del Mac. Le transizioni possono avere una breve pausa di avvio del produttore.

I dati usano un modello Swift e JSON scritto atomicamente in `~/Library/Application Support/OrbisStream`; le chiavi sono nel Portachiavi ed escluse dall’esportazione. I controlli dell’interfaccia chiamano direttamente i servizi Swift.

Le catture passano attraverso AVFoundation. Per l’audio dello schermo occorre un dispositivo audio virtuale selezionato come microfono. Camera, microfono e schermo richiedono i permessi macOS. La qualità Auto usa un benchmark sintetico; l’adattamento del bitrate osserva l’avanzamento del publisher. Il controllo pubblico YouTube resta best effort: consenso, errori HTTP o pagine non riconosciute producono lo stato “non verificata”.

## Statistiche Apple

`SystemMetrics.swift` richiama direttamente Mach, sysctl, getrusage, getifaddrs e IOKit da Swift. CPU, memoria attiva/wired/compressa, swap e risorse del processo sono campionati con contatori nativi; rete e I/O disco si ricavano dalle differenze tra campioni. Il primo campione mostra `—` dove serve un intervallo. ProcessInfo fornisce stato termico e risparmio energetico, Foundation la capacità del volume.

La rete comprende le interfacce fisiche `en*` del Mac; il disco comprende i driver che espongono statistiche IOKit. CPU dell’app: 100% equivale a un core. La pagina attività campiona separatamente CPU, memoria e thread dei processi FFmpeg tramite proc_pidinfo. Le temperature numeriche CPU/GPU e l’utilizzo GPU non vengono stimati.

## Verifica

Il build esegue i test del relay, timestamp, URL, profili, mix, persistenza, controllo YouTube e output FFmpeg reale. Dopo il confezionamento verifica firme, architetture e avvio dei helper. I test e il build nativo richiedono macOS; questa revisione è stata preparata su Linux, quindi la compilazione SwiftUI e l’esecuzione dell’installer sul Mac restano da verificare. Camera, microfono, schermo e pubblicazione reale richiedono una prova sul Mac.
