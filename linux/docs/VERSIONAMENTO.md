# Versionamento del port Linux

`linux/VERSION` è la sola versione da modificare per un rilascio. CMake la usa per il nome del pacchetto Debian, genera `version.hpp` per `orbis-stream --version`, per l'intestazione del programma e per `/api/bootstrap`. Il browser mostra il valore ricevuto dall'API.

Ogni rilascio registra le modifiche in `linux/CHANGELOG.md`; il comando `bash scripts/create-deb.sh` controlla che binario e `VERSION` coincidano, poi verifica il pacchetto estratto.

La versione Linux è indipendente dalla versione Windows: il checkout Windows esaminato è **2.1.2**, mentre il numero Linux misura i rilasci del port, non certifica parità. Aumentare PATCH per correzioni compatibili, MINOR per nuove funzioni o flussi recuperati, MAJOR per modifiche incompatibili dei dati/API. Ogni rilascio richiede compilazione, test e verifica del `.deb` estratto.

La serie iniziale `0.0.x` registra gli avanzamenti del port ancora incompleto: la versione corrente è `0.0.4`. Passare a `0.1.0` quando il primo insieme concordato di funzioni ha una verifica completa; `1.0.0` richiede il raggiungimento del contratto funzionale dichiarato, non solo la riuscita della compilazione.

La versione dello schema dati è separata: `database_schema.cpp` usa `PRAGMA user_version=1`. Il database viene inizializzato una volta; uno schema Windows o più nuovo viene rifiutato prima di creare tabelle Linux nello stesso file. Le future migrazioni devono avanzare `user_version` in transazione e mantenere la lettura dei dati esistenti.

Il riferimento file per file è in `PORTING_MAP.md`: ogni voce deve passare da `MANCANTE` a `PARZIALE` e poi a `VERIFICATO` solo con un comportamento equivalente e una prova. I file Windows possono cambiare indipendentemente: rigenerare il loro inventario con `scripts/sync-windows.py` prima di aggiornare lo stato di parità.
