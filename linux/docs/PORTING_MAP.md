# Mappa file per file: Windows → Linux

Riferimento: `windows-inventory.json`, checkout Windows 2.1.2 del 12 ottobre 2026. Generare di nuovo con `python3 scripts/porting_map.py` dopo aver aggiornato l'inventario.

`COPIATO` significa hash identico; `PARZIALE` significa solo che esiste codice Linux correlato, non che il comportamento sia equivalente; `MANCANTE` richiede implementazione; `RIFERIMENTO` indica documenti o file specifici di Windows. Nessun file di codice è segnato come verificato sulla sola base del nome.

Totale 257 file: COPIATO 64, MANCANTE 31, PARZIALE 160, RIFERIMENTO 2.

| File Windows | SHA-256 (12) | File Linux | Stato | Lavoro residuo |
|---|---|---|---|---|
| `Installer/OrbisStream.wxs` | `97a815cece7f` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/build-installer.ps1` | `d7871ce81376` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/build-installer.sh` | `e51e45fb4a9a` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/build-msi.ps1` | `2fcb6a512d29` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/fetch-ffmpeg.ps1` | `22317221f0fd` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/fetch-ffmpeg.sh` | `70a4fe74cde0` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `Installer/installer.nsi` | `9c46580c66a2` | CMakeLists.txt; scripts/create-deb.sh | PARZIALE | Installer Debian da confrontare con quello Windows |
| `LICENSE` | `559fd2eab22f` | web/resources/LICENSE-windows.txt | COPIATO | Byte identici all'inventario |
| `README.md` | `66ccca707c11` | README.md | PARZIALE | Manuale Linux specifico |
| `samples/overlays/README.md` | `b73da3f7d115` | — | RIFERIMENTO | File Windows senza traduzione di codice diretta |
| `sed_script.sh` | `39affaa82590` | — | RIFERIMENTO | File Windows senza traduzione di codice diretta |
| `src/Orbis.Stream.App/App.xaml.cs` | `fa306ba1e768` | src/main.cpp; web/index.html | PARZIALE | Shell Linux usa browser locale |
| `src/Orbis.Stream.App/AppText.cs` | `d32b2ac1363c` | src/main.cpp; web/index.html | PARZIALE | Shell Linux usa browser locale |
| `src/Orbis.Stream.App/AssemblyInfo.cs` | `3fac2af92385` | src/main.cpp; web/index.html | PARZIALE | Shell Linux usa browser locale |
| `src/Orbis.Stream.App/MainWindow.xaml.cs` | `329383722b26` | src/main.cpp; web/index.html | PARZIALE | Shell Linux usa browser locale |
| `src/Orbis.Stream.App/Orbis.Stream.App.csproj` | `47c879e7516a` | CMakeLists.txt; VERSION | PARZIALE | Build e versione Linux indipendenti |
| `src/Orbis.Stream.App/wwwroot/css/base.css` | `5db6e559e01e` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/cleanup.css` | `9a4a2966b57d` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/composer.css` | `455d58d4c336` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/controls.css` | `d5e27201578c` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/countdown.css` | `fcbd69f01e27` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/dashboard.css` | `13961623f1f2` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/feedback.css` | `d377efab2eb3` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/layout.css` | `55b191daaea9` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/manual.css` | `90306123fa2e` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/meters.css` | `f64ce9cd08c1` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/overlays.css` | `a8a51f570c8a` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/preview.css` | `921fc6a25460` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/scenes.css` | `72e1e16e57f5` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/surfaces.css` | `0b2f7af39ad6` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/css/tokens.css` | `31e9770fc711` | web/style.css | PARZIALE | Stile/struttura da confrontare selettore per selettore |
| `src/Orbis.Stream.App/wwwroot/js/composer.js` | `19b109c44fef` | web/app.js | PARZIALE | Interazioni da confrontare funzione per funzione |
| `src/Orbis.Stream.App/wwwroot/js/scenes.js` | `e783d7c11821` | web/app.js | PARZIALE | Interazioni da confrontare funzione per funzione |
| `src/Orbis.Stream.App/wwwroot/js/site.js` | `02d513390548` | web/app.js | PARZIALE | Interazioni da confrontare funzione per funzione |
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
| `src/Orbis.Stream.Core/Data/VideoRepository.cs` | `1dccfdc7ca22` | src/core.cpp | PARZIALE | Schema SQLite Linux diverso da quello Windows |
| `src/Orbis.Stream.Core/Data/VideoSettingRepository.cs` | `5e50879b52f0` | src/core.cpp | PARZIALE | Schema SQLite Linux diverso da quello Windows |
| `src/Orbis.Stream.Core/Domain/Entities.cs` | `2547a218fd41` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/Enums.cs` | `6f78cc3e98d9` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/FfmpegCodecCatalog.cs` | `f2a4bc27e24c` | — | MANCANTE | Catalogo codec completo mancante |
| `src/Orbis.Stream.Core/Domain/LiveStatus.cs` | `7b90b680f3ad` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/SceneButton.cs` | `7c1db8246cd5` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/SourceKind.cs` | `7118ce7ba708` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/VideoSettingAdaptiveBitrate.cs` | `a251a8a05f16` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/VideoSettingLatency.cs` | `c679cf611509` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Domain/VideoSettingQuality.cs` | `94a0a4e52577` | src/core.hpp; src/core.cpp | PARZIALE | Modello JSON Linux da confrontare proprietà per proprietà |
| `src/Orbis.Stream.Core/Hosting/AutoCleanupService.cs` | `a274344a1ed5` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/DatabaseBootstrapper.cs` | `a97ebe474d63` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/EncoderTuningStartup.cs` | `86630f7803fb` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/OrbisEndpoints.cs` | `e834a1662da4` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/OrbisHost.cs` | `d287f870dc5e` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/OrbisServiceCollectionExtensions.cs` | `3930a809e5e2` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/OrbisWebApplicationExtensions.cs` | `e7c1e7cffc16` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Hosting/RequestValidator.cs` | `1333e931de4b` | src/server.cpp; src/main.cpp | PARZIALE | Route e vita del servizio da confrontare |
| `src/Orbis.Stream.Core/Http/ExceptionHandlingMiddleware.cs` | `f9c7f1f40991` | src/server.cpp | PARZIALE | Errori e formato risposte da confrontare |
| `src/Orbis.Stream.Core/Http/OrbisExceptions.cs` | `2d55ba10bbc4` | src/server.cpp | PARZIALE | Errori e formato risposte da confrontare |
| `src/Orbis.Stream.Core/Http/ResponseFactory.cs` | `6cb62c591818` | src/server.cpp | PARZIALE | Errori e formato risposte da confrontare |
| `src/Orbis.Stream.Core/I18n/Localizer.cs` | `1d9a0a0888db` | web/app.js; web/resources/ui | PARZIALE | Cataloghi presenti, controlli Linux non tutti tradotti |
| `src/Orbis.Stream.Core/I18n/ManualText.cs` | `dda99dc08a71` | web/app.js; web/resources/ui | PARZIALE | Cataloghi presenti, controlli Linux non tutti tradotti |
| `src/Orbis.Stream.Core/I18n/MessageCatalog.cs` | `88fe4c10f7b6` | web/app.js; web/resources/ui | PARZIALE | Cataloghi presenti, controlli Linux non tutti tradotti |
| `src/Orbis.Stream.Core/I18n/MessageFormatter.cs` | `7470194897ea` | web/app.js; web/resources/ui | PARZIALE | Cataloghi presenti, controlli Linux non tutti tradotti |
| `src/Orbis.Stream.Core/I18n/PropertiesBundle.cs` | `0957396c7870` | web/app.js; web/resources/ui | PARZIALE | Cataloghi presenti, controlli Linux non tutti tradotti |
| `src/Orbis.Stream.Core/I18n/UiText.cs` | `89e5578cee3f` | web/app.js; web/resources/ui | PARZIALE | Cataloghi presenti, controlli Linux non tutti tradotti |
| `src/Orbis.Stream.Core/Logging/FileLoggerProvider.cs` | `6856ecbe669a` | — | MANCANTE | Logger applicativo e rotazione mancanti |
| `src/Orbis.Stream.Core/Messages/messages_ar.properties` | `616d6796a11d` | web/resources/messages/messages_ar.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_da.properties` | `98df690ee0cd` | web/resources/messages/messages_da.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_de.properties` | `84931f19fd16` | web/resources/messages/messages_de.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_el.properties` | `27ddd1119533` | web/resources/messages/messages_el.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_en.properties` | `dfad2f5658bc` | web/resources/messages/messages_en.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_es.properties` | `cd534692e38f` | web/resources/messages/messages_es.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_fil.properties` | `cb898ea867f8` | web/resources/messages/messages_fil.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_fr.properties` | `c879860d5783` | web/resources/messages/messages_fr.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_hi.properties` | `65300943c47e` | web/resources/messages/messages_hi.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_hod.properties` | `018e00334d3f` | web/resources/messages/messages_hod.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_it.properties` | `a1be8c67ba4a` | web/resources/messages/messages_it.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_ja.properties` | `17cc6dd36e01` | web/resources/messages/messages_ja.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_ko.properties` | `3dfad663fb68` | web/resources/messages/messages_ko.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_la.properties` | `800b18fb1b63` | web/resources/messages/messages_la.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_nl.properties` | `798c3343fe72` | web/resources/messages/messages_nl.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_pt.properties` | `45a6dea270fc` | web/resources/messages/messages_pt.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_ro.properties` | `9e8ce18fe985` | web/resources/messages/messages_ro.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_ru.properties` | `972c44419d9f` | web/resources/messages/messages_ru.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_tlh.properties` | `0ba0c49c5b4f` | web/resources/messages/messages_tlh.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_tr.properties` | `8199f7b5a97e` | web/resources/messages/messages_tr.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Messages/messages_zh.properties` | `3bbf0249be6f` | web/resources/messages/messages_zh.properties | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Orbis.Stream.Core.csproj` | `30e2f94d0e68` | CMakeLists.txt; VERSION | PARZIALE | Build e versione Linux indipendenti |
| `src/Orbis.Stream.Core/Pages/Countdown.cshtml` | `c9e582af3286` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/MainChannelSetting.cshtml` | `83ab73b6d3a6` | web/pages/MainChannelSetting.html; web/app.js | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/MainChannelSetting.cshtml.cs` | `b761f6cd3c39` | web/app.js; src/server.cpp | PARZIALE | Flusso e validazioni della pagina da confrontare |
| `src/Orbis.Stream.Core/Pages/MainLayout.cshtml` | `77af0c2dc526` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/MainLayout.cshtml.cs` | `f8644bb17f1d` | web/app.js; src/server.cpp | PARZIALE | Flusso e validazioni della pagina da confrontare |
| `src/Orbis.Stream.Core/Pages/MainLayoutList.cshtml` | `6654595ff035` | web/pages/MainLayoutList.html; web/app.js | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/MainLayoutList.cshtml.cs` | `9a3a894cb7f3` | web/app.js; src/server.cpp | PARZIALE | Flusso e validazioni della pagina da confrontare |
| `src/Orbis.Stream.Core/Pages/MainLive.cshtml` | `5a44642050fb` | web/pages/MainLive.html; web/app.js; src/live_rows.cpp | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/MainLive.cshtml.cs` | `cf634e9e752f` | src/live_rows.cpp; src/server.cpp; web/app.js | PARZIALE | Righe/snapshot e wizard portati; locking video e notifiche push Windows ancora diversi |
| `src/Orbis.Stream.Core/Pages/MainLiveHistory.cshtml` | `27ae35160bb1` | web/pages/MainLiveHistory.html; web/app.js | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/MainLiveHistory.cshtml.cs` | `aceaf8a6bdbb` | src/history_cleanup_service.cpp; src/server.cpp; web/app.js | PARZIALE | Filtri e pulizia portati; modello dati e scene temporanee ancora diversi |
| `src/Orbis.Stream.Core/Pages/MainLiveMenu.cshtml` | `d293f2ca8f85` | web/pages/MainLiveMenu.html; web/app.js | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/MainLiveMenu.cshtml.cs` | `5e73dc0c7384` | web/app.js; src/server.cpp | PARZIALE | Flusso e validazioni della pagina da confrontare |
| `src/Orbis.Stream.Core/Pages/MainManual.cshtml` | `9fa8f58b6e70` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/MainMenu.cshtml` | `9aa25a715fc1` | web/pages/MainMenu.html; web/app.js | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/MainPreview.cshtml` | `6142924a2887` | web/pages/MainPreview.html; web/app.js | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/MainPreview.cshtml.cs` | `b87290dd2698` | web/app.js; src/server.cpp | PARZIALE | Flusso e validazioni della pagina da confrontare |
| `src/Orbis.Stream.Core/Pages/MainSetting.cshtml` | `d4626e99d286` | web/pages/MainSetting.html; web/app.js | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/MainTaskManager.cshtml` | `13d638252bd6` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/MainTaskManager.cshtml.cs` | `f582840e9851` | web/app.js; src/server.cpp | PARZIALE | Flusso e validazioni della pagina da confrontare |
| `src/Orbis.Stream.Core/Pages/MainVideoSetting.cshtml` | `5ee7df24533b` | web/pages/MainVideoSetting.html; web/app.js | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/MainVideoSetting.cshtml.cs` | `aeaf82f7b750` | web/app.js; src/server.cpp | PARZIALE | Flusso e validazioni della pagina da confrontare |
| `src/Orbis.Stream.Core/Pages/OrbisPageModel.cs` | `b48f328ee53b` | web/app.js; src/server.cpp | PARZIALE | Flusso e validazioni della pagina da confrontare |
| `src/Orbis.Stream.Core/Pages/PageHelpers.cs` | `146998a41524` | web/app.js; src/server.cpp | PARZIALE | Flusso e validazioni della pagina da confrontare |
| `src/Orbis.Stream.Core/Pages/Shared/_Layout.cshtml` | `72527945fbd5` | web/index.html; web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/Shared/_Pager.cshtml` | `d36587b61f29` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/Shared/_SceneDeckIcon.cshtml` | `1a4cbf0f3cc3` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/Shared/_SpotIcon.cshtml` | `61a95cdcab2c` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/Shared/_VideoSettingFields.cshtml` | `9664431a7b86` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/VideoSettingForm.cs` | `d26c40454b8d` | web/app.js; src/server.cpp | PARZIALE | Flusso e validazioni della pagina da confrontare |
| `src/Orbis.Stream.Core/Pages/_AppStats.cshtml` | `a954523575b3` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_LiveRows.cshtml` | `f9d8f113f68f` | web/pages/_LiveRows.html; web/app.js; src/live_rows.cpp | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/_PlaylistVideos.cshtml` | `73902cf7a839` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_SceneComposer.cshtml` | `07b1dfe98aab` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_SceneDeck.cshtml` | `49272363885d` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_StartWizardConfiguration.cshtml` | `8a482a25af62` | web/pages/_StartWizardConfiguration.html; web/app.js | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/_StartWizardSetting.cshtml` | `45eb916bc80a` | web/pages/_StartWizardSetting.html; web/app.js | PARZIALE | Struttura e percorso ripresi; grafica e dettagli da confrontare |
| `src/Orbis.Stream.Core/Pages/_Stats.cshtml` | `e9a874b1645f` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_SystemInfo.cshtml` | `dffad664bd3d` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_ViewImports.cshtml` | `47666ab1e2ad` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Pages/_ViewStart.cshtml` | `62afaee35009` | web/app.js | PARZIALE | Pagina Razor da riprodurre nel client Linux |
| `src/Orbis.Stream.Core/Services/BackgroundTaskExecutor.cs` | `4c33ff84e227` | src/stream.cpp; src/media_job.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/EncoderTuningService.cs` | `b9d11e500f2b` | src/media_job.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/FfmpegVideoPlaylistStreamer.cs` | `f7c28c0de165` | src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/ImageService.cs` | `b20cd1bf7565` | src/media_job.cpp; src/server.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/LiveChangeNotifier.cs` | `d697b45b225d` | — | MANCANTE | Notifiche push live mancanti |
| `src/Orbis.Stream.Core/Services/LiveHistoryCleanupService.cs` | `5b093c7c3352` | src/history_cleanup_service.cpp; src/history_cleanup_service.hpp; src/core.cpp; src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/LivePreviewService.cs` | `d017948663da` | src/stream.cpp; src/server.cpp | PARZIALE | Servizio da verificare metodo per metodo |
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
| `src/Orbis.Stream.Core/Services/StreamingService.cs` | `f961c0acce3c` | src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/SystemInfoService.cs` | `c88c3c1cac5f` | src/core.cpp; src/metrics.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/VideoService.cs` | `2e91cdb490ce` | src/core.cpp; src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/VideoSettingService.cs` | `a2e2d72ceb02` | src/core.cpp; src/stream.cpp | PARZIALE | Servizio da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Services/YouTubeAirState.cs` | `856d1865b39b` | — | MANCANTE | Stato automatico YouTube mancante |
| `src/Orbis.Stream.Core/Services/YouTubeAirWatch.cs` | `295888495f26` | — | MANCANTE | Monitoraggio automatico YouTube mancante |
| `src/Orbis.Stream.Core/Streaming/BitrateLadder.cs` | `3c82f41089e9` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FfmpegCommandBuilder.cs` | `eed59b9b8ff3` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FfmpegProbe.cs` | `acb2bc49ff0f` | src/core.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FfmpegStreamingSession.cs` | `6dba0c2005b6` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FlvPacedRelay.cs` | `601e61abee83` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/FlvSink.cs` | `c5f04505ca11` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/HybridWaiter.cs` | `035df7679ae2` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/LiveOutput.cs` | `fb8f5356ed24` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/LivePlatformEmbeds.cs` | `fb2f6fae89d6` | src/media_job.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/LivePreviewFrames.cs` | `69f20244e11e` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/LiveStreamKeys.cs` | `7a1b1c73e39f` | src/live_stream_keys.cpp; src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/MonitorTarget.cs` | `424061f06190` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/OverlayMedia.cs` | `49a5c757e830` | src/stream.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/Streaming/Rtmp/Amf0.cs` | `e8e9217a1945` | — | MANCANTE | Protocollo RTMP nativo mancante |
| `src/Orbis.Stream.Core/Streaming/Rtmp/RtmpPublisher.cs` | `c00dd2455ef6` | — | MANCANTE | Publisher RTMP nativo mancante |
| `src/Orbis.Stream.Core/Streaming/StreamPlatform.cs` | `e45dd690223a` | src/stream_platform.cpp; src/stream.cpp; src/media_job.cpp | PARZIALE | Semantica streaming da verificare metodo per metodo |
| `src/Orbis.Stream.Core/SystemInfo/AppInfo.cs` | `25db9d2ee8f3` | src/metrics.cpp; src/core.cpp | PARZIALE | Metriche con contatori specifici di Linux |
| `src/Orbis.Stream.Core/SystemInfo/SystemInfoProviders.cs` | `0c4e8ab6c32d` | src/metrics.cpp; src/core.cpp | PARZIALE | Metriche con contatori specifici di Linux |
| `src/Orbis.Stream.Core/Ui/manual.ar.json` | `a3469b422b2b` | web/resources/ui/manual.ar.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.da.json` | `2a335c11776c` | web/resources/ui/manual.da.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.de.json` | `b7f54e7ca506` | web/resources/ui/manual.de.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.el.json` | `50792dae2c0d` | web/resources/ui/manual.el.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.en.json` | `725bc786b877` | web/resources/ui/manual.en.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.es.json` | `4c14fd6ab231` | web/resources/ui/manual.es.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.fil.json` | `55bb70cef390` | web/resources/ui/manual.fil.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.fr.json` | `7327757ecbb3` | web/resources/ui/manual.fr.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.hi.json` | `0b947eb1a8eb` | web/resources/ui/manual.hi.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.hod.json` | `dba78c2b6c1d` | web/resources/ui/manual.hod.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.it.json` | `17be7419b9aa` | web/resources/ui/manual.it.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.ja.json` | `35a0035e6a37` | web/resources/ui/manual.ja.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.ko.json` | `214b1cfd9e06` | web/resources/ui/manual.ko.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.la.json` | `80ec1037dadd` | web/resources/ui/manual.la.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.nl.json` | `b5e0a6af12d3` | web/resources/ui/manual.nl.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.pt.json` | `05faf8fcbb26` | web/resources/ui/manual.pt.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.ro.json` | `706351cb03ed` | web/resources/ui/manual.ro.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.ru.json` | `dc8a1a9fea31` | web/resources/ui/manual.ru.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.tlh.json` | `b878c3e180ab` | web/resources/ui/manual.tlh.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.tr.json` | `1d7459f46632` | web/resources/ui/manual.tr.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/manual.zh.json` | `9473ce491e06` | web/resources/ui/manual.zh.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.ar.json` | `19fa4b103eec` | web/resources/ui/ui.ar.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.da.json` | `9d7d9fafa58c` | web/resources/ui/ui.da.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.de.json` | `81c8e7c7d18f` | web/resources/ui/ui.de.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.el.json` | `09e046e0cdd4` | web/resources/ui/ui.el.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.en.json` | `8fe17a62131a` | web/resources/ui/ui.en.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.es.json` | `6a6f303710cd` | web/resources/ui/ui.es.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.fil.json` | `7a3b1fe8da20` | web/resources/ui/ui.fil.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.fr.json` | `13bee91c2c66` | web/resources/ui/ui.fr.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.hi.json` | `84a784d17506` | web/resources/ui/ui.hi.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.hod.json` | `168267bb6f6d` | web/resources/ui/ui.hod.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.it.json` | `0a8cb4e0a02b` | web/resources/ui/ui.it.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.ja.json` | `f0d128b020ca` | web/resources/ui/ui.ja.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.ko.json` | `3fe17174a6dd` | web/resources/ui/ui.ko.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.la.json` | `c2845f592acd` | web/resources/ui/ui.la.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.nl.json` | `0e57efe8276e` | web/resources/ui/ui.nl.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.pt.json` | `277eaf19904f` | web/resources/ui/ui.pt.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.ro.json` | `14038c227d82` | web/resources/ui/ui.ro.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.ru.json` | `8a25f3f14e12` | web/resources/ui/ui.ru.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.tlh.json` | `a7897555b4f5` | web/resources/ui/ui.tlh.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.tr.json` | `c96b726c3aa9` | web/resources/ui/ui.tr.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Core/Ui/ui.zh.json` | `a71efe43fcb6` | web/resources/ui/ui.zh.json | COPIATO | Byte identici all'inventario |
| `src/Orbis.Stream.Tests/AdaptiveBitrateSwitchTests.cs` | `9c394fce1a0a` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/BitrateLadderTests.cs` | `394d5c7ff3d4` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/DatabaseTests.cs` | `d43e5526bad1` | tests/core_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/DomainAndPagingTests.cs` | `86119e95c3e2` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/EncoderQualityTests.cs` | `d7e75ae03587` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/EndToEndApiTests.cs` | `be497cd7572e` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/FfmpegCommandBuilderTests.cs` | `ca110c4ce303` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/FfmpegStreamingTests.cs` | `343b74e35a22` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/FileLoggerTests.cs` | `10c1d0085bd6` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/I18nTests.cs` | `62603695650a` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/LiveFrameTests.cs` | `2e6ca695e7e2` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/LiveHistoryCleanupTests.cs` | `088e814eadc8` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/LivePlatformEmbedTests.cs` | `2c0b22b83e69` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/LivePreviewFrameFileTests.cs` | `80c84902bf22` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/LivePreviewTests.cs` | `a2906211145e` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/ManualTextTests.cs` | `772d15368673` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/MediaProxyTests.cs` | `97d67f232b36` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/MessageCatalogTests.cs` | `90fa645b736a` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/MessageCodeTests.cs` | `2d7135e986bc` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/Orbis.Stream.Tests.csproj` | `da09e6f5e1d6` | CMakeLists.txt; VERSION | PARZIALE | Build e versione Linux indipendenti |
| `src/Orbis.Stream.Tests/OverlayCommandTests.cs` | `8fa33d43f33d` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/OverlayLiveTests.cs` | `bfe269cbc88a` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/PacedRelayTests.cs` | `569875b9a547` | tests/core_tests.cpp | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/PlaylistTests.cs` | `5467a36c30c2` | tests/integration.py | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/RelayTransportTests.cs` | `4e75505f43b6` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/RtmpPublisherTests.cs` | `6b71de7a2261` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/RuntimeOptionsTests.cs` | `36724209ac13` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/SceneButtonApiTests.cs` | `b839fbc513c4` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/SceneDeckTests.cs` | `7cbad58455ea` | tests/browser.mjs | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/SceneLiveTests.cs` | `56164d6c04ad` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/SceneSwitchLiveTests.cs` | `c7a22cf6f60e` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/StyleSheetTests.cs` | `52541b8ed7f4` | tests/browser.mjs | PARZIALE | Casi Windows da trasferire |
| `src/Orbis.Stream.Tests/TestPictures.cs` | `778545808506` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/TikTokTests.cs` | `217af0fe3a90` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/UiTextTests.cs` | `0aa7ebc96b80` | — | MANCANTE | Test equivalente non presente |
| `src/Orbis.Stream.Tests/YouTubeAirWatchTests.cs` | `06ab0e3ae7d3` | — | MANCANTE | Test equivalente non presente |
