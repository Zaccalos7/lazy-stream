# Mappa file per file: Windows → Linux

Riferimento: `windows-inventory.json`, checkout Windows locale aggiornato il 16 ottobre 2026. Generare di nuovo con `python3 scripts/porting_map.py` dopo aver aggiornato l'inventario.

`COPIATO` significa hash identico; `PARZIALE` significa solo che esiste codice Linux correlato, non che il comportamento sia equivalente; `MANCANTE` richiede implementazione; `RIFERIMENTO` indica documenti o file specifici di Windows; `ESCLUSO` è fuori ambito su richiesta e `ADATTATO` mantiene le risorse salvo l'esclusione richiesta. Nessun file di codice è segnato come verificato sulla sola base del nome.

Totale 272 file: ADATTATO 63, COPIATO 1, ESCLUSO 2, MANCANTE 11, PARZIALE 193, RIFERIMENTO 2.

| File Windows | SHA-256 (12) | File Linux | Stato | Lavoro residuo |
|---|---|---|---|---|
| `Installer/OrbisStream.wxs` | `97a815cece7f` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/build-installer.ps1` | `d7871ce81376` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/build-installer.sh` | `e51e45fb4a9a` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/build-msi.ps1` | `2fcb6a512d29` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/fetch-ffmpeg.ps1` | `9ffc0816dd8d` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/fetch-ffmpeg.sh` | `8062f59267b1` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/installer.nsi` | `1239d703e57e` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `LICENSE` | `559fd2eab22f` | web/resources/LICENSE-windows.txt | COPIATO | Byte identici all'inventario |
| `README.md` | `5b3a75405baa` | README.md | PARZIALE | Manuale Linux specifico |
| `samples/overlays/README.md` | `b73da3f7d115` | — | RIFERIMENTO | File Windows senza traduzione di codice diretta |
| `sed_script.sh` | `39affaa82590` | — | RIFERIMENTO | File Windows senza traduzione di codice diretta |
| `src/Orbis.Stream.App/App.xaml.cs` | `fa306ba1e768` | src/main.cpp; src/native_window.cpp | PARZIALE | Shell Qt nativa predefinita; API solo in headless |
| `src/Orbis.Stream.App/AppText.cs` | `d32b2ac1363c` | src/main.cpp; src/native_window.cpp | PARZIALE | Shell Qt nativa predefinita; API solo in headless |
| `src/Orbis.Stream.App/AssemblyInfo.cs` | `3fac2af92385` | src/main.cpp; src/native_window.cpp | PARZIALE | Shell Qt nativa predefinita; API solo in headless |
| `src/Orbis.Stream.App/MainWindow.xaml.cs` | `00acaad9f97b` | src/main.cpp; src/native_window.cpp | PARZIALE | Shell Qt nativa predefinita; API solo in headless |
| `src/Orbis.Stream.App/Orbis.Stream.App.csproj` | `47c879e7516a` | CMakeLists.txt; VERSION | PARZIALE | Build e versione Linux indipendenti |
| `src/Orbis.Stream.App/wwwroot/css/base.css` | `5db6e559e01e` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/cleanup.css` | `9a4a2966b57d` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/composer.css` | `455d58d4c336` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/controls.css` | `df9caed8b476` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/countdown.css` | `fcbd69f01e27` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/dashboard.css` | `13961623f1f2` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/feedback.css` | `d9cba1c879c3` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/layout.css` | `55b191daaea9` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/manual.css` | `90306123fa2e` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/meters.css` | `f64ce9cd08c1` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/overlays.css` | `3c47dc0d47fd` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/preview.css` | `921fc6a25460` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/scenes.css` | `72e1e16e57f5` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/surfaces.css` | `0b2f7af39ad6` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/css/tokens.css` | `31e9770fc711` | web/style.css | PARZIALE | Riferimento CSS conservato; la GUI corrente usa stile e widget Qt |
| `src/Orbis.Stream.App/wwwroot/js/composer.js` | `a0007790f4bb` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; web/app.js | PARZIALE | Interazioni trasferite in Qt; JavaScript conservato come riferimento |
| `src/Orbis.Stream.App/wwwroot/js/scenes.js` | `e783d7c11821` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; web/app.js | PARZIALE | Interazioni trasferite in Qt; JavaScript conservato come riferimento |
| `src/Orbis.Stream.App/wwwroot/js/site.js` | `03660218c068` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; web/app.js | PARZIALE | Interazioni trasferite in Qt; JavaScript conservato come riferimento |
| `src/Orbis.Stream.Core/Configuration/OrbisRuntimeOptions.cs` | `8c7b25ccd306` | src/main.cpp | PARZIALE | Opzioni runtime da confrontare |
| `src/Orbis.Stream.Core/Contracts/LocalDateTimeConverter.cs` | `2ec0b5a48275` | src/server.cpp; src/core.cpp | PARZIALE | Contratto HTTP Linux ancora diverso |
| `src/Orbis.Stream.Core/Contracts/Requests.cs` | `c5399c9fa182` | src/server.cpp; src/core.cpp | PARZIALE | Contratto HTTP Linux ancora diverso |
| `src/Orbis.Stream.Core/Data/DatabaseSchema.cs` | `64cae458a1fb` | src/database_schema.cpp; src/core.cpp | PARZIALE | Schema e migrazioni Linux separati |
| `src/Orbis.Stream.Core/Data/Paging.cs` | `dc3b222e371c` | src/core.cpp | PARZIALE | Schema SQLite Linux diverso da quello Windows |
| `src/Orbis.Stream.Core/Data/SceneButtonRepository.cs` | `e6007f2efc53` | src/core.cpp | PARZIALE | Schema SQLite Linux diverso da quello Windows |
| `src/Orbis.Stream.Core/Data/SceneRepository.cs` | `1875e00f5f94` | src/core.cpp | PARZIALE | Schema SQLite Linux diverso da quello Windows |
| `src/Orbis.Stream.Core/Data/SettingRepository.cs` | `9aa559188378` | src/core.cpp | PARZIALE | Schema SQLite Linux diverso da quello Windows |
| `src/Orbis.Stream.Core/Data/SqlQueryBuilder.cs` | `680e539f8181` | src/core.cpp | PARZIALE | Schema SQLite Linux diverso da quello Windows |
| `src/Orbis.Stream.Core/Data/SqliteConnectionFactory.cs` | `21641913024c` | src/core.cpp | PARZIALE | Schema SQLite Linux diverso da quello Windows |
| `src/Orbis.Stream.Core/Data/VideoRepository.cs` | `084b050db76c` | src/core.cpp | PARZIALE | Schema SQLite Linux diverso da quello Windows |
| `src/Orbis.Stream.Core/Data/VideoSettingRepository.cs` | `5e50879b52f0` | src/core.cpp | PARZIALE | Schema SQLite Linux diverso da quello Windows |
| `src/Orbis.Stream.Core/Domain/Entities.cs` | `2547a218fd41` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/Enums.cs` | `6f78cc3e98d9` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/FfmpegCodecCatalog.cs` | `f2a4bc27e24c` | web/resources/codec-catalog.json; scripts/sync-codec-catalog.py; src/native_live_pages.cpp | PARZIALE | Cataloghi selezionabili; codec, pixel e AVOption risolti da FFmpeg 9.0.2; hardware dipendente da OS/driver |
| `src/Orbis.Stream.Core/Domain/LiveStatus.cs` | `e64bfa04b071` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/SceneButton.cs` | `7c1db8246cd5` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/SourceKind.cs` | `96bf971f8624` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/VideoSettingAdaptiveBitrate.cs` | `a251a8a05f16` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/VideoSettingLatency.cs` | `c679cf611509` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/VideoSettingQuality.cs` | `94a0a4e52577` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Hosting/AutoCleanupService.cs` | `a274344a1ed5` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/DatabaseBootstrapper.cs` | `659fcd9a0a7f` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/EncoderTuningStartup.cs` | `86630f7803fb` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/OrbisEndpoints.cs` | `9d9d43c245fc` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/OrbisHost.cs` | `d287f870dc5e` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/OrbisServiceCollectionExtensions.cs` | `3930a809e5e2` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/OrbisWebApplicationExtensions.cs` | `e7c1e7cffc16` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/RequestValidator.cs` | `1333e931de4b` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Http/ExceptionHandlingMiddleware.cs` | `f9c7f1f40991` | src/server.cpp | PARZIALE | Errori e formato risposte da confrontare |
| `src/Orbis.Stream.Core/Http/OrbisExceptions.cs` | `2d55ba10bbc4` | src/server.cpp | PARZIALE | Errori e formato risposte da confrontare |
| `src/Orbis.Stream.Core/Http/ResponseFactory.cs` | `6cb62c591818` | src/server.cpp | PARZIALE | Errori e formato risposte da confrontare |
| `src/Orbis.Stream.Core/I18n/Localizer.cs` | `1d9a0a0888db` | src/native_i18n.cpp; src/native_i18n.hpp; web/resources/ui; tests/native_tests.cpp | PARZIALE | 21 cataloghi riusati nella GUI Qt; revisione linguistica non certificata |
| `src/Orbis.Stream.Core/I18n/ManualText.cs` | `dda99dc08a71` | src/native_i18n.cpp; src/native_i18n.hpp; web/resources/ui; tests/native_tests.cpp | PARZIALE | 21 cataloghi riusati nella GUI Qt; revisione linguistica non certificata |
| `src/Orbis.Stream.Core/I18n/MessageCatalog.cs` | `88fe4c10f7b6` | src/native_i18n.cpp; src/native_i18n.hpp; web/resources/ui; tests/native_tests.cpp | PARZIALE | 21 cataloghi riusati nella GUI Qt; revisione linguistica non certificata |
| `src/Orbis.Stream.Core/I18n/MessageFormatter.cs` | `7470194897ea` | src/native_i18n.cpp; src/native_i18n.hpp; web/resources/ui; tests/native_tests.cpp | PARZIALE | 21 cataloghi riusati nella GUI Qt; revisione linguistica non certificata |
| `src/Orbis.Stream.Core/I18n/PropertiesBundle.cs` | `0957396c7870` | src/native_i18n.cpp; src/native_i18n.hpp; web/resources/ui; tests/native_tests.cpp | PARZIALE | 21 cataloghi riusati nella GUI Qt; revisione linguistica non certificata |
| `src/Orbis.Stream.Core/I18n/UiText.cs` | `89e5578cee3f` | src/native_i18n.cpp; src/native_i18n.hpp; web/resources/ui; tests/native_tests.cpp | PARZIALE | 21 cataloghi riusati nella GUI Qt; revisione linguistica non certificata |
| `src/Orbis.Stream.Core/Logging/FileLoggerProvider.cs` | `6856ecbe669a` | src/async_logger.cpp; tests/streaming_tests.cpp | PARZIALE | Coda limitata, file giornaliero e conservazione 3 giorni; integrazione logging Qt da verificare |
| `src/Orbis.Stream.Core/Messages/messages_ar.properties` | `616d6796a11d` | web/resources/messages/messages_ar.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_da.properties` | `98df690ee0cd` | web/resources/messages/messages_da.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_de.properties` | `84931f19fd16` | web/resources/messages/messages_de.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_el.properties` | `27ddd1119533` | web/resources/messages/messages_el.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_en.properties` | `dfad2f5658bc` | web/resources/messages/messages_en.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_es.properties` | `cd534692e38f` | web/resources/messages/messages_es.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_fil.properties` | `cb898ea867f8` | web/resources/messages/messages_fil.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_fr.properties` | `c879860d5783` | web/resources/messages/messages_fr.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_hi.properties` | `65300943c47e` | web/resources/messages/messages_hi.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_hod.properties` | `018e00334d3f` | web/resources/messages/messages_hod.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_it.properties` | `a1be8c67ba4a` | web/resources/messages/messages_it.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_ja.properties` | `17cc6dd36e01` | web/resources/messages/messages_ja.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_ko.properties` | `3dfad663fb68` | web/resources/messages/messages_ko.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_la.properties` | `800b18fb1b63` | web/resources/messages/messages_la.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_nl.properties` | `798c3343fe72` | web/resources/messages/messages_nl.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_pt.properties` | `45a6dea270fc` | web/resources/messages/messages_pt.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_ro.properties` | `9e8ce18fe985` | web/resources/messages/messages_ro.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_ru.properties` | `972c44419d9f` | web/resources/messages/messages_ru.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_tlh.properties` | `0ba0c49c5b4f` | web/resources/messages/messages_tlh.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_tr.properties` | `8199f7b5a97e` | web/resources/messages/messages_tr.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Messages/messages_zh.properties` | `3bbf0249be6f` | web/resources/messages/messages_zh.properties | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Orbis.Stream.Core.csproj` | `c0c1daa9b68b` | CMakeLists.txt; VERSION | PARZIALE | Build e versione Linux indipendenti |
| `src/Orbis.Stream.Core/Pages/Countdown.cshtml` | `c9e582af3286` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/MainChannelSetting.cshtml` | `a1f126e04049` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/MainChannelSetting.html; web/app.js | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/MainChannelSetting.cshtml.cs` | `2de8ec6d74ab` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp | PARZIALE | Flusso trasferito in Qt; validazioni da confrontare |
| `src/Orbis.Stream.Core/Pages/MainLayout.cshtml` | `77af0c2dc526` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/MainLayout.cshtml.cs` | `f8644bb17f1d` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp | PARZIALE | Flusso trasferito in Qt; validazioni da confrontare |
| `src/Orbis.Stream.Core/Pages/MainLayoutList.cshtml` | `6654595ff035` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/MainLayoutList.html; web/app.js | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/MainLayoutList.cshtml.cs` | `9a3a894cb7f3` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp | PARZIALE | Flusso trasferito in Qt; validazioni da confrontare |
| `src/Orbis.Stream.Core/Pages/MainLive.cshtml` | `799efcc684da` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/MainLive.html; web/app.js; src/live_rows.cpp | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/MainLive.cshtml.cs` | `b8ba64eabda5` | src/live_rows.cpp; src/server.cpp; web/app.js | PARZIALE | Righe/snapshot e wizard portati; locking video e notifiche push Windows ancora diversi |
| `src/Orbis.Stream.Core/Pages/MainLiveHistory.cshtml` | `279800c96455` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/MainLiveHistory.html; web/app.js | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/MainLiveHistory.cshtml.cs` | `37827f538bde` | src/history_cleanup_service.cpp; src/server.cpp; web/app.js | PARZIALE | Filtri e pulizia portati; modello dati e scene temporanee ancora diversi |
| `src/Orbis.Stream.Core/Pages/MainLiveMenu.cshtml` | `d293f2ca8f85` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/MainLiveMenu.html; web/app.js | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/MainLiveMenu.cshtml.cs` | `5e73dc0c7384` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp | PARZIALE | Flusso trasferito in Qt; validazioni da confrontare |
| `src/Orbis.Stream.Core/Pages/MainManual.cshtml` | `f1fe933e3a68` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/MainMenu.cshtml` | `9aa25a715fc1` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/MainMenu.html; web/app.js | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/MainPreview.cshtml` | `0632dc643c84` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/MainPreview.html; web/app.js | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/MainPreview.cshtml.cs` | `b87290dd2698` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp | PARZIALE | Flusso trasferito in Qt; validazioni da confrontare |
| `src/Orbis.Stream.Core/Pages/MainSetting.cshtml` | `d4626e99d286` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/MainSetting.html; web/app.js | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/MainTaskManager.cshtml` | `13d638252bd6` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/MainTaskManager.cshtml.cs` | `f582840e9851` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp | PARZIALE | Flusso trasferito in Qt; validazioni da confrontare |
| `src/Orbis.Stream.Core/Pages/MainVideoSetting.cshtml` | `5ee7df24533b` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/MainVideoSetting.html; web/app.js | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/MainVideoSetting.cshtml.cs` | `e13d12b75bfa` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp | PARZIALE | Flusso trasferito in Qt; validazioni da confrontare |
| `src/Orbis.Stream.Core/Pages/OrbisPageModel.cs` | `b48f328ee53b` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp | PARZIALE | Flusso trasferito in Qt; validazioni da confrontare |
| `src/Orbis.Stream.Core/Pages/PageHelpers.cs` | `5440effbe298` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp | PARZIALE | Flusso trasferito in Qt; validazioni da confrontare |
| `src/Orbis.Stream.Core/Pages/Shared/_Layout.cshtml` | `2933577f35de` | web/index.html; web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/Shared/_Pager.cshtml` | `d36587b61f29` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/Shared/_SceneDeckIcon.cshtml` | `1a4cbf0f3cc3` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/Shared/_SpotIcon.cshtml` | `61a95cdcab2c` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/Shared/_VideoSettingFields.cshtml` | `9664431a7b86` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/VideoSettingForm.cs` | `d26c40454b8d` | src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp | PARZIALE | Flusso trasferito in Qt; validazioni da confrontare |
| `src/Orbis.Stream.Core/Pages/_AppStats.cshtml` | `a954523575b3` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_ErrorReportButton.cshtml` | `bbc6b26d72dc` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_ErrorReportDialog.cshtml` | `b2c54eba23d0` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_FacebookLiveNotice.cshtml` | `86c000edfadb` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_LiveRows.cshtml` | `106076c789d8` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/_LiveRows.html; web/app.js; src/live_rows.cpp | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/_PlaylistVideos.cshtml` | `c2114899553e` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_SceneComposer.cshtml` | `07b1dfe98aab` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_SceneDeck.cshtml` | `49272363885d` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_StartWizardConfiguration.cshtml` | `84f4ccd84b96` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/_StartWizardConfiguration.html; web/app.js | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/_StartWizardSetting.cshtml` | `9fd3b6e00f20` | src/native_window.cpp; src/native_live_pages.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; web/pages/_StartWizardSetting.html; web/app.js | PARZIALE | Flusso trasferito in widget Qt; HTML conservato come riferimento |
| `src/Orbis.Stream.Core/Pages/_Stats.cshtml` | `e9a874b1645f` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_SystemInfo.cshtml` | `dffad664bd3d` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_ViewImports.cshtml` | `47666ab1e2ad` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_ViewStart.cshtml` | `62afaee35009` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Services/BackgroundTaskExecutor.cs` | `4c33ff84e227` | src/stream.cpp; src/media_job.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/EncoderTuningService.cs` | `b9d11e500f2b` | src/media_job.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/ErrorReport.cs` | `1707cf808fc6` | src/native_dialogs.cpp; third_party/qrcodegen | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/FfmpegVideoPlaylistStreamer.cs` | `8723f585dfd4` | src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/ImageService.cs` | `b20cd1bf7565` | src/media_job.cpp; src/server.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/LiveChangeNotifier.cs` | `d697b45b225d` | src/change_notifier.hpp; src/server.cpp; src/native_window.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/LiveHistoryCleanupService.cs` | `e466d6528540` | src/history_cleanup_service.cpp; src/history_cleanup_service.hpp; src/core.cpp; src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/LivePreviewService.cs` | `e395a8fafd55` | src/stream.cpp; src/server.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/LiveTakeovers.cs` | `405760070cf7` | src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/MediaProxyService.cs` | `bea84f48da7a` | src/media_job.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/MediaStore.cs` | `3fbf55b6406a` | src/server.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/NaturalFileNameComparer.cs` | `fbc052306b42` | src/core.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/OverlayLibrary.cs` | `e038d59d70c2` | src/server.cpp; src/core.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/SceneButtonService.cs` | `b058300c9e58` | src/core.cpp; src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/SceneReference.cs` | `9ca14a4235ac` | src/core.cpp; src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/SceneRows.cs` | `5da9edda45d7` | src/core.cpp; src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/SceneService.cs` | `a96b3d196241` | src/scene_service.cpp; src/scene_service.hpp; src/server.cpp | PARZIALE | CheckSlots/CheckLayout e protezione storico portati; contratto e repository ancora diversi |
| `src/Orbis.Stream.Core/Services/SettingService.cs` | `640601fe09bb` | src/core.cpp; src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/SourceCatalogService.cs` | `841669fc185b` | src/core.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/SourceSnapshotService.cs` | `0756164a2229` | src/server.cpp; src/core.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/StreamingService.cs` | `d7036800eb60` | src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/SystemInfoService.cs` | `c88c3c1cac5f` | src/core.cpp; src/metrics.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/VideoService.cs` | `39fa78e88985` | src/core.cpp; src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/VideoSettingService.cs` | `a2e2d72ceb02` | src/core.cpp; src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/YouTubeAirState.cs` | `856d1865b39b` | src/platform_lookup.cpp; src/platform_lookup.hpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/YouTubeAirWatch.cs` | `295888495f26` | src/youtube_air_watch.cpp; src/platform_lookup.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/BitrateLadder.cs` | `3c82f41089e9` | src/rate_control.cpp; tests/streaming_tests.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FfmpegCommandBuilder.cs` | `263611b9e5ab` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FfmpegProbe.cs` | `acb2bc49ff0f` | src/core.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FfmpegProcessLog.cs` | `639c0b206293` | src/core.cpp; src/async_logger.cpp; src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FfmpegStreamingSession.cs` | `27a58730fabc` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FlvPacedRelay.cs` | `fb7cb62aa367` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FlvSink.cs` | `c5f04505ca11` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/HybridWaiter.cs` | `035df7679ae2` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/LiveOutput.cs` | `fe8328cf3947` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/LivePlatformEmbeds.cs` | `fb2f6fae89d6` | src/platform_lookup.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/LivePreviewFrames.cs` | `69f20244e11e` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/LiveStreamKeys.cs` | `7a1b1c73e39f` | — | ESCLUSO | Piattaforma rimossa dal port C++ su richiesta; chiavi temporanee e test dedicati non applicabili |
| `src/Orbis.Stream.Core/Streaming/MonitorTarget.cs` | `424061f06190` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/NvencOptions.cs` | `1db4ee9be433` | src/encoder_options.cpp; tests/streaming_tests.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/OverlayMedia.cs` | `6f43024675d5` | src/overlay_media.cpp; src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/Rtmp/Amf0.cs` | `e8e9217a1945` | src/paced_output.cpp; CMakeLists.txt; tests/integration.py | PARZIALE | Trasporto nativo mediante API pubbliche libavformat 9.0.2; RTMP locale provato, RTMPS su account da verificare |
| `src/Orbis.Stream.Core/Streaming/Rtmp/RtmpPublisher.cs` | `c00dd2455ef6` | src/paced_output.cpp; CMakeLists.txt; tests/integration.py | PARZIALE | Trasporto nativo mediante API pubbliche libavformat 9.0.2; RTMP locale provato, RTMPS su account da verificare |
| `src/Orbis.Stream.Core/Streaming/StreamPlatform.cs` | `b0f3024c8814` | src/stream_platform.cpp; src/stream.cpp; src/media_job.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/WebpOverlayInputs.cs` | `6a6474308c92` | src/overlay_media.cpp; tests/streaming_tests.cpp | PARZIALE | Copie private, loop ANIM FFmpeg 9; animazione, alpha e ricorrenza provate da delivery.py |
| `src/Orbis.Stream.Core/SystemInfo/AppInfo.cs` | `25db9d2ee8f3` | src/metrics.cpp; src/core.cpp | PARZIALE | Metriche con contatori specifici di Linux |
| `src/Orbis.Stream.Core/SystemInfo/SystemInfoProviders.cs` | `0c4e8ab6c32d` | src/metrics.cpp; src/core.cpp | PARZIALE | Metriche con contatori specifici di Linux |
| `src/Orbis.Stream.Core/Ui/manual.ar.json` | `5d7ca8f057e0` | web/resources/ui/manual.ar.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.da.json` | `ea3b90448aaa` | web/resources/ui/manual.da.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.de.json` | `b092b1e9c9e9` | web/resources/ui/manual.de.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.el.json` | `5a8df4f23d0b` | web/resources/ui/manual.el.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.en.json` | `928d18add074` | web/resources/ui/manual.en.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.es.json` | `8e0a1bf4ef4c` | web/resources/ui/manual.es.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.fil.json` | `2efeee8d5b9d` | web/resources/ui/manual.fil.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.fr.json` | `f327270b404f` | web/resources/ui/manual.fr.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.hi.json` | `7586a040cd72` | web/resources/ui/manual.hi.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.hod.json` | `c781cc9e1840` | web/resources/ui/manual.hod.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.it.json` | `1fc5e8576e4b` | web/resources/ui/manual.it.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.ja.json` | `0ae529d9057f` | web/resources/ui/manual.ja.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.ko.json` | `7696137ef0e5` | web/resources/ui/manual.ko.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.la.json` | `14b8b3d1802f` | web/resources/ui/manual.la.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.nl.json` | `cd83973bd7a4` | web/resources/ui/manual.nl.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.pt.json` | `db25d894db91` | web/resources/ui/manual.pt.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.ro.json` | `76e37c08bd52` | web/resources/ui/manual.ro.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.ru.json` | `6cc400828b1d` | web/resources/ui/manual.ru.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.tlh.json` | `d8906802baa5` | web/resources/ui/manual.tlh.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.tr.json` | `3d5a71bc98d1` | web/resources/ui/manual.tr.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/manual.zh.json` | `d98cd5498437` | web/resources/ui/manual.zh.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.ar.json` | `848b3be5216f` | web/resources/ui/ui.ar.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.da.json` | `7228ecc25869` | web/resources/ui/ui.da.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.de.json` | `dda04cba3cee` | web/resources/ui/ui.de.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.el.json` | `7e804d778caa` | web/resources/ui/ui.el.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.en.json` | `c11eedaeaf00` | web/resources/ui/ui.en.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.es.json` | `6294018f1646` | web/resources/ui/ui.es.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.fil.json` | `af148299b1a4` | web/resources/ui/ui.fil.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.fr.json` | `3cda01d23502` | web/resources/ui/ui.fr.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.hi.json` | `9dedf4ab7e03` | web/resources/ui/ui.hi.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.hod.json` | `07b3c3036d9b` | web/resources/ui/ui.hod.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.it.json` | `f72273b642b4` | web/resources/ui/ui.it.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.ja.json` | `1907e33ba924` | web/resources/ui/ui.ja.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.ko.json` | `6e47983814d8` | web/resources/ui/ui.ko.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.la.json` | `fce05409d0b9` | web/resources/ui/ui.la.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.nl.json` | `a71a19fa40b9` | web/resources/ui/ui.nl.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.pt.json` | `6c2eec42f641` | web/resources/ui/ui.pt.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.ro.json` | `70417c6fc3f4` | web/resources/ui/ui.ro.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.ru.json` | `e0287f885cd3` | web/resources/ui/ui.ru.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.tlh.json` | `acf80379122c` | web/resources/ui/ui.tlh.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.tr.json` | `9c4ba8f5a1cb` | web/resources/ui/ui.tr.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Core/Ui/ui.zh.json` | `2fe8d533d8c4` | web/resources/ui/ui.zh.json | ADATTATO | Risorse Windows con esclusione della piattaforma rimossa; confronto automatico in parity-scope |
| `src/Orbis.Stream.Tests/AdaptiveBitrateSwitchTests.cs` | `9c394fce1a0a` | tests/streaming_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/AnimatedWebpTests.cs` | `56432b4ce11b` | tests/delivery.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/BitrateLadderTests.cs` | `394d5c7ff3d4` | tests/streaming_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/DatabaseTests.cs` | `fb05441e02c0` | tests/core_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/DomainAndPagingTests.cs` | `9910787dc860` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/EncoderQualityTests.cs` | `91b146afd077` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/EndToEndApiTests.cs` | `be497cd7572e` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/ErrorReportTests.cs` | `a0f8ab0728de` | tests/native_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/FfmpegCommandBuilderTests.cs` | `f9cbfb3009f4` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/FfmpegProcessLogTests.cs` | `9a5d9a6bc762` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/FfmpegStreamingTests.cs` | `343b74e35a22` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/FileLoggerTests.cs` | `10c1d0085bd6` | tests/streaming_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/I18nTests.cs` | `62603695650a` | tests/native_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/LiveFrameTests.cs` | `2e6ca695e7e2` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/LiveHistoryCleanupTests.cs` | `088e814eadc8` | tests/core_tests.cpp; tests/integration.py; tests/native_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/LivePlatformEmbedTests.cs` | `2c0b22b83e69` | tests/streaming_tests.cpp; tests/integration.py; tests/platform_preview_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/LivePreviewFrameFileTests.cs` | `80c84902bf22` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/LivePreviewTests.cs` | `ab6d424c8200` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/ManualTextTests.cs` | `772d15368673` | tests/native_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/MediaProxyTests.cs` | `97d67f232b36` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/MessageCatalogTests.cs` | `90fa645b736a` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/MessageCodeTests.cs` | `2d7135e986bc` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/Orbis.Stream.Tests.csproj` | `da09e6f5e1d6` | CMakeLists.txt; VERSION | PARZIALE | Build e versione Linux indipendenti |
| `src/Orbis.Stream.Tests/OverlayCommandTests.cs` | `1c7299401d4e` | tests/streaming_tests.cpp; tests/delivery.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/OverlayLiveTests.cs` | `bfe269cbc88a` | tests/delivery.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/PacedRelayTests.cs` | `1b85e0e6c1e1` | tests/core_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/PlaylistTests.cs` | `0b796250ace8` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/RelayTransportTests.cs` | `962903d6f9c9` | tests/delivery.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/RtmpPublisherTests.cs` | `ed48799d0893` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/RuntimeOptionsTests.cs` | `36724209ac13` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/SceneButtonApiTests.cs` | `b839fbc513c4` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/SceneDeckTests.cs` | `7cbad58455ea` | tests/browser.mjs | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/SceneLiveTests.cs` | `00afc1d9cc8d` | tests/native_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/SceneSwitchLiveTests.cs` | `c7a22cf6f60e` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/SharedPublisherStateTests.cs` | `30495b970bec` | tests/integration.py; tests/native_shutdown.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/StreamingPlatformPageTests.cs` | `7bd551f04658` | tests/native_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/StyleSheetTests.cs` | `52541b8ed7f4` | tests/browser.mjs | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/TestPictures.cs` | `778545808506` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/TikTokTests.cs` | `0ff2bc64666d` | — | ESCLUSO | Piattaforma rimossa dal port C++ su richiesta; chiavi temporanee e test dedicati non applicabili |
| `src/Orbis.Stream.Tests/UiTextTests.cs` | `0aa7ebc96b80` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/WebpOverlayInputsTests.cs` | `943d47c9b9f8` | tests/streaming_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/YouTubeAirWatchTests.cs` | `06ab0e3ae7d3` | tests/streaming_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/YouTubeDeliveryTests.cs` | `fbf0df0191d0` | tests/delivery.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/YouTubeProbeTests.cs` | `e8bffd72b0ed` | — | MANCANTE | Test equivalente non presente |
