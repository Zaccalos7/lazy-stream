# OrbisStream · macOS

App nativa **SwiftUI / Swift** per macOS 14 o successivo, Apple Silicon e Intel. FFmpeg resta il motore di composizione, codifica e pubblicazione. Il progetto principale è ora il pacchetto Swift alla radice; i sorgenti Windows in `src/` sono conservati come riferimento della migrazione.

## Avvio sul Mac

Installa Xcode o i Command Line Tools e una distribuzione FFmpeg per macOS, per esempio:

```sh
brew install ffmpeg
./scripts/build-macos.sh
open dist/OrbisStream.app
```

Apri `Package.swift` con Xcode per lavorare sui sorgenti. Lo script esegue i test, compila in Release e crea un bundle `.app` con firma locale e descrizioni dei permessi. L'architettura è quella del Mac che compila. FFmpeg/FFprobe sono cercati nel bundle (`Contents/Helpers`), nei percorsi Homebrew Intel/Apple Silicon e nel `PATH`; puoi specificarli nelle impostazioni. Non vengono distribuiti gli `.exe` o le DLL Windows.

Il bundle prodotto usa FFmpeg installato sul Mac. Per distribuire un'app autonoma occorre fornire binari FFmpeg/FFprobe macOS e le loro dipendenze, rispettare le relative licenze, firmare i componenti e notarizzare il bundle con Developer ID. Il build locale non realizza notarizzazione o un installer DMG.

## Funzioni migrate

- Canali multipli, chiavi nel Portachiavi, configurazione di cartelle, scene e profili.
- Procedura di avvio con conto alla rovescia, start/stop e selezione della diretta osservata.
- Playlist ordinabili, abilitazione, ripresa da posizione, loop, salto al video successivo.
- Scene e layout riutilizzabili, canvas con trascinamento, coordinate, dimensioni, ordine dei livelli e selezione delle sorgenti.
- File, schermi, fotocamere e microfoni attraverso **AVFoundation**, usando gli indici rilevati dallo stesso FFmpeg.
- Overlay statici e animati, libreria locale, pulsanti scena, intermezzi, posizione, durata, scorciatoie macOS e ripresa del programma.
- Mixer audio per sorgente, mute e volume 0–200%, audio stereo/silenzio quando necessario.
- Profili video/audio con bitrate, GOP in secondi, risoluzione, fps, formato pixel, contenitore, qualità, preset, tune, bassa latenza, adattamento del bitrate e opzioni FFmpeg.
- **VideoToolbox H.264/HEVC** e alternativa software x264/x265. RTMP è limitato a FLV/H.264 con audio AAC/MP3; HEVC, Opus e altri contenitori sono conservati nei profili ma non vengono accettati per una diretta RTMP.
- Relay FLV Swift con timestamp continui, un processo FFmpeg publisher persistente e produttori sostituibili per playlist, scene, intermezzi e modifiche del mixer.
- Anteprima della composizione, avanzamento di FFmpeg, pagina pubblica della piattaforma in WebKit, registri con server mascherato, riconnessione e watchdog.
- Storico con snapshot, riavvio, esportazione, cancellazione e pulizia periodica per canale.
- Panoramica/attività con CPU, RAM, compressione, swap, rete, disco, uptime, risparmio energetico e stato termico macOS.

## Scelte Apple e differenze da Windows

La UI usa sidebar, toolbar, SF Symbols, form, pannelli file, finestre impostazioni e menu nativi, con tema chiaro/scuro di sistema. Non usa WPF, WebView2, ASP.NET o un server HTTP locale. I vecchi endpoint HTTP non sono mantenuti: le viste chiamano i servizi Swift.

I dati sono in `~/Library/Application Support/OrbisStream/workspace.json`, scritti atomicamente. Overlay e media dei pulsanti sono copiati in Application Support; la playlist usa i file originali. Le chiavi non entrano in JSON, storico o export: trasferendo i dati su un altro Mac devi reinserirle nel Portachiavi. L'importazione accetta il nuovo schema Swift, **non** il database SQLite Windows: gli UUID e il modello di persistenza sono nuovi.

Gli encoder NVENC/QSV/AMF e la cattura gdigrab/dshow sono sostituiti da VideoToolbox/AVFoundation. Il trasporto usa sempre il publisher FFmpeg: il selettore Windows tra RTMP implementato in .NET e FFmpeg non è applicabile. Il relay Swift dosa i pacchetti in tempo reale e mantiene la connessione durante le transizioni; non riproduce esattamente i buffer/preroll specifici del vecchio relay. Cambiando produttore può esserci una breve pausa, e cambiando formato la piattaforma può interrompere le proprie rendizioni.

L'audio di sistema non è acquisito direttamente: AVFoundation cattura il video dello schermo; per l'audio usa un dispositivo virtuale selezionato come microfono. Non è un'implementazione ScreenCaptureKit. Il sistema richiede i permessi camera/microfono/schermo; lo script bundle include le descrizioni di utilizzo.

La qualità Auto equivale al preset bilanciato per gli encoder software; non è ancora un benchmark automatico degli encoder. L'adattamento osserva l'avanzamento del publisher rispetto al tempo reale, con riduzione dopo campioni lenti e recupero dopo stabilità; non misura direttamente la capacità della rete. L'avanzamento FFmpeg prova che il processo sta inviando, **non** che la piattaforma sia pubblicamente in onda: non viene replicato il precedente controllo YouTubeAirWatch tramite scraping. La pagina piattaforma resta consultabile e la UI non afferma una conferma on-air inesistente.

La guida e l'interfaccia di questa versione sono in italiano; i bundle di traduzione e le immagini della guida Windows non sono convertiti. Non è quindi ancora una migrazione con parità integrale: questi punti sono esplicitamente elencati per la verifica prima del rilascio.

## Statistiche macOS

`AppleMetrics` legge CPU con `host_statistics`, memoria con `host_statistics64`, swap con `sysctlbyname`, memoria del processo con `task_info`, CPU del processo con `getrusage` e traffico delle interfacce fisiche `en*` con `getifaddrs`. Swift calcola le differenze tra campioni usando l'uptime; il primo campione mostra `—` dove serve un intervallo. La memoria visualizzata è active + wired + compressed − purgeable, non una copia esatta della formula di Monitoraggio Attività.

CPU del Mac è 0–100%; CPU dell'app può superare 100% (un core). Le risorse dell'app non includono i processi FFmpeg. Il traffico riguarda l'intero Mac ed esclude loopback, tunnel e bridge; non è il bitrate della diretta. Per quest'ultimo si legge `-progress` dal publisher. Il disco indica capacità totale/disponibile, non throughput I/O. Le temperature numeriche CPU/GPU e l'utilizzo GPU sono indisponibili tramite un'API pubblica uniforme: vengono sostituiti dallo stato termico e dal risparmio energetico, senza valori fittizi.

Riferimenti: [FFmpeg AVFoundation](https://ffmpeg.org/ffmpeg-devices.html#avfoundation), [Apple host_statistics64](https://developer.apple.com/documentation/kernel/1502863-host_statistics64).

## Verifica

```sh
swift test
swift build -c release
```

I test coprono riconoscimento piattaforme, URL/chiavi, validazione, composizione/mix, comandi Apple, TikTok portrait, progress, parsing/rebase FLV, persistenza, ordinamento naturale e un output FFmpeg reale con anteprima quando FFmpeg è installato. La CI `.github/workflows/macos.yml` installa FFmpeg, esegue i test e crea il bundle su macOS.

L'ambiente di sviluppo di questa migrazione è Linux: i test del motore possono girare in un toolchain Swift Linux, ma SwiftUI/AppKit, Mach, Portachiavi, permessi TCC e dispositivi devono essere compilati e provati sul Mac. La presenza della CI non equivale a un'esecuzione già riuscita. Prima di considerare pronta la release, verifica sul Mac camera/microfono/schermo, alpha animato, più dirette, transizioni e invio a un ingest di prova.
