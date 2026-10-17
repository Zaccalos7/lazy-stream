# Percorsi e funzioni macOS

Aggiornamento: 18 ottobre 2026. Il riferimento è il progetto Windows disponibile nella cartella accanto (`lazy-stream-windows/src/Orbis.Stream.Core/Pages`). L’interfaccia resta SwiftUI: sidebar di sistema, schede adattive, toolbar unificata, pannelli e Portachiavi macOS.

| Operazione | Percorso Mac, corrispondente al flusso Windows |
|---|---|
| Gestire le trasmissioni | Panoramica → Gestione Live oppure Live → Gestione Live |
| Avviare una scena | Live → Avvia Live → Video e audio → Configurazione streaming → Componi scena → countdown |
| Avviare una cartella | Live → Avvia playlist → Video e audio → Configurazione streaming → Cartella video → countdown |
| Controllare il segnale | Live / Anteprima, anche dalla riga della diretta |
| Configurare la codifica | Impostazioni → Video e audio, anche dalla Panoramica |
| Configurare le destinazioni | Impostazioni → Canali, anche dalla Panoramica |
| Preparare la libreria | Live → Playlist / Pulsanti scena / Overlay; Scene e layout nella sidebar |
| Consultare lo storico | Live → Storico, anche dalla Panoramica |
| Recuperare una sessione | Live → Recupero oppure Riprendi dalla riga salvata |
| Controllare i processi | Attività dalla sidebar o dalla Panoramica |
| Consultare/stampare la guida | Manuale → Indice oppure Stampa / Salva PDF |
| Aspetto, lingua, dati e aggiornamenti | Impostazioni → Preferenze; tema e lingua anche nella sidebar |

Il pulsante Indietro conserva il percorso tra le sezioni; le ricerche si azzerano quando si cambia pagina. La toolbar e il menu Diretta espongono entrambi gli avvii. L’avvio da Canali o Playlist usa lo stesso wizard.

Gestione Live legge le trasmissioni salvate, anche dopo un riavvio dell’app. Offre filtri Stato/Canale/Piattaforma, dieci righe per pagina e azioni per dettagli, anteprima, pulsanti scena, spot, stop, associazione di profilo, ripresa, riavvio, segnalazione ed eliminazione. La ripresa conserva il video raggiunto e il timestamp; Ricomincia azzera i seek dello snapshot. Il checkpoint viene aggiornato anche con profilo, scena e playlist correnti.

Una nuova configurazione contiene i dieci preset standard/leggeri di Twitch, YouTube, Kick, Facebook e TikTok presenti nel riferimento Windows. Dimensioni, fps e bitrate corrispondono; l’encoder è Apple VideoToolbox. Il preset Twitch standard conserva risoluzione/fps della sorgente. Per configurazioni precedenti, Video e audio → Aggiungi preset mancanti aggiunge solo le combinazioni piattaforma/livello assenti. I profili esistenti vengono conservati.

Canali offre il selettore della piattaforma e compila il server del riferimento Windows; il campo resta modificabile per gli ingest personali. La chiave TikTok continua a essere richiesta per ciascuna diretta. La validazione RTMP precede l’avvio o la sostituzione del profilo in onda.

Il mixer comprende anche l’audio della playlist, con volume 0–200% e mute. Le segnalazioni di errore mostrano testo e QR suddivisi per byte UTF-8, con copia/esportazione e apertura della pagina issue. Chiavi, token noti e URL RTMP vengono mascherati prima dei registri e del report. Il testo delle nuove azioni riusa i cataloghi condivisi nelle 21 lingue; le descrizioni specifiche Mac senza traduzione usano l’italiano. L’arabo imposta il layout da destra a sinistra.

## Verifica

Su Linux: parsing della sintassi di tutti i sorgenti/test Swift, controllo di JSON e cataloghi `.strings`, sintassi Bash e `git diff --check`. Questo non equivale a una compilazione SwiftUI.

Su un Mac Apple Silicon, eseguire `./scripts/build-macos.sh`: prima del pacchetto esegue tutti i test Core e Mac, inclusi `WorkflowParityTests` (preset, ripresa/riavvio, vecchi documenti, profili RTMP, redazione, suddivisione Unicode) e il controllo del mixer playlist in `LiveSessionTests`.

Le seguenti prove native restano da eseguire:

1. Aprire ogni percorso della tabella, tornare indietro e verificare toolbar, pannelli e dimensioni delle finestre.
2. Avviare una scena e una playlist, annullare un countdown e verificare più canali contemporanei.
3. Cambiare profilo, volume, scena e video durante la diretta; inserire due spot e riprendere il programma senza chiudere il publisher.
4. Interrompere/riprendere una playlist dopo il primo video; riaprire l’app e verificare ripresa, timestamp e riavvio dall’inizio.
5. Provare catture camera/schermo/microfono, overlay animati e pubblicazione sulle destinazioni configurate.
6. Leggere i QR con caratteri Unicode e verificare copia/esportazione, cambio lingua/tema e stampa/PDF.

La parità operativa, la compilazione nativa e l’installer restano da validare sul Mac; il censimento di codice in `parity.json` non costituisce una certificazione di queste prove.
