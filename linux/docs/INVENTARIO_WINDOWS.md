# Inventario dei sorgenti Windows

257 file censiti, 61 route API. Snapshot SHA-256 disponibile in `windows-inventory.json`.

Il censimento riguarda il checkout locale di lazy-stream-windows; non deriva dalle funzioni aggiunte soltanto alla versione macOS. La parità funzionale è descritta in [MIGRAZIONE.md](MIGRAZIONE.md).

## Route API Windows

| Metodo | Percorso originale |
|---|---|
| GET | `/scene/layouts` |
| GET | `/scene/{pkid:long}` |
| POST | `/scene/save` |
| DELETE | `/scene/{pkid:long}` |
| GET | `/preview/sources` |
| GET | `/preview/sources/snapshot` |
| POST | `/preview/sources/prepare` |
| GET | `/preview/sources/probe` |
| GET | `/scene/overlays` |
| POST | `/scene/overlays` |
| GET | `/scene/overlays/{name}` |
| GET | `/scene/overlays/{name}/still` |
| DELETE | `/scene/overlays/{name}` |
| POST | `/live/start-scene-live` |
| POST | `/live/start-live` |
| POST | `/live/start-video-live` |
| PUT | `/live/stop-live` |
| GET | `/live/{pkid:int}/scene` |
| POST | `/live/{pkid:int}/scene/resume` |
| POST | `/live/{pkid:int}/scene/{button:long}` |
| GET | `/scene-buttons` |
| POST | `/scene-buttons` |
| PUT | `/scene-buttons/{pkid:long}` |
| DELETE | `/scene-buttons/{pkid:long}` |
| POST | `/scene-buttons/media` |
| GET | `/scene-buttons/media/{name}/still` |
| GET | `/video/getPage` |
| GET | `/video/getAllVideo` |
| GET | `/video/getAllChannelWithVideoLive` |
| PUT | `/video/unlockVideo` |
| DELETE | `/video/live-history/{pkid:long}` |
| DELETE | `/video/live-history/older-than/{months:int}` |
| POST | `/video/live-history/cleanup` |
| GET | `/video/live-history/cleanup` |
| POST | `/settings/save` |
| PUT | `/settings/change` |
| GET | `/settings/retrive` |
| GET | `/settings/retrive-channels` |
| GET | `/settings/retrive-directories-path` |
| DELETE | `/settings/delete` |
| GET | `/video-setting/getVideoSettings` |
| POST | `/video-setting/save` |
| POST | `/video-setting/link` |
| PUT | `/video-setting/edit` |
| DELETE | `/video-setting/delete` |
| POST | `/image/upload` |
| GET | `/image/loadimage` |
| GET | `/taskManager/statistics/allInfo` |
| GET | `/taskManager/statistics/cpu` |
| GET | `/taskManager/statistics/ram` |
| GET | `/taskManager/statistics/swap` |
| GET | `/taskManager/statistics/cpu/temperature` |
| GET | `/taskManager/statistics/gpu/temperature` |
| GET | `/preview/live` |
| GET | `/preview/live/embed` |
| POST | `/preview/live/{pkid:int}/air/dismiss` |
| GET | `/preview/live/{pkid:int}/video` |
| GET | `/preview/live/{pkid:int}/frame` |
| PUT | `/preview/live/{pkid:int}/parameters` |
| PUT | `/preview/live/{pkid:int}/volume` |
| GET | `/updates` |

## File, tipi, proprietà e metodi pubblici

### `Installer/OrbisStream.wxs`

SHA-256: `97a815cece7f91c180f368e8e26669424ce88d231d5049d1da33424206a4f0bf`.

### `Installer/build-installer.ps1`

SHA-256: `d7871ce81376e7b01612ba17a052be68373ddeec984fea8e62b9bd6924d89f9c`.

### `Installer/build-installer.sh`

SHA-256: `e51e45fb4a9a36964fdd444e731a4a71e28ce06f62651366c335f5bd9c89c0af`.

### `Installer/build-msi.ps1`

SHA-256: `2fcb6a512d29237f6fa74b0544bd41f6b0eb6fb5afefdfd3affa45fe99890b83`.

### `Installer/fetch-ffmpeg.ps1`

SHA-256: `22317221f0fdf919d27c9f281b7e448a9b7855780850ff2c8703062fcc1ef1d6`.

### `Installer/fetch-ffmpeg.sh`

SHA-256: `70a4fe74cde0e3be3db0696a40e264c544e7e43d9c36ff6208c7dc9c45031d84`.

### `Installer/installer.nsi`

SHA-256: `9c46580c66a2fd205407d0c7e984e934ba8e95405b7f6a92e6a9e163a34c3d50`.

### `LICENSE`

SHA-256: `559fd2eab22fe11af69707fb6d0be5fe2d03e7dfc8fe8ff37071e5c2a04474f0`.

### `README.md`

SHA-256: `66ccca707c1196e44f1b25166d8eae867f4f93efb760b17da6e38218bd9efbb1`.

### `samples/overlays/README.md`

SHA-256: `b73da3f7d115eb84bdf77ffa36f49dfb55aa8f16819589ce824502326c29b933`.

### `sed_script.sh`

SHA-256: `39affaa825907658cf988caac3b924c72e41ddad37cf1709e7a023d139cbe0d1`.

### `src/Orbis.Stream.App/App.xaml.cs`

Tipi: `App`.

SHA-256: `fa306ba1e76838c9ca1523f12225dbe9a8b67c9f3469d893a09684f24983e938`.

### `src/Orbis.Stream.App/AppText.cs`

Tipi: `AppText`.

Metodi pubblici: `BrowserInitError`, `BrowserReachError`, `StartError`, `UnexpectedError`.

SHA-256: `d32b2ac1363cb06ab365f4936e7b7248a44b8477677687b2dd3007cfa0b83e69`.

### `src/Orbis.Stream.App/AssemblyInfo.cs`

SHA-256: `3fac2af92385f676e9a941befd23fe38f0a5fa6167d4a3a88f07e730c5595ca3`.

### `src/Orbis.Stream.App/MainWindow.xaml.cs`

Tipi: `MainWindow`.

SHA-256: `329383722b26af705efccafe52449f4e6a2de54a16159c5bb3718a1d3e827288`.

### `src/Orbis.Stream.App/Orbis.Stream.App.csproj`

SHA-256: `47c879e7516a4ac1d80b29f62f433a7151a751dab436e94352d7d58c154a01a8`.

### `src/Orbis.Stream.App/wwwroot/css/base.css`

SHA-256: `5db6e559e01eb48fedcc6145d8eb15758e9a5753bdf6154e7f9ee0680823c349`.

### `src/Orbis.Stream.App/wwwroot/css/cleanup.css`

SHA-256: `9a4a2966b57d52c816549ae0c331f734032c363bc89f70633c5f3f52b52219d1`.

### `src/Orbis.Stream.App/wwwroot/css/composer.css`

SHA-256: `455d58d4c3360f0f651cbf31b7bd785cc0900c0a31b12ebef5701a69520806de`.

### `src/Orbis.Stream.App/wwwroot/css/controls.css`

SHA-256: `d5e27201578cf805b87169d1964800eafe5f2de51cfdb3d9d71a39d3d4c9f774`.

### `src/Orbis.Stream.App/wwwroot/css/countdown.css`

SHA-256: `fcbd69f01e27006b12c547844e3057a57a3b3631f87eb09060da114ccf61162a`.

### `src/Orbis.Stream.App/wwwroot/css/dashboard.css`

SHA-256: `13961623f1f2605fec24a1250040a1e6e65055c9a13f9b99e4a2febdeae2a3e8`.

### `src/Orbis.Stream.App/wwwroot/css/feedback.css`

SHA-256: `d377efab2eb33dd715999166d5f26881f8fd19fd77e411c19598b0f2b54f132a`.

### `src/Orbis.Stream.App/wwwroot/css/layout.css`

SHA-256: `55b191daaea922b3138e5ea38a1265147b12c35a72149b3aecad64286bd9d5ee`.

### `src/Orbis.Stream.App/wwwroot/css/manual.css`

SHA-256: `90306123fa2ea0c7fe6b303d79992a6eade8e1e09df184b71aa186903524a28e`.

### `src/Orbis.Stream.App/wwwroot/css/meters.css`

SHA-256: `f64ce9cd08c12dba83396388d97ccf54fe0ef333362897bb2ab53f590bbb125e`.

### `src/Orbis.Stream.App/wwwroot/css/overlays.css`

SHA-256: `a8a51f570c8a08a4c6141d8211d57407159569523bc179ab07c50ec4d7796017`.

### `src/Orbis.Stream.App/wwwroot/css/preview.css`

SHA-256: `921fc6a25460a05560aa3cc7bad39513b447ab6c855ff6a8a33b746e5d5d8842`.

### `src/Orbis.Stream.App/wwwroot/css/scenes.css`

SHA-256: `72e1e16e57f59c8bc1c8eec41d702ea1fc7894b14d76b0686c270067d9cb18cb`.

### `src/Orbis.Stream.App/wwwroot/css/surfaces.css`

SHA-256: `0b2f7af39ad659344a14e8a65c599ca155edb54ae4dcff6b6d24b3167be37396`.

### `src/Orbis.Stream.App/wwwroot/css/tokens.css`

SHA-256: `31e9770fc711c25607cf13f7b1ab3b54d3629d8f8ad0742a158619b69c436280`.

### `src/Orbis.Stream.App/wwwroot/js/composer.js`

SHA-256: `19b109c44fefe1ff2378f5ddf072e1983701b99b8e5caeb89632f1d120c80828`.

### `src/Orbis.Stream.App/wwwroot/js/scenes.js`

SHA-256: `e783d7c11821f51b06db49b76d16f7aab51f7f6d55dd1ea2949b483b6b1ebf02`.

### `src/Orbis.Stream.App/wwwroot/js/site.js`

SHA-256: `02d513390548aea7993fc7c5e4ae2a2dbe387809eb51f9b927ce4f5614695ac8`.

### `src/Orbis.Stream.Core/Configuration/OrbisRuntimeOptions.cs`

Tipi: `OrbisRuntimeOptions`.

Proprietà: `DataDirectory`, `DatabasePath`, `LogDirectory`, `ImagesDirectory`, `Port`, `EmbeddedBrowser`, `FfmpegPath`, `FfprobePath`, `WebRootPath`.

Metodi pubblici: `EnsureDirectories`, `Resolve`.

SHA-256: `8c7b25ccd306fb63e21728e9be89097b4603263178dea84da245c3455ebc6313`.

### `src/Orbis.Stream.Core/Contracts/LocalDateTimeConverter.cs`

Tipi: `LocalDateTimeConverter`, `NullableLocalDateTimeConverter`, `DateTimeParsing`.

Metodi pubblici: `Parse`.

SHA-256: `2ec0b5a482750dbdc93e4b518553312537d0a6bb65e5b363a9330b996a911d49`.

### `src/Orbis.Stream.Core/Contracts/Requests.cs`

Tipi: `SettingRequest`, `SettingResponse`, `VideoPathResponse`, `VideoPathAndKeyResponse`, `ChannelResponse`, `AudioSettingsRequest`, `VideoOptionRequest`, `VideoSettingsRequest`, `LiveParameterRequest`, `LiveVolumeRequest`, `SceneButtonRequest`, `VideoLiveHistoryRequest`, `VideoResponse`, `VideoRequest`, `StartLiveRequest`, `StartSceneLiveRequest`, `SceneItemRequest`, `SceneRequest`, `SceneSavedResponse`, `SystemInfoResponse`, `ApiEnvelope`, `LiveRow`, `PlaylistDetails`.

Metodi pubblici: `FromEntity`, `ToEntity`.

SHA-256: `c5399c9fa1821b4f8af188e65585f2e21341c0e33bc3fb422b3d1d4629df087e`.

### `src/Orbis.Stream.Core/Data/DatabaseSchema.cs`

Tipi: `DatabaseSchema`, `ColumnDefinition`.

Metodi pubblici: `EnsureCreated`.

SHA-256: `64cae458a1fb28b80c34174dd8323b34840530de96e21711eb71497ffec3db15`.

### `src/Orbis.Stream.Core/Data/Paging.cs`

Tipi: `SortOrder`, `PageRequest`, `PagedResult`, `SpringPage`, `SpringPageMetadata`, `SpringPageSort`, `SpringSortDescriptor`, `SpringPageFactory`.

Metodi pubblici: `Default`, `Parse`.

SHA-256: `dc3b222e371cc2a0cc8a8241ef5ff2def2c92b3bb1adfc37ac22501993647f6c`.

### `src/Orbis.Stream.Core/Data/SceneButtonRepository.cs`

Tipi: `SceneButtonRepository`.

Metodi pubblici: `Count`, `Delete`, `FindAll`, `FindByPkid`, `Insert`, `Update`, `UsesMedia`.

SHA-256: `e6007f2efc538397632b37e5f142c2bd8aead9e99080c29bd9bfdd96de0cf710`.

### `src/Orbis.Stream.Core/Data/SceneRepository.cs`

Tipi: `SceneRepository`.

Metodi pubblici: `Delete`, `FindAll`, `FindByPkid`, `FindItemsOf`, `FindLayouts`, `Insert`, `IsOnAir`, `Save`, `Update`, `UsesOverlay`.

SHA-256: `1875e00f5f945318e906bb7d4512e39de21aff9cad820dc8ba4f20309d7c8751`.

### `src/Orbis.Stream.Core/Data/SettingRepository.cs`

Tipi: `SettingRepository`, `VideoLiveHistoryRepository`.

Metodi pubblici: `Delete`, `DeleteRow`, `FindAll`, `FindByFolderOfVideoToStreamAndLocalDateTimeStartLive`, `FindById`, `FindByPkid`, `FindByStreamUrlAndStreamKey`, `FindOlderThan`, `Insert`, `Update`.

SHA-256: `9aa559188378a94d5385ca8bb9b3918bf09bb57454758df953c023712e2f7566`.

### `src/Orbis.Stream.Core/Data/SqlQueryBuilder.cs`

Tipi: `FilterKind`, `FilterField`, `EntityFields`, `OrbisQueryException`, `SqlQueryBuilder`, `FilterValueConverter`.

Proprietà: `Join`, `TableAlias`, `Limit`, `Offset`.

Metodi pubblici: `ApplyPaging`, `ApplyTo`, `Column`, `Convert`, `OrderBy`, `Qualified`, `TryResolveField`, `WhereEquals`, `WhereIn`, `WherePredicate`, `WhereRaw`.

SHA-256: `680e539f818179a3f1a87db192840b8dba5dcae1a9495ad56d0c52c4a6c2f672`.

### `src/Orbis.Stream.Core/Data/SqliteConnectionFactory.cs`

Tipi: `SqliteConnectionFactory`.

Proprietà: `DatabasePath`.

Metodi pubblici: `Open`, `OpenAsync`.

SHA-256: `21641913024ce7d14a1d8e6c393cc33f7ebfa4121bf07e73ef7430221069bec6`.

### `src/Orbis.Stream.Core/Data/VideoRepository.cs`

Tipi: `VideoRepository`, `SqliteValue`, `LiveRowEntity`.

Metodi pubblici: `Delete`, `DeleteByLiveHistoryId`, `FindByLiveHistoryId`, `FindByLiveStatus`, `FindByLiveStatusAndChannelName`, `FindByPkid`, `FindByScenePkid`, `FindByVideoPathAndVideoLiveHistoryPkid`, `FindChannelNames`, `FindLivePage`, `FindPaged`, `FindPkidsByVideoSettingId`, `From`, `Insert`, `SetStopFlag`, `SetVideoSetting`, `SetVolume`, `ToBoolean`, `ToInt64`, `ToNullableDateTime`, `ToNullableDouble`, `ToNullableInt32`, `ToNullableInt64`, `ToText`, `Update`.

SHA-256: `1dccfdc7ca22737a9648f504b72050159b4f0ae3a54c5237e932c039c25dd618`.

### `src/Orbis.Stream.Core/Data/VideoSettingRepository.cs`

Tipi: `VideoSettingRepository`.

Metodi pubblici: `Delete`, `FindAll`, `FindById`, `FindByIsDefaultConfigurationTrueAndDefaultPlatformConfiguration`, `FindByTitleAndPlatform`, `Insert`, `Update`.

SHA-256: `5e50879b52f00ee399597a786fd1149e88f0a9cff2b759c332ab9d95ae20a24f`.

### `src/Orbis.Stream.Core/Domain/Entities.cs`

Tipi: `SettingEntity`, `VideoLiveHistoryEntity`, `AudioSettingEntity`, `VideoSettingsOptionEntity`, `VideoSettingEntity`, `VideoEntity`, `SceneEntity`, `SceneItemEntity`.

Proprietà: `Id`, `StreamUrl`, `StreamKey`, `PlatformStreamName`, `Description`, `VideoFolder`, `IsActive`, `ChannelName`, `ScenePkid`, `AutoCleanupEnabled`, `AutoCleanupIntervalMonths`, `AutoCleanupOlderThanMonths`, `FfmpegSender`, `Pkid`, `FolderOfVideoToStream`, `LocalDateTimeStartLive`, `StreamUrl`, `StreamKey`, `PlatformStreamName`, `UserName`, `Id`, `AudioCodec`, `AudioBitrate`, `Key`, `Value`, `Id`, `Title`, `VideoCodec`, `VideoCodecName`, `PixelFormat`, `VideoBitrate`, `VideoFormat`, `LastModified`, `IsDefaultConfiguration`, `DefaultPlatformConfiguration`, `GopSize`, `VideoWidth`, `VideoHeight`, `FrameRate`, `IsVideoAndAudioSettingActive`, `AudioSetting`, `AudioSettingId`, `VideoSettingsOptions`, `Pkid`, `Name`, `VideoPath`, `Extension`, `LiveStatus`, `LastTimeStampBeforeStop`, `Message`, `ShouldBeStop`, `StartDateLive`, `ChannelName`, `VideoLiveHistoryId`, `VideoSettingId`, `SourceKind`, `SourceTarget`, `ScenePkid`, `X`, `Y`, `Width`, `Height`, `DurationMilliseconds`, `SourceWidth`, `SourceHeight`, `AudioEnabled`, `Volume`, `Pkid`, `IsLayout`, `Name`, `Description`, `Width`, `Height`, `LastModified`, `Items`, `Pkid`, `ScenePkid`, `SourceKind`, `SourceTarget`, `Label`, `X`, `Y`, `Width`, `Height`, `AudioEnabled`.

SHA-256: `2547a218fd411a584419b3e07218dbaa6f8474cf996a3eecf7976f708097bf1a`.

### `src/Orbis.Stream.Core/Domain/Enums.cs`

Tipi: `SystemInfoField`, `SystemInfoFieldExtensions`, `VideoExtensions`.

Metodi pubblici: `ContentTypeOf`, `IsBrowserPlayable`, `IsVideoExtensionPresent`, `ToInfoName`, `ToMeterKey`.

SHA-256: `6f78cc3e98d98a1c2749129420ba558a6e674d80232f851db5861f8e5a493048`.

### `src/Orbis.Stream.Core/Domain/FfmpegCodecCatalog.cs`

Tipi: `MediaOption`, `values`, `FfmpegCodecCatalog`.

Metodi pubblici: `IdOfEncoderName`, `LabelOf`, `ResolveAudioCodecName`, `ResolvePixelFormat`, `ResolveVideoCodecName`.

SHA-256: `f2a4bc27e24ce30415e955b8dbec9b21801863d7ee23c76306b72205ccb622ea`.

### `src/Orbis.Stream.Core/Domain/LiveStatus.cs`

Tipi: `ordinal`, `LiveStatus`, `LiveStatusExtensions`, `LiveStatusJsonConverter`.

Metodi pubblici: `FromStorage`, `ToStorageValue`, `ToWireValue`, `TryParseWireValue`.

SHA-256: `7b90b680f3ad996c92ce4ba627d8ac8a4bb67655a62af067af2279039c72cacf`.

### `src/Orbis.Stream.Core/Domain/SceneButton.cs`

Tipi: `SceneButtonKind`, `SceneButtonEntity`, `SceneButtonKindExtensions`, `SceneHotkey`.

Proprietà: `Pkid`, `Position`, `Label`, `MediaName`, `Hotkey`, `DisplayMode`, `Placement`, `DurationSeconds`, `X`, `Y`, `Width`, `Height`, `LastModified`.

Metodi pubblici: `IsValid`, `ToWireValue`.

SHA-256: `7c1db8246cd511f90f7032edbed5871ac77107f818efd4a5bb5f1ec9f8cd241d`.

### `src/Orbis.Stream.Core/Domain/SourceKind.cs`

Tipi: `is`, `SourceKind`, `SourceKindExtensions`.

Metodi pubblici: `HasPicture`, `IsCaptureDevice`, `IsDefined`, `IsOverlay`, `ToWireValue`, `TryParse`, `TryParseWireValue`.

SHA-256: `7118ce7ba708441881ef81fe45b67882fd176fe15e51fe08bf3db8d97c912829`.

### `src/Orbis.Stream.Core/Domain/VideoSettingAdaptiveBitrate.cs`

Tipi: `VideoSettingAdaptiveBitrate`.

Metodi pubblici: `IsOn`, `Set`.

SHA-256: `a251a8a05f16a1aa0767e1404a5950e92609373775e0569847e522a7a26fe056`.

### `src/Orbis.Stream.Core/Domain/VideoSettingLatency.cs`

Tipi: `VideoSettingLatency`.

Metodi pubblici: `IsOn`, `Set`.

SHA-256: `c679cf611509da2d5278db8fc62b325c86a4059e6f4abb0022fae0efce765790`.

### `src/Orbis.Stream.Core/Domain/VideoSettingQuality.cs`

Tipi: `EncoderQuality`, `VideoSettingQuality`.

Metodi pubblici: `Of`, `Parse`, `Set`.

SHA-256: `94a0a4e5257735b14f286f59d94c41fe8633caa19c49a02677b4ad2985d264eb`.

### `src/Orbis.Stream.Core/Hosting/AutoCleanupService.cs`

Tipi: `AutoCleanupService`.

SHA-256: `a274344a1ed5760e5612c924a8211cff1e61431fc941c61ac493b58184785907`.

### `src/Orbis.Stream.Core/Hosting/DatabaseBootstrapper.cs`

Tipi: `DatabaseBootstrapper`, `DefaultSpec`.

Metodi pubblici: `StartAsync`, `StopAsync`.

SHA-256: `a97ebe474d63d4b81641c12faee3477c728a6f870a122e36f55b452afb192209`.

### `src/Orbis.Stream.Core/Hosting/EncoderTuningStartup.cs`

Tipi: `EncoderTuningStartup`.

Metodi pubblici: `StartAsync`, `StopAsync`.

SHA-256: `86630f7803fb907d165c43866eecefd34395ce7d362d222088bc7b56631c7377`.

### `src/Orbis.Stream.Core/Hosting/OrbisEndpoints.cs`

Tipi: `OrbisEndpoints`.

Metodi pubblici: `MapOrbisEndpoints`.

SHA-256: `e834a1662da4b7cf158671130bcef21a9c6fb6ae39fd0490f300db425a68ad07`.

### `src/Orbis.Stream.Core/Hosting/OrbisHost.cs`

Tipi: `OrbisHost`.

Metodi pubblici: `Create`, `ResolveOptions`.

SHA-256: `d287f870dc5edc70cc2823f560319191fbcc10fd23129582bded5f3d8bbc70e4`.

### `src/Orbis.Stream.Core/Hosting/OrbisServiceCollectionExtensions.cs`

Tipi: `OrbisServiceCollectionExtensions`.

Metodi pubblici: `AddOrbisStream`.

SHA-256: `3930a809e5e2ecc0b0cc09962ce756c6690e73654948fb1b3cfc736a209a7d5f`.

### `src/Orbis.Stream.Core/Hosting/OrbisWebApplicationExtensions.cs`

Tipi: `OrbisWebApplicationExtensions`.

Metodi pubblici: `BuildOrbisWebApplication`.

SHA-256: `e7c1e7cffc1627b75adf2fee1ad4f4a8f836a473fd880da963fbad14b53e89d9`.

### `src/Orbis.Stream.Core/Hosting/RequestValidator.cs`

Tipi: `RequestValidator`.

Metodi pubblici: `RequireImage`, `RequireLiveParameters`, `RequireLiveVolume`, `RequireOverlay`, `RequireSetting`, `RequireStartLive`, `RequireStartSceneLive`, `RequireVideo`.

SHA-256: `1333e931de4b555038e798dc8fe9d6c1f5f2227d75a6a2d0c21a21d65d8a293a`.

### `src/Orbis.Stream.Core/Http/ExceptionHandlingMiddleware.cs`

Tipi: `ExceptionHandlingMiddleware`.

Metodi pubblici: `InvokeAsync`.

SHA-256: `f9c7f1f409910f396973f6cb7a598b211bf65562af124847d9bd2a1f364c6a65`.

### `src/Orbis.Stream.Core/Http/OrbisExceptions.cs`

Tipi: `of`, `OrbisException`, `NotFoundCustomException`, `DuplicationEntityException`, `LiveException`, `FileReadingException`, `StreamingException`, `SqlCustomException`, `RequestValidationException`.

Proprietà: `MessageCode`, `Parameters`.

SHA-256: `2d55ba10bbc43834e9ef8b947e0ae2f48d709c5bf72b50c467475f1c0d6de970`.

### `src/Orbis.Stream.Core/Http/ResponseFactory.cs`

Tipi: `MessageResponse`, `ResponseFactory`.

Metodi pubblici: `Build`, `BuildBadResponse`, `BuildBadResponseWithoutMessageLabel`.

SHA-256: `6cb62c59181820019f6ff9af8d46a9710f7e6b075a38e49e398fa6e3bd4b43cd`.

### `src/Orbis.Stream.Core/I18n/Localizer.cs`

Tipi: `Localizer`.

Proprietà: `CurrentLanguage`.

Metodi pubblici: `PrintMessage`.

SHA-256: `1d9a0a0888dbb63f751ae3aca8cd729b360b8d3ac99c7523bddb4227ce8ab287`.

### `src/Orbis.Stream.Core/I18n/ManualText.cs`

Tipi: `ManualSection`, `ManualText`, `ManualDocument`, `ManualSectionDto`.

Metodi pubblici: `Sections`.

SHA-256: `dda99dc08a711b3dd8eb9db1e696d33759993fd4cc60897f82721f990b7a2c47`.

### `src/Orbis.Stream.Core/I18n/MessageCatalog.cs`

Tipi: `MessageNotFoundException`, `MessageCatalog`.

Proprietà: `Code`, `Language`.

Metodi pubblici: `GetMessage`, `LocateMessagesDirectory`, `ResolveCulture`, `ResolveLanguage`.

SHA-256: `88fe4c10f7b6adf9769b140f600d103da058a21ff95bbc8aa4e38de43a347155`.

### `src/Orbis.Stream.Core/I18n/MessageFormatter.cs`

Tipi: `MessageFormatter`.

Metodi pubblici: `Format`.

SHA-256: `7470194897ea5d22f3f4585a113c6fee1e1c8547c53a395a28e07f497fe0d41a`.

### `src/Orbis.Stream.Core/I18n/PropertiesBundle.cs`

Tipi: `PropertiesBundle`.

Metodi pubblici: `Load`, `Parse`, `TryGetValue`.

SHA-256: `0957396c7870c0333d4b7c0b1b611748e68c97165fcb5f958533e5f089f96a5b`.

### `src/Orbis.Stream.Core/I18n/UiText.cs`

Tipi: `UiText`, `LanguageOption`.

Proprietà: `Language`.

Metodi pubblici: `IsRightToLeft`, `IsSupported`.

SHA-256: `89e5578cee3fe2590fb122e4db4b01f2b7a6e6ea0c7ac22a5299797f18d19ea2`.

### `src/Orbis.Stream.Core/Logging/FileLoggerProvider.cs`

Tipi: `FileLoggerProvider`, `struct`, `FileLogger`.

Metodi pubblici: `CreateLogger`, `Dispose`, `IsEnabled`, `PathOf`.

SHA-256: `6856ecbe669a30950902d2daa0c9577fbe09635828ea624bce67523aa915f854`.

### `src/Orbis.Stream.Core/Messages/messages_ar.properties`

SHA-256: `616d6796a11dc541d239ac5e5f8a1b2c2426edbc1d841432a2badc9b9bfc2534`.

### `src/Orbis.Stream.Core/Messages/messages_da.properties`

SHA-256: `98df690ee0cdbae32aa2f9a2d58bf108ce5c798410ced625309fd93603063ed2`.

### `src/Orbis.Stream.Core/Messages/messages_de.properties`

SHA-256: `84931f19fd16b2aeec02cadf181645385f3a8092a1126742d3f515bb855b9e84`.

### `src/Orbis.Stream.Core/Messages/messages_el.properties`

SHA-256: `27ddd1119533bbee78dc388bd36b0cc007507c506a90191e0ed82c0f68167070`.

### `src/Orbis.Stream.Core/Messages/messages_en.properties`

SHA-256: `dfad2f5658bc386ad35fa24547eda846beb6ecdd3e24797bd148150e2b4ae9df`.

### `src/Orbis.Stream.Core/Messages/messages_es.properties`

SHA-256: `cd534692e38f0eabac02c7c6c06abf4d50bb4091dc4e30e89a3f36468204c507`.

### `src/Orbis.Stream.Core/Messages/messages_fil.properties`

SHA-256: `cb898ea867f86add4cdda979cb0b22aad9e070eea554be2b01c0a2de8662eb0e`.

### `src/Orbis.Stream.Core/Messages/messages_fr.properties`

SHA-256: `c879860d5783a5092492da72e048bd72a44e12e2deae3ab6eb5f23adf656d5ca`.

### `src/Orbis.Stream.Core/Messages/messages_hi.properties`

SHA-256: `65300943c47e8bfed15182e8fa2f69191be5f4b8e870edcc19dbc67567bb0ca0`.

### `src/Orbis.Stream.Core/Messages/messages_hod.properties`

SHA-256: `018e00334d3fa8afd20d216deaf0d54118e9b279ad65573dc4a55c69472f1b69`.

### `src/Orbis.Stream.Core/Messages/messages_it.properties`

SHA-256: `a1be8c67ba4a093a56995dfd4cfc443e344d22f0ae892c70f8d63b3c0082eb4c`.

### `src/Orbis.Stream.Core/Messages/messages_ja.properties`

SHA-256: `17cc6dd36e01bfb906af585b3c4ce189ff2bcc4de43b80764fdf717d187f6afa`.

### `src/Orbis.Stream.Core/Messages/messages_ko.properties`

SHA-256: `3dfad663fb6853c3219dc6b0542b807ecf79c93fcde6796f8e7f4b52414cc06c`.

### `src/Orbis.Stream.Core/Messages/messages_la.properties`

SHA-256: `800b18fb1b63c10409a0466efe4de65fa4d6f39533053883209d4283590ca41c`.

### `src/Orbis.Stream.Core/Messages/messages_nl.properties`

SHA-256: `798c3343fe72da77759126ab7ee0a72a31bbb67df4325d6d3376898964b5b138`.

### `src/Orbis.Stream.Core/Messages/messages_pt.properties`

SHA-256: `45a6dea270fca6040c7f51656124b8d44f27113296bf73cfdebb5ea7a16112bc`.

### `src/Orbis.Stream.Core/Messages/messages_ro.properties`

SHA-256: `9e8ce18fe985ab021b987bc4de4b23c90f6c9183035c917ad2b9d4e120caa89f`.

### `src/Orbis.Stream.Core/Messages/messages_ru.properties`

SHA-256: `972c44419d9fa82d488bc169415719ff8950134aba75fe47cedb8984a71e6f5e`.

### `src/Orbis.Stream.Core/Messages/messages_tlh.properties`

SHA-256: `0ba0c49c5b4f0c2f8387e231e1c43d07c5b202b76621fabbe8e69441a5bdb445`.

### `src/Orbis.Stream.Core/Messages/messages_tr.properties`

SHA-256: `8199f7b5a97e7cc318abfa19ef3d26ef1cabfa9c44dfaf08638ef366ed24043b`.

### `src/Orbis.Stream.Core/Messages/messages_zh.properties`

SHA-256: `3bbf0249be6ff1edaa43439c4536adb9ee55b935daac26c62781d89e80cfad9d`.

### `src/Orbis.Stream.Core/Orbis.Stream.Core.csproj`

SHA-256: `30e2f94d0e68f9aff1c16143eb3e768585d9f0fe1060eaf4d7b25791614bcf2d`.

### `src/Orbis.Stream.Core/Pages/Countdown.cshtml`

SHA-256: `c9e582af32867a6b06b3d2c189f9c05bfab678b8b536d2bac17dc4a3930ed933`.

### `src/Orbis.Stream.Core/Pages/MainChannelSetting.cshtml`

SHA-256: `83ab73b6d3a61eb4f66efc661ae4dccf5cf1e925bdcc8f1efbc5e4f40c65163e`.

### `src/Orbis.Stream.Core/Pages/MainChannelSetting.cshtml.cs`

Tipi: `PlatformChoice`, `MainChannelSettingModel`.

Proprietà: `Configurations`, `Platform`, `Active`, `Editing`.

Metodi pubblici: `ChoiceOf`, `OnGet`, `OnPostDelete`, `OnPostSave`, `OnPostToggle`.

SHA-256: `b761f6cd3c3945e457db8c487b95d9b6d44e06418ed63c2c862d4efa84d4c483`.

### `src/Orbis.Stream.Core/Pages/MainLayout.cshtml`

SHA-256: `77af0c2dc526d27dda88d63f9e6c4690648ed434dde7ec92471e1c87dfc79fea`.

### `src/Orbis.Stream.Core/Pages/MainLayout.cshtml.cs`

Tipi: `MainLayoutModel`.

Proprietà: `Edit`.

Metodi pubblici: `OnGet`.

SHA-256: `f8644bb17f1dc583ba1181d58555564c9a2065ae52696edbea748e504ac6fe8f`.

### `src/Orbis.Stream.Core/Pages/MainLayoutList.cshtml`

SHA-256: `6654595ff035b793108abe13148dab64786a3366cf672d417132f9fef6820a9d`.

### `src/Orbis.Stream.Core/Pages/MainLayoutList.cshtml.cs`

Tipi: `MainLayoutListModel`.

Proprietà: `Layouts`.

Metodi pubblici: `OnGet`, `OnPostDelete`.

SHA-256: `9a3a894cb7f3fc4f853c7d33c925d143deba4b485ba10149c300ad497d2a6ddb`.

### `src/Orbis.Stream.Core/Pages/MainLive.cshtml`

SHA-256: `5a44642050fbf67c662fd7c2eb4fe9130ccac4ee21bbd34ee6d57c3475cf795a`.

### `src/Orbis.Stream.Core/Pages/MainLive.cshtml.cs`

Tipi: `MainLiveModel`.

Proprietà: `LiveStatus`, `ChannelName`, `Platform`, `PageIndex`, `Videos`, `Channels`, `Locked`, `ActiveSettings`, `ActiveConfigurations`, `LinkPkid`, `LinkHistory`, `Form`, `Start`, `ComposeScene`, `SettingId`, `ConfigurationId`, `Playlist`, `Folder`, `Details`, `OpenedPlaylist`.

Metodi pubblici: `OnGet`, `OnGetDetailsRows`, `OnGetRows`, `OnPostDelete`, `OnPostDeletePlaylist`, `OnPostLink`, `OnPostPlaylist`, `OnPostReplay`, `OnPostReplayFrom`, `OnPostRestart`, `OnPostSpot`, `OnPostStartScene`, `OnPostStop`, `OnPostUnlock`.

SHA-256: `cf634e9e752f94e3e68aecb4fd51d60faabb6f1e4acf2df4e0363bd078c02a15`.

### `src/Orbis.Stream.Core/Pages/MainLiveHistory.cshtml`

SHA-256: `27ae35160bb16b87e7e729b304569e2e414d0e173e927cfb5746963a1598e3fb`.

### `src/Orbis.Stream.Core/Pages/MainLiveHistory.cshtml.cs`

Tipi: `MainLiveHistoryModel`, `the`, `of`.

Proprietà: `PageIndex`, `LiveStatus`, `ChannelName`, `TitleSearch`, `Videos`, `Channels`, `AutoCleanupEnabled`, `AutoCleanupIntervalMonths`, `AutoCleanupOlderThanMonths`.

Metodi pubblici: `OnGet`, `OnPostDelete`, `OnPostSaveAutoCleanup`.

SHA-256: `aceaf8a6bdbb9bf323a9e2e16f1e91a421008c48daed58ddcc0c4301259385dc`.

### `src/Orbis.Stream.Core/Pages/MainLiveMenu.cshtml`

SHA-256: `d293f2ca8f858fa4864785623a1836bc9d137e4e0287fa9351372fdf7fe02d85`.

### `src/Orbis.Stream.Core/Pages/MainLiveMenu.cshtml.cs`

Tipi: `MainLiveMenuModel`.

Metodi pubblici: `OnGet`.

SHA-256: `5e73dc0c738495feb7fc135ae32383a0b6908ef09bb1cab0740a03e09a73b21e`.

### `src/Orbis.Stream.Core/Pages/MainManual.cshtml`

SHA-256: `9fa8f58b6e706195ecefd02191e68f0008aa9bd4319002fdb82aeb8e12207e76`.

### `src/Orbis.Stream.Core/Pages/MainMenu.cshtml`

SHA-256: `9aa25a715fc12ae0ea7e3d7154da44ce215514416f538b28bb7ca1f5a812b33c`.

### `src/Orbis.Stream.Core/Pages/MainPreview.cshtml`

SHA-256: `6142924a28871e4de452f0578c26e2d78366cd31471250ca24d2bca0b132594f`.

### `src/Orbis.Stream.Core/Pages/MainPreview.cshtml.cs`

Tipi: `MainPreviewModel`, `that`.

Proprietà: `Live`, `Compose`.

Metodi pubblici: `FramesUrlOf`, `OnGet`, `OnPostSpot`, `OnPostStop`.

SHA-256: `b87290dd269889527d93dd3740418d0becd5841458cfdcc984c1493ae555f020`.

### `src/Orbis.Stream.Core/Pages/MainSetting.cshtml`

SHA-256: `d4626e99d28684bce1edb01515c9a2ef78210cbd9d8bfc3971c2d4c7e1de0a53`.

### `src/Orbis.Stream.Core/Pages/MainTaskManager.cshtml`

SHA-256: `13d638252bd62d6d0e5d65332e66d958e71664886453c814bf459fd6b3a020bb`.

### `src/Orbis.Stream.Core/Pages/MainTaskManager.cshtml.cs`

Tipi: `MainTaskManagerModel`.

Proprietà: `Facts`.

Metodi pubblici: `OnGet`, `OnGetFacts`.

SHA-256: `f582840e9851cfe15708556b8cae2518c39e8a8181a0349f681d8b79d5a1aece`.

### `src/Orbis.Stream.Core/Pages/MainVideoSetting.cshtml`

SHA-256: `5ee7df24533b036218a12e8d8d471ff67e04dbcdf956e5f98beee6d492f3f8b8`.

### `src/Orbis.Stream.Core/Pages/MainVideoSetting.cshtml.cs`

Tipi: `MainVideoSettingModel`.

Proprietà: `Settings`, `Form`.

Metodi pubblici: `OnGet`, `OnPostDelete`, `OnPostDuplicate`, `OnPostSave`, `OnPostToggle`.

SHA-256: `aeaf82f7b750643e0e026b8194aab50b3df8f68ae7a88204c250109bedebb40d`.

### `src/Orbis.Stream.Core/Pages/OrbisPageModel.cs`

Tipi: `NoticeKind`, `OrbisPageModel`.

Proprietà: `NoticeText`, `NoticeKindName`.

SHA-256: `b48f328ee53b5d452bc31847400448da7f3cd9c065565fc6ea85b7fdeb7fc723`.

### `src/Orbis.Stream.Core/Pages/PageHelpers.cs`

Tipi: `PagerModel`, `SceneComposerModel`, `LiveStatusView`, `SettingTitleView`, `LiveLinkView`, `that`, `names`.

Proprietà: `Window`.

Metodi pubblici: `ClassOf`, `Css`, `Date`, `Key`, `LabelOf`, `MarkupOf`, `Of`, `PlatformOf`, `UrlOf`, `YouTubeChannelUrl`, `YouTubeStudioUrl`.

SHA-256: `146998a415245cb4b4c2d187c41e9cd988ae809fd8a19b3e0d48b51bec1404a3`.

### `src/Orbis.Stream.Core/Pages/Shared/_Layout.cshtml`

SHA-256: `72527945fbd5f9d4c240ac7c55a5f5fc1c8e09773caa8818434d151c1ee7e400`.

### `src/Orbis.Stream.Core/Pages/Shared/_Pager.cshtml`

SHA-256: `d36587b61f297d4a43931f4fbca85668645277905db6128c38cc708427a7e72e`.

### `src/Orbis.Stream.Core/Pages/Shared/_SceneDeckIcon.cshtml`

SHA-256: `1a4cbf0f3cc3f2509c8d89d7f50fab87e885280e1795ac259cf010fc51e8d88b`.

### `src/Orbis.Stream.Core/Pages/Shared/_SpotIcon.cshtml`

SHA-256: `61a95cdcab2c3ae518dcc51615bc613be9163abd9ef9247cd158658525be231c`.

### `src/Orbis.Stream.Core/Pages/Shared/_VideoSettingFields.cshtml`

SHA-256: `9664431a7b86622e698bdb3c8b717b13b7df07795dd3771e0b1b124cc782e749`.

### `src/Orbis.Stream.Core/Pages/VideoSettingForm.cs`

Tipi: `VideoSettingForm`.

Proprietà: `Id`, `Title`, `VideoCodec`, `VideoCodecName`, `PixelFormat`, `VideoBitrate`, `VideoFormat`, `GopSize`, `VideoWidth`, `VideoHeight`, `FrameRate`, `AudioCodec`, `AudioBitrate`, `Preset`, `Tune`, `LowLatency`, `AdaptiveBitrate`, `Quality`, `IsActive`.

Metodi pubblici: `CodecNameItems`, `FormatBitrate`, `FormatItems`, `From`, `Items`, `PresetItems`, `ToRequest`, `TuneItems`, `WriteBitrate`.

SHA-256: `d26c40454b8d1aacf7fd7e6f5b1021affb8658ec7d550c54b6768085c178478f`.

### `src/Orbis.Stream.Core/Pages/_AppStats.cshtml`

SHA-256: `a954523575b3820e58e4f91c2c343e6781b88b8c501f629e468a6062aa35bdd3`.

### `src/Orbis.Stream.Core/Pages/_LiveRows.cshtml`

SHA-256: `f9d8f113f68ff31c2c24357325ab818fcedcb53bd6cb025203393ed5fe569e0a`.

### `src/Orbis.Stream.Core/Pages/_PlaylistVideos.cshtml`

SHA-256: `73902cf7a83905e4f5678f8f7f3a15e4d0b6647e271053f1981776dc31fdd32e`.

### `src/Orbis.Stream.Core/Pages/_SceneComposer.cshtml`

SHA-256: `07b1dfe98aabc6633feb1f17bb8a6103f112c7cdea398fe69f92beb155f77061`.

### `src/Orbis.Stream.Core/Pages/_SceneDeck.cshtml`

SHA-256: `49272363885d652a2376e8b72ed1249033aef7f2d39b7d26533da7709ad5329b`.

### `src/Orbis.Stream.Core/Pages/_StartWizardConfiguration.cshtml`

SHA-256: `8a482a25af620b87b8fd69b2e38446ef4bffa2f7b839c83ce47a292a95ac59ac`.

### `src/Orbis.Stream.Core/Pages/_StartWizardSetting.cshtml`

SHA-256: `45eb916bc80a2a6aaa423753ffbc2046f78b76656cbaaf06d4290f69dedf8b17`.

### `src/Orbis.Stream.Core/Pages/_Stats.cshtml`

SHA-256: `e9a874b1645fa5ca8c2dad652545f9679dc14e72e98e0f49850fdaebf2499888`.

### `src/Orbis.Stream.Core/Pages/_SystemInfo.cshtml`

SHA-256: `dffad664bd3d4908d734d139cc35e40e2ab303900ca998228b2a3d9f954fd620`.

### `src/Orbis.Stream.Core/Pages/_ViewImports.cshtml`

SHA-256: `47666ab1e2add5914da44086abcfb4f24d71229110812d31ed640a21072c669c`.

### `src/Orbis.Stream.Core/Pages/_ViewStart.cshtml`

SHA-256: `62afaee3500962d2262cf59943e45ad8be3d3e0d26698b4152a0c4b7705fc129`.

### `src/Orbis.Stream.Core/Services/BackgroundTaskExecutor.cs`

Tipi: `BackgroundTaskExecutor`, `WorkItem`, `Worker`.

Proprietà: `QueueLength`, `ActiveWorkerCount`, `Thread`, `Cancellation`.

Metodi pubblici: `DisposeAsync`, `Execute`.

SHA-256: `4c33ff84e2273c3e43cd0bd0a86adb821e5194fa9af9f2dae38125d63a79424f`.

### `src/Orbis.Stream.Core/Services/EncoderTuningService.cs`

Tipi: `EncoderTuningService`.

Metodi pubblici: `HardwareEncoderAsync`, `ResolveAsync`, `TuneDefaultsAsync`.

SHA-256: `b9d11e500f2bdde7fcaf821e1b526aef9fa88689254830fed65b6e9727bad10a`.

### `src/Orbis.Stream.Core/Services/FfmpegVideoPlaylistStreamer.cs`

Tipi: `FfmpegVideoPlaylistStreamer`, `StreamOutcome`, `ReconnectBudget`, `PlannedVideo`, `PausedProgram`, `VideoGroup`, `IVideoPlaylistStreamer`.

Proprietà: `Used`, `StopReason`.

Metodi pubblici: `StreamPlaylistAsync`.

SHA-256: `f7c28c0de16584e4fb39f0b3dcaf8007f14ef54edf2845e318ef4871fab67141`.

### `src/Orbis.Stream.Core/Services/ImageService.cs`

Tipi: `ImageService`, `ImagePayload`.

Metodi pubblici: `LoadImage`, `SaveImage`.

SHA-256: `b20cd1bf756509090fe6cf7155442e623017a980e68b9fd4b32052baa5f23f3e`.

### `src/Orbis.Stream.Core/Services/LiveChangeNotifier.cs`

Tipi: `LiveChangeNotifier`.

Proprietà: `Version`.

Metodi pubblici: `Raise`, `WaitAsync`.

SHA-256: `d697b45b225d97565139c8268c1edaa8fe08ee2275460bcda58dbaba6288378f`.

### `src/Orbis.Stream.Core/Services/LiveHistoryCleanupService.cs`

Tipi: `LiveHistoryCleanupProgress`, `LiveHistoryCleanupService`.

Proprietà: `Progress`.

Metodi pubblici: `DeleteLive`, `Run`, `ThresholdOf`, `TryStart`.

SHA-256: `5b093c7c335297bd57ad426fb4e72a2bd8a3968323f57dbcbaf78d56df2c7c85`.

### `src/Orbis.Stream.Core/Services/LivePreviewService.cs`

Tipi: `LiveMedia`, `LiveParameters`, `LiveSource`, `LiveOption`, `LiveSnapshot`, `LivePreviewService`.

Metodi pubblici: `ApplyParameters`, `ApplyVolume`, `EmbedAsync`, `Offline`, `Snapshot`.

SHA-256: `d017948663daa3abd2b2e73c5bb1c316a156a0f7412c4ba1edacd6315fe92e52`.

### `src/Orbis.Stream.Core/Services/LiveTakeovers.cs`

Tipi: `TakeoverKind`, `Takeover`, `ActiveSceneOverlay`, `LiveScene`, `LiveSceneState`, `LiveTakeovers`, `Live`.

Proprietà: `Lease`, `Pending`, `Current`, `Ended`, `ActiveOverlay`.

Metodi pubblici: `ActiveOverlayOf`, `BackToProgram`, `Close`, `CurrentOf`, `HasPending`, `IsOpen`, `LiveSceneOf`, `Of`, `OfOverlay`, `Open`, `Request`, `RequestOverlay`, `Resume`, `ShouldEnd`, `StopOverlay`, `TryTake`, `Uses`.

SHA-256: `405760070cf7eb355e0db996d1ae5a7b31665071a4c6e75a6caeacbef6124487`.

### `src/Orbis.Stream.Core/Services/MediaProxyService.cs`

Tipi: `MediaInput`, `MediaProxyService`.

Metodi pubblici: `Dispose`, `IsHdr`, `Needs`, `Prepare`, `Resolve`.

SHA-256: `bea84f48da7a857e0a8afedbdf00bda1951e1d42097d5a5ba23017b3b30f8881`.

### `src/Orbis.Stream.Core/Services/MediaStore.cs`

Tipi: `MediaStore`.

Proprietà: `Directory`, `MaxBytes`.

Metodi pubblici: `Accepts`, `AddAsync`, `Contains`, `ContentTypeOf`, `Files`, `LabelOf`, `PathOf`, `StemOf`, `Sweep`, `TryDelete`.

SHA-256: `3fbf55b6406a797eabaf18fe70b403253a38ac1a1bb5d7ee507c15ce20a18bc3`.

### `src/Orbis.Stream.Core/Services/NaturalFileNameComparer.cs`

Tipi: `NaturalFileNameComparer`.

Metodi pubblici: `Compare`.

SHA-256: `fbc052306b42583faf81ececf43bb6097bd84d07e4944131e145ec85d745a7e9`.

### `src/Orbis.Stream.Core/Services/OverlayLibrary.cs`

Tipi: `OverlayEntry`, `OverlayFile`, `with`, `OverlayLibrary`.

Metodi pubblici: `AddAsync`, `Contains`, `Delete`, `Find`, `IsOverlayFile`, `List`, `MediaOfAsync`, `StillAsync`.

SHA-256: `e038d59d70c21ddddd295fb5b419ef192af9202822623a4180b00fc6e43b0fb3`.

### `src/Orbis.Stream.Core/Services/SceneButtonService.cs`

Tipi: `SceneButtonResponse`, `SceneMediaResponse`, `SceneButtonService`.

Metodi pubblici: `AddMediaAsync`, `Create`, `Delete`, `Find`, `KindOf`, `List`, `PathOf`, `Update`.

SHA-256: `b058300c9e58137124274e7344ce1cafa6cd6236f0eb56cdc59b757b1170ba2a`.

### `src/Orbis.Stream.Core/Services/SceneReference.cs`

Tipi: `SceneReference`.

Metodi pubblici: `IsScene`, `ItemPath`, `Of`.

SHA-256: `9ca14a4235ac7f6af508afbb2273375bdf8fefaefdb950b7acc9a5092995dbe0`.

### `src/Orbis.Stream.Core/Services/SceneRows.cs`

Tipi: `SceneRows`.

Metodi pubblici: `BaseOf`.

SHA-256: `5da9edda45d793cd321eaa786cfbe955d6f1d85f21aad6bdfd8311da8c32a2b8`.

### `src/Orbis.Stream.Core/Services/SceneService.cs`

Tipi: `SceneService`.

Metodi pubblici: `Delete`, `GetLayouts`, `GetOne`.

SHA-256: `a96b3d1962411399a877219ee2f8d9d21e624a4e5f18040eb7ffea14e3b47320`.

### `src/Orbis.Stream.Core/Services/SettingService.cs`

Tipi: `SettingService`.

Metodi pubblici: `AddNewConfiguration`, `DeleteAStreamingSetting`, `ModifySetting`, `RetrieveChannel`, `RetrieveDirectoriesSettingsPath`, `RetrieveSettings`, `SaveAutoCleanup`.

SHA-256: `640601fe09bb5f27e883a2b37a70c77b36138e7a3e26d8297eaa1b6ebd7b1b15`.

### `src/Orbis.Stream.Core/Services/SourceCatalogService.cs`

Tipi: `SourceOption`, `ISourceProvider`, `DisplaySourceProvider`, `DisplayMonitors`, `Monitor`, `CameraSourceProvider`, `FileSourceProvider`, `SourceCatalogService`.

Metodi pubblici: `Enumerate`, `List`.

SHA-256: `841669fc185b31df118a2f190b29317776ee5ef81900c83476a6a79f44a6400a`.

### `src/Orbis.Stream.Core/Services/SourceSnapshotService.cs`

Tipi: `SourceSnapshotService`.

Metodi pubblici: `GrabAsync`.

SHA-256: `0756164a22290b8107e998bcf8ccd4f3350ba5c9cef1c11bc139eab18370b757`.

### `src/Orbis.Stream.Core/Services/StreamingService.cs`

Tipi: `StreamingService`, `too`, `means`, `FileFacts`.

Metodi pubblici: `EnqueueSpot`, `PlaySceneButton`, `PrepareStartFrom`, `ResetFlag`, `RestartFromBeginning`, `ResumeProgram`, `SceneStateOf`, `Shutdown`, `StartLive`, `StartPlaylist`, `StartSceneLive`, `StartVideo`, `StopAllAsync`, `StopBecause`, `StopVideoStreamingByPkid`.

SHA-256: `f961c0acce3cc119e1d16f73a65c130153a3dbf393d4da8ea4c1ac35cdfab22e`.

### `src/Orbis.Stream.Core/Services/SystemInfoService.cs`

Tipi: `SystemInfoService`.

Metodi pubblici: `GetAllSystemInfo`, `GetAppCpuPercent`, `GetAppDiskPercent`, `GetAppGpuPercent`, `GetAppNetworkPercent`, `GetAppRamPercent`, `GetCpuPercent`, `GetCpuTemperature`, `GetFfmpegProcessStats`, `GetGpuTemperature`, `GetRamPercent`, `GetSwapPercent`, `GetSystemFacts`.

SHA-256: `c88c3c1cac5f4747a3dc82b6494071de01d607cccbabd2f62d827d92d9cfe3c5`.

### `src/Orbis.Stream.Core/Services/VideoService.cs`

Tipi: `VideoService`.

Metodi pubblici: `DeleteLiveHistory`, `DeleteOldLiveHistory`, `DeletePlaylist`, `DeleteVideo`, `FindVideo`, `GetAllChannelOnline`, `GetAllVideoList`, `GetLivePage`, `GetLockedVideos`, `GetPlaylist`, `GetVideoChannelNames`, `GetVideoList`, `UnlockVideo`.

SHA-256: `2e91cdb490cec34c5591ce16569df87eb1c0625f9f403be6085713546d44075f`.

### `src/Orbis.Stream.Core/Services/VideoSettingService.cs`

Tipi: `VideoSettingService`.

Metodi pubblici: `DeleteVideoSetting`, `EditSettingsVideo`, `FindSettingOf`, `GetAllVideoSettings`, `LinkAndSaveSettingsPlaylist`, `LinkAndSaveSettingsVideo`, `SaveSettingsVideo`.

SHA-256: `a2e2d72ceb024ee2a54cd94e0b1762466765ef28a900bcf7f7ed0caecff69ce3`.

### `src/Orbis.Stream.Core/Services/YouTubeAirState.cs`

Tipi: `YouTubeAirState`, `Live`.

Metodi pubblici: `Dismiss`, `Forget`, `IsOffAir`, `IsUnverified`, `Missing`, `Observe`, `ObserveEnding`, `OnAirSince`, `OnAirVideoOf`.

SHA-256: `856d1865b39b20d47085813139dff0716f3fe115e498128c9142d7d21690ccbc`.

### `src/Orbis.Stream.Core/Services/YouTubeAirWatch.cs`

Tipi: `does`, `YouTubeAirWatch`.

Metodi pubblici: `Dismiss`, `IsOffAir`, `IsUnverified`, `OnAirVideoOf`.

SHA-256: `295888495f26fd59dd57b65b9baa8b314747e4679d6c01afd057fed07a921235`.

### `src/Orbis.Stream.Core/Streaming/BitrateLadder.cs`

Tipi: `RateAdaptation`, `struct`, `RateDecision`, `BitrateLadder`, `struct`.

Proprietà: `Cap`.

Metodi pubblici: `BitrateFor`, `Observe`.

SHA-256: `3c82f41089e9b375989c90df65e726dc71b5dec522685fdeb2d3c2ae3a1e851e`.

### `src/Orbis.Stream.Core/Streaming/FfmpegCommandBuilder.cs`

Tipi: `FfmpegStreamRequest`, `FfmpegCompositionItem`, `ProxyEncoding`, `FfmpegCompositionRequest`, `FfmpegCommandBuilder`.

Metodi pubblici: `Build`, `BuildComposition`, `BuildEncoderTrial`, `BuildProxy`, `BuildSender`, `BuildSnapshot`, `BuildStreamingUrl`, `CarriesSound`, `Fit`, `FrameOfLive`, `Letterbox`, `OverlayTile`, `PresetArguments`, `ResolveOutput`, `ScaleFilter`.

SHA-256: `eed59b9b8ff34e54d6925d081209b86f3f3e268494120e7ae385faba894d09c7`.

### `src/Orbis.Stream.Core/Streaming/FfmpegProbe.cs`

Tipi: `MediaProbeResult`, `MediaOutput`, `FfmpegProbe`, `FfmpegToolLocator`.

Metodi pubblici: `CreateStartInfo`, `Parse`, `ProbeAsync`, `RunCapturedAsync`.

SHA-256: `acb2bc49ff0fa6801e36f4650c9f69b70d17ed8f084b56b758b8b805a16fa53d`.

### `src/Orbis.Stream.Core/Streaming/FfmpegStreamingSession.cs`

Tipi: `FfmpegStreamingSession`, `StreamingSessionRegistry`.

Proprietà: `Profile`, `OnAirFor`, `AdaptiveQuality`, `NominalBitrate`, `AudioBitrate`, `AdaptsBitrate`, `BehindFor`, `VideoPkid`, `InputPath`, `Probe`, `Output`, `StartedAt`, `StopReason`.

Metodi pubblici: `AbortAsync`, `AdaptBitrate`, `DisposeAsync`, `EndNaturallyAsync`, `HandOverAsync`, `ReadErrorAsync`, `RedactStreamKey`, `Register`, `Remove`, `RequestRestart`, `Start`, `StartComposition`, `StopAllAsync`, `StopAsync`, `TryGet`, `WaitForExitAsync`, `Watched`.

SHA-256: `6dba0c2005b6bf16776b280513a0b1920802664f2ffcb641055e6d8078b35d3c`.

### `src/Orbis.Stream.Core/Streaming/FlvPacedRelay.cs`

Tipi: `FlvPacedRelay`, `struct`, `RelaySegment`.

Proprietà: `IsOpen`.

Metodi pubblici: `Attach`, `Cancel`, `Close`, `Detach`, `Sample`, `Start`.

SHA-256: `601e61abee838d0294b08da7486f111282c00f10f078670d09582f857f550ae3`.

### `src/Orbis.Stream.Core/Streaming/FlvSink.cs`

Tipi: `FlvTag`, `IFlvSink`, `StreamFlvSink`.

Proprietà: `Type`, `Timestamp`, `Length`.

Metodi pubblici: `Abort`, `Complete`, `Open`, `Release`, `WriteHeader`, `WriteTag`.

SHA-256: `c5f04505ca11f394848aae20220b907825908f41a1a8d5f90a54080d37e01a6f`.

### `src/Orbis.Stream.Core/Streaming/HybridWaiter.cs`

Tipi: `HybridWaiter`, `ISleeper`, `HighResolutionTimer`, `TimerHandle`, `PlainSleeper`, `TimerResolution`.

Metodi pubblici: `Create`, `Dispose`, `Raise`, `Sleep`, `Ticks`, `TryCreate`, `WaitUntil`.

SHA-256: `035df7679ae20cb01e22f9047aa23f58d3d051778ccf3a8b0520e7273b0c082f`.

### `src/Orbis.Stream.Core/Streaming/LiveOutput.cs`

Tipi: `LiveOutput`.

Proprietà: `OutputUrl`, `Frame`, `Profile`, `QualityCap`, `Ladder`.

Metodi pubblici: `Abort`, `AdaptBitrate`, `Attach`, `Cap`, `CompleteAsync`, `DisposeAsync`, `For`, `Pin`, `TryLower`.

SHA-256: `fb8f5356ed2409daf7330060b5e935b0e189de43ac96cf79754d0b8e245aa4e4`.

### `src/Orbis.Stream.Core/Streaming/LivePlatformEmbeds.cs`

Tipi: `LivePlatformEmbed`, `LivePlatformEmbeds`, `LivePlatformLookup`.

Metodi pubblici: `Failed`, `HasEndedAsync`, `Of`, `ResolveAsync`, `TwitchUrl`.

SHA-256: `fb2f6fae89d6b1c4a5d5ed6952f871d8a1caff975c946b0aa0fc857e00f17c33`.

### `src/Orbis.Stream.Core/Streaming/LivePreviewFrames.cs`

Tipi: `PreviewFrame`, `LivePreviewFrames`, `Slot`.

Proprietà: `Gate`, `Latest`, `Version`.

Metodi pubblici: `Forget`, `Latest`, `NextAsync`, `PathOf`.

SHA-256: `69f20244e11eca0f2b05b6dcad0ae1776416419cc983ac1d68ec4a912fb6d3f1`.

### `src/Orbis.Stream.Core/Streaming/LiveStreamKeys.cs`

Tipi: `LiveStreamKeys`.

Metodi pubblici: `IsAskedFor`, `IsStandIn`, `StandInFor`.

SHA-256: `7a1b1c73e39f3c815795edadcdb9dd7d2d76c2c656f61e6d7a8f2f0c2c9d2e7b`.

### `src/Orbis.Stream.Core/Streaming/MonitorTarget.cs`

Tipi: `MonitorTarget`.

Metodi pubblici: `Of`, `TryParse`.

SHA-256: `424061f06190daaaabefb07327d921724629c30a8efbe5b637b1658f19debd82`.

### `src/Orbis.Stream.Core/Streaming/OverlayMedia.cs`

Tipi: `OverlayMedia`.

Metodi pubblici: `Of`.

SHA-256: `49a5c757e830497df963135cfa0f1a68454c7643f694b08124091abf4c634a2e`.

### `src/Orbis.Stream.Core/Streaming/Rtmp/Amf0.cs`

Tipi: `Amf0Writer`, `Amf0Reader`.

Metodi pubblici: `Boolean`, `Null`, `Number`, `Object`, `ReadAll`, `String`, `ToArray`.

SHA-256: `e8e9217a19453e174752f73bd50562dc4eb9b0554b5d6e7b285b19f5e146bc50`.

### `src/Orbis.Stream.Core/Streaming/Rtmp/RtmpPublisher.cs`

Tipi: `RtmpException`, `RtmpPublisher`, `InboundChannel`.

Metodi pubblici: `Abort`, `Complete`, `Dispose`, `Open`, `WriteHeader`, `WriteTag`.

SHA-256: `c00dd2455ef6840ad36d9bdf11c798c4719eeb8262b9b68c0170ac12ff11d5bc`.

### `src/Orbis.Stream.Core/Streaming/StreamPlatform.cs`

Tipi: `StreamPlatform`, `RelayTransport`, `RelayPacing`, `StreamPlatformProfile`, `StreamPlatforms`.

Metodi pubblici: `Detect`, `For`, `NormalizeIngestUrl`, `Orient`, `WithTransport`.

SHA-256: `e45dd690223ab75f574e34504b028d7e74a6b8f13eb49285602ec88c885a1351`.

### `src/Orbis.Stream.Core/SystemInfo/AppInfo.cs`

Tipi: `AppInfo`.

SHA-256: `25db9d2ee8f32eb308258389dc9e3dd531c0773ee861d8886a92fc0d761ccb9b`.

### `src/Orbis.Stream.Core/SystemInfo/SystemInfoProviders.cs`

Tipi: `SystemFact`, `FfmpegProcessStat`, `ISystemInfoProvider`, `SystemInfoProviderFactory`, `AppMetricsHelper`, `TemperatureCache`, `Reading`, `WindowsSystemInfoProvider`, `at`, `in`, `that`, `carries`, `PortableSystemInfoProvider`.

Metodi pubblici: `CreateDefault`, `FormatBytes`, `FormatUptime`, `GetAppCpuPercent`, `GetAppDiskPercent`, `GetAppGpuPercent`, `GetAppNetworkMbps`, `GetAppNetworkPercent`, `GetAppRamPercent`, `GetCpuPercent`, `GetCpuTemperature`, `GetFacts`, `GetFfmpegProcessStats`, `GetGpuTemperature`, `GetRamPercent`, `GetSwapPercent`, `IsUsableAt`.

SHA-256: `0c4e8ab6c32da392a5f5c9e5c4f2c6d9d239903099794a710f1116d1d4e0ece0`.

### `src/Orbis.Stream.Core/Ui/manual.ar.json`

SHA-256: `a3469b422b2b5864be3239dc0ea0bb063a58e721abcabd3af921199f3432a454`.

### `src/Orbis.Stream.Core/Ui/manual.da.json`

SHA-256: `2a335c11776c80db8a9e03bb4a20acd115d912a3124363da5efc20bac6917d33`.

### `src/Orbis.Stream.Core/Ui/manual.de.json`

SHA-256: `b7f54e7ca5066a8d45e54f2283a47f55e42bc077acb0757c3c51335fedf4d611`.

### `src/Orbis.Stream.Core/Ui/manual.el.json`

SHA-256: `50792dae2c0d9c80f40f18e7f0f28c86ff2e6278fab0cc57b2088dcabcc2048c`.

### `src/Orbis.Stream.Core/Ui/manual.en.json`

SHA-256: `725bc786b8778d288b4410cd05dea7ec3831e2acab2bbe0e32dcaa47dd6f5a5d`.

### `src/Orbis.Stream.Core/Ui/manual.es.json`

SHA-256: `4c14fd6ab23157438869d1f7af1ac90612288f83339f8163505d3b4aa19d8f18`.

### `src/Orbis.Stream.Core/Ui/manual.fil.json`

SHA-256: `55bb70cef390570bf3460668d7553ef449ba861027006bfc2a54010f96ccc370`.

### `src/Orbis.Stream.Core/Ui/manual.fr.json`

SHA-256: `7327757ecbb340eae6bfb7e10987c8ecc3a073fc67774b5f30b5e516f148ab4d`.

### `src/Orbis.Stream.Core/Ui/manual.hi.json`

SHA-256: `0b947eb1a8ebe10a4163ed301a473797bc792b3f36610848cc2908430842e8c1`.

### `src/Orbis.Stream.Core/Ui/manual.hod.json`

SHA-256: `dba78c2b6c1d7861c17b1a503a2eb4e4adff4155d683eb0603eaea8078fe9c3b`.

### `src/Orbis.Stream.Core/Ui/manual.it.json`

SHA-256: `17be7419b9aa3122ec53a15d74ba6d397ef0401072760a08dce5dcc05d555981`.

### `src/Orbis.Stream.Core/Ui/manual.ja.json`

SHA-256: `35a0035e6a37f9dffec0470ea743771fb585d8b0bbfb75ea49e75c9504239c65`.

### `src/Orbis.Stream.Core/Ui/manual.ko.json`

SHA-256: `214b1cfd9e060a302b4915ff74d84caf1ca35ff13580468604c2fce8be3ce248`.

### `src/Orbis.Stream.Core/Ui/manual.la.json`

SHA-256: `80ec1037dadd1b5ac686060cf47896c63fff1e088013798458b86ea0bba434bf`.

### `src/Orbis.Stream.Core/Ui/manual.nl.json`

SHA-256: `b5e0a6af12d3e354361ae5110391f67b8f0736b93e689852fccfe4cd8efb0864`.

### `src/Orbis.Stream.Core/Ui/manual.pt.json`

SHA-256: `05faf8fcbb26cc87c33b75ba291fd0fcc37055c505df22c246c17362856b8a60`.

### `src/Orbis.Stream.Core/Ui/manual.ro.json`

SHA-256: `706351cb03edb567a5cefe6975e1ddb94ef9cbe521c437ea403d95a51416c634`.

### `src/Orbis.Stream.Core/Ui/manual.ru.json`

SHA-256: `dc8a1a9fea31c599613e7115ea0fdc014bc88b15e4dc01ebd1a939888b8ee193`.

### `src/Orbis.Stream.Core/Ui/manual.tlh.json`

SHA-256: `b878c3e180ab8731cc88919220124b5f69e629c209c7112dd8f4481030f3462f`.

### `src/Orbis.Stream.Core/Ui/manual.tr.json`

SHA-256: `1d7459f466320f5758e65a84609ba684554f3258dfab2754423c020c4182c338`.

### `src/Orbis.Stream.Core/Ui/manual.zh.json`

SHA-256: `9473ce491e0697b70e57f5b782433521dc6f93d26f62faa72fae86e2d5cb44f4`.

### `src/Orbis.Stream.Core/Ui/ui.ar.json`

SHA-256: `19fa4b103eec238d3d6737f364647da7f933e0e0796461c6e580634f1f183938`.

### `src/Orbis.Stream.Core/Ui/ui.da.json`

SHA-256: `9d7d9fafa58c48d67cd7469c051a5b9ba44d2cf9b8ee8b04f8098799bc26f948`.

### `src/Orbis.Stream.Core/Ui/ui.de.json`

SHA-256: `81c8e7c7d18fba311d8f1a4175e54e50466e5796bf19e3c02ba095113997a307`.

### `src/Orbis.Stream.Core/Ui/ui.el.json`

SHA-256: `09e046e0cdd41470cc40f91e6ea144ec6991e5f1b1259670f8f1ec7de9625586`.

### `src/Orbis.Stream.Core/Ui/ui.en.json`

SHA-256: `8fe17a62131af67a3c46664c07d8db319d1ad6040f3f4422fa8286ce8e4d24c3`.

### `src/Orbis.Stream.Core/Ui/ui.es.json`

SHA-256: `6a6f303710cdad20c50e611814d036de46b4e3a773e05c18498c5db9b24d6493`.

### `src/Orbis.Stream.Core/Ui/ui.fil.json`

SHA-256: `7a3b1fe8da20599d34a089ff81ba65d089dce7db635c4163e4abff89ce25c968`.

### `src/Orbis.Stream.Core/Ui/ui.fr.json`

SHA-256: `13bee91c2c66ff1f31d284096e10978adb45ee4add854f8b1d29dc5acdf85964`.

### `src/Orbis.Stream.Core/Ui/ui.hi.json`

SHA-256: `84a784d175067c650b977acbd8533f83ecf81a89daa4cd28da49e78a19cc7c5e`.

### `src/Orbis.Stream.Core/Ui/ui.hod.json`

SHA-256: `168267bb6f6dccafcc7e45d137c01c1d19c19ddfaefbe1e11f80d67e34b564a3`.

### `src/Orbis.Stream.Core/Ui/ui.it.json`

SHA-256: `0a8cb4e0a02b46a47e0838471db6dc21a1e2445395363ecfcba86a3612579ef5`.

### `src/Orbis.Stream.Core/Ui/ui.ja.json`

SHA-256: `f0d128b020ca86491381acacc6a04c2db07918a2b836cf39f37ffd6e69a28670`.

### `src/Orbis.Stream.Core/Ui/ui.ko.json`

SHA-256: `3fe17174a6dd548e01a72408d58825797795a077bb842c008b36fba7955af366`.

### `src/Orbis.Stream.Core/Ui/ui.la.json`

SHA-256: `c2845f592acd0fc38874cfb9f8f40821694b698cd9bf46bd793ac294354f9bb9`.

### `src/Orbis.Stream.Core/Ui/ui.nl.json`

SHA-256: `0e57efe8276ef23962e9d20af417081c21c80a365f29853c2937be8c2f7b4ac9`.

### `src/Orbis.Stream.Core/Ui/ui.pt.json`

SHA-256: `277eaf19904f570547dc29f63e257474860a1a1418e1835b1f95ebb9404f03fa`.

### `src/Orbis.Stream.Core/Ui/ui.ro.json`

SHA-256: `14038c227d8217ddce0bf32335cecf7f19d6893666d7fe1398e74ef2f840e12e`.

### `src/Orbis.Stream.Core/Ui/ui.ru.json`

SHA-256: `8a25f3f14e12144f539db21be9d9708940b59c71a079fa0b7533f2da9c2e97ab`.

### `src/Orbis.Stream.Core/Ui/ui.tlh.json`

SHA-256: `a7897555b4f5b290c83da73ff747b1db256449b3abcd982c2799039e0bf2fcab`.

### `src/Orbis.Stream.Core/Ui/ui.tr.json`

SHA-256: `c96b726c3aa94cab345c44835c1ab865e686e5606d5a9900caccfd7d3fe1195a`.

### `src/Orbis.Stream.Core/Ui/ui.zh.json`

SHA-256: `a71efe43fcb6cd083045620648a0d1b1850783cb63f55195ad8fac2d325c9c6d`.

### `src/Orbis.Stream.Tests/AdaptiveBitrateSwitchTests.cs`

Tipi: `AdaptiveBitrateSwitchTests`.

Metodi pubblici: `OffTheLiveGoesOutAtTheBitrateOfTheSettingWhateverTheLadderDecided`, `OnlyAnExplicitNoTurnsItOff`, `SetReplacesWhatTheSettingSaidBefore`, `TheFormStartsOnAndCarriesTheSwitchBothWays`, `TheSwitchIsADecisionOfThisApplicationNotAnOptionOfFfmpeg`.

SHA-256: `9c394fce1a0a012a64fa952ba3a708d3b786a2adc97c715fdbf022438d827a41`.

### `src/Orbis.Stream.Tests/BitrateLadderTests.cs`

Tipi: `BitrateLadderTests`, `Relay`.

Proprietà: `Nominal`, `Elapsed`.

Metodi pubblici: `ADipTheRelayWinsBackIsNoReasonToSwitch`, `ALadderWithoutRulesOrASettingWithoutBitrateNeverMoves`, `ALiveTheNetworkCarriesKeepsTheBitrateOfTheSetting`, `ANetworkThatCarriesLessThanTheLiveBringsTheBitrateDownToWhatItCarries`, `ANetworkThatStoppedIsAStallNotABitrate`, `ANewConnectionKeepsTheBitrateTheNetworkLeftTheLiveAt`, `ASettingBroughtUnderTheCapIsTheBitrateOfTheLive`, `ASlowLeakIsCaughtOnceTheLiveHasSlippedTheMaxDrift`, `AStepUpTakenBackDoublesTheWaitBeforeTheNext`, `AnEncoderThatDoesNotKeepUpIsNotTheNetworksFault`, `OneStepNeverMoreThanHalvesTheBitrateAndTheFloorHoldsIt`, `Reconnect`, `Run`, `TheLiveClimbsBackInStepsOnceItWentOutClean`, `YouTubeRidesOutWhatTwitchAnswers`.

SHA-256: `394d5c7ff3d4d47594b32acf6e1c48e31d37357c3db3a746083bd090a3f90a22`.

### `src/Orbis.Stream.Tests/DatabaseTests.cs`

Tipi: `TemporaryDatabase`, `DatabaseSchemaTests`, `SceneRepositoryTests`, `VideoRepositoryTests`, `SettingRepositoryTests`, `LivePagePlatformTests`.

Proprietà: `LiveHistoryId`, `Directory`, `DatabasePath`.

Metodi pubblici: `Delete_RemovesTheRow`, `Delete_TakesTheItemsWithIt`, `Dispose`, `EnsureCreated_IsIdempotent`, `EnsureCreated_KeepsExistingData`, `EnsureCreated_MigratesAPreviousDatabase`, `EnsureCreated_MovesTheOldYouTubeIngest`, `EnsureCreated_TurnsTheOldScenesIntoLayouts`, `FindAll_ReturnsTheItemsInStackingOrder`, `FindAll_SupportsDynamicFilters`, `FindByLiveStatusAndChannelName_CombinesBothConditions`, `FindChannelNames_ListsEveryChannelOfTheVideosOnceAndInOrder`, `FindLayouts_LeavesTheScenesOfTheLivesOut`, `FindPaged_AppliesFiltersSortingAndPaging`, `FindPaged_CombinesTheChannelTheStatusAndTheSearch`, `FindPaged_KeepsOnlyTheRowsWhoseContainerIsAVideo`, `FindPaged_LooksTheWildcardsOfASearchForThemselves`, `FindPaged_SearchesTheTitleAsAPartOfItAndNotAsAllOfIt`, `FindPaged_SupportsTheVideoLiveHistoryJoin`, `FindPaged_WithoutTheWhitelistStillAnswerEveryRow`, `InsertAndFind_KeepsTheTileAndTheResolutionOfTheFile`, `InsertAndFind_RoundTripsEveryColumn`, `Insert_GeneratesTheNextIdentifier`, `Insert_ThenUpdate_ReplacesTheItemsInOrder`, `SetStopFlag_UpdatesThePollingFlagOfTheStreamer`, `SetVideoSetting_UpdatesOnlyTheConfigurationReference`, `SetVolume_SurvivesTheStreamerWritingTheRowItHolds`, `StartAsync_ReleasesTheLivesLeftRunning`, `TheLivePageListsTheLivesOfOnePlatform`, `Update_ReplacesEveryField`.

SHA-256: `d43e5526bad1f13ba9705aac917351ad783cd775a5b72b86705eba639757f400`.

### `src/Orbis.Stream.Tests/DomainAndPagingTests.cs`

Tipi: `LiveStatusTests`, `PageRequestTests`, `FilterValueConverterTests`, `AutoCleanupServiceTests`, `DatabaseBootstrapperTests`, `FfmpegCodecCatalogTests`, `values`, `UserPathTests`, `LiveLinkTests`, `SystemFactTests`, `TemperatureTests`, `LiveChangeNotifierTests`.

Metodi pubblici: `AChangeThatArrivesBetweenTheTestAndTheWaitIsNotLost`, `AListenerResumesFromTheChangeItAlreadyHas`, `AListenerThatArrivesLateIsAnsweredAtOnce`, `AListenerThatGoesAwayDoesNotHoldTheOnesThatStay`, `AListenerWaitsForAChangeAndIsToldWhichOneItIs`, `AMachineWithoutSensorsIsNotAskedAgainEverySecond`, `AudioCodecs_FollowTheFfmpegEnum`, `ChoiceOf_ReadsTheNameThenTheIngestOfAConfiguration`, `Convert_ParsesTheSupportedKinds`, `Convert_ThrowsTheOrbisQueryExceptionOfTheDynamicSpecification`, `DelayAsync_TakesAnIntervalOfMonthsWithoutStoppingTheApplication`, `EachPlatformHasItsOwnMarkAndItsOwnColour`, `EveryChangeHasItsOwnNumber`, `Facts_DescribeTheMachineThatRunsThem`, `Facts_NeverCarryAnEmptyValue`, `FormatBytes_UsesTheBinaryDivisorAndNoLocale`, `FormatUptime_IsReadableWithoutAnyLanguage`, `FormatUptime_OfAClockThatWentBackwardsIsZero`, `FromStorage_AlsoReadsTheWireNameAndTheOrdinalAsText`, `FromStorage_FallsBackToOfflineForUnknownValues`, `FromStorage_ReadsTheHibernateOrdinal`, `KickAndFacebookGamingHaveTheirNamesMarksAndColours`, `LabelAndMark_AreTheOnesOfTheConfigurationForm`, `ManyListenersAreAllWokenByTheSameChange`, `NormalizeUserPath_StripsCopyAsPathQuotes`, `Ordinals_MatchTheHibernateColumnOfTheJavaEntity`, `Parse_ClampsThePagingParameters`, `Parse_ReadsTheSpringSortSyntax`, `Parse_UsesTheDefaultPageSizeAndSort`, `PixelFormats_FollowTheFfmpegEnum`, `PlatformOf_HasNoAnswerForAnIngestItDoesNotKnow`, `PlatformOf_KnowsKickAndFacebookByTheDomainOfTheirIngest`, `PlatformOf_ReadsTheIngestOfTheLive`, `PlatformOf_StillWantsThePresetForTheIngestsThatAreTheSameForEverybody`, `Schema_TurnsTheFfmpegDeliveryOnForYouTubeOnceAndOffForTwitch`, `SpringPageMetadata_IsConsistentWithTheContent`, `StartAsync_AddsTheNewPlatformsOnceToAnInstallationThatHadTheOldOnes`, `StartAsync_CreatesOneDefaultConfigurationPerPlatform`, `StartAsync_KeepsASingleDefaultConfigurationPerPlatform`, `StartAsync_MovesTheLegacyYouTubeDefaultsButNotOnesTheUserEdited`, `StartAsync_RenamesThePlaceholderDefaultInsteadOfAddingAnother`, `StartAsync_SeedsKickAndFacebookGamingWithTheirOwnDefaults`, `StartAsync_SeedsTikTokStandingUp`, `StartAsync_SeedsYouTubeAtAFixedPictureWithNothingThatReplacesTheConstantRate`, `Temperatures_AnswerAPlausibleReadingOrNoneAtAll`, `Temperatures_AreReadOnceForAsManyAsksAsThereAre`, `TheChannelSettingsOfferTheFivePlatformsWithTheirIngest`, `TheLowDefaultsAreShownInTheLanguageOfThePage`, `UrlOf_EscapesWhatWouldBreakTheAddress`, `UrlOf_FallsBackToTheNameOfTheApiWhenTheChannelIsNotThere`, `UrlOf_HasNoLinkWithoutAPlatformOrAChannel`, `UrlOf_KickIsTheChannelAndFacebookGamingThePage`, `UrlOf_TakesTheChannelOfTheConfigurationNotThePlatform`, `UrlOf_TikTokIsTheLiveOfTheAccount`, `UrlOf_TwitchIsTheChannelPage`, `UrlOf_YouTubeBeforeTheBroadcastIsSeenIsTheControlRoomOfTheChannel`, `UrlOf_YouTubeIsTheLiveControlRoomOfTheBroadcastOnAir`, `VideoCodecName_WinsOverTheNumericCodec`, `VideoExtensions_AcceptTheSupportedContainers`, `YouTubeChannelUrl_IsThePageOfTheHandleOrOfTheId`.

SHA-256: `86119e95c3e221ea8d3e1780c1537b4d835705cb1ddaa606540c985146ef7333`.

### `src/Orbis.Stream.Tests/EncoderQualityTests.cs`

Tipi: `EncoderQualityTests`.

Metodi pubblici: `ALevelChosenByTheUserIsNeverMeasured`, `APresetTypedByHandWinsOverTheLevel`, `Auto_FallsBackToTheLightLevelOnAGpuThatDoesNotKeepUp`, `Auto_GetsTheBalancedLevelOnAMachineThatKeepsUp`, `EveryEncoderSpellsTheLevelItsOwnWay`, `TheDecisionsOfTheApplicationNeverReachFfmpeg`, `TheDefaultsAreLeftAloneWhenFfmpegDoesNotRun`, `TheDefaultsGoToTheGpuAndLeaveTheSettingsOfTheUserAlone`, `TheFormKeepsTheLevel`, `TheMeasuredLevelOverridesTheAutomaticOne`, `WithoutLowLatencyThePresetAndTheTuneAreRequired`.

SHA-256: `d7e75ae0358702984dc229427d5c3da15eb0cb9cbafdc2ccdcc929d9882dc057`.

### `src/Orbis.Stream.Tests/EndToEndApiTests.cs`

Tipi: `TestHostRunner`, `ApplicationFixture`, `EndToEndApiTests`, `VideoSnapshot`.

Proprietà: `DataDirectory`, `Port`, `WebRootPath`, `Client`, `NoRedirectClient`.

Metodi pubblici: `ALayout_KeepsOnlyItsSlots_AndTheScenesOfTheLivesAreNotListed`, `ANewScene_KeepsItsSourcesAndAnswersWithItsId`, `DisposeAsync`, `EveryPageSaysWhichBuildThisIsAndWhoWroteIt`, `FfmpegSenderSwitch_IsOnForANewConfiguration`, `FfmpegSenderSwitch_IsSavedAndSurvivesTheActiveToggle`, `GetAllVideo_IgnoresThePagingSortAndLanguageParameters`, `GetAllVideo_RejectsAnUnknownFilterProperty`, `GetPage_ReturnsTheSpringPageEnvelope`, `InitializeAsync`, `LanguageQueryParameterIsNotAFilter`, `LanguageSelector_NeverRedirectsOutsideTheApplication`, `LanguageSelector_StoresTheChoiceAndTranslatesThePages`, `LinkingASetting_EditsTheOneOfTheRow_AndCopiesASharedOneOnce`, `LowLatencySwitch_SurvivesASaveAndReadsBackAsItWasLeft`, `MainLive_RowFormsCarryTheVideoKeyInTheirAction`, `MainLive_StartLiveOpensWithUrlParameter`, `Pages_RenderOnTheServer`, `Root_RedirectsToTheClientRouter`, `Settings_AreCreatedListedAndDeleted`, `Start`, `StartLive_ValidatesTheRequestAndRecordsTheFailureOfTheStreamer`, `StaticFiles_AreAlwaysRevalidated`, `Swagger_IsExposedUnderTheDocumentedPaths`, `SystemInformation_IsServedAsPlainNumbers`, `TaskManager_PushesTheMetersInsteadOfBeingAskedForThem`, `TaskManager_ShowsTheMetersStraightAwayAndTheFactsOnTheirOwn`, `Temperatures_AreServedAsPlainNumbersOrAsNothingAtAll`, `TheDashboardAsksForAWeaselWithABanner`, `TheDefaultSettingsAreNamedAfterTheirPlatform`, `TheLiveHistoryPagesLikeTheLiveGridDoes`, `TheLiveHistory_LeavesTheImagesAndOtherOverlaysOut`, `UnknownPaths_AreNotFound`, `Updates_SaysWhenARowMovedAndOnlyWhenSomeoneIsListening`, `Updates_SendsRowsOnlyWhenARowReallyMoved`, `VideoSettings_ExposeTheDefaultPlatformConfigurations`.

SHA-256: `be497cd7572edf730eb93ed0774858938d9e7baf30b5efc0c4b984e58f6fd757`.

### `src/Orbis.Stream.Tests/FfmpegCommandBuilderTests.cs`

Tipi: `FfmpegCommandBuilderTests`, `FfmpegCompositionTests`, `SourceCatalogTests`.

Metodi pubblici: `ACameraOpenedForItsPictureCarriesNoSound`, `ACanvasIsNotBoundedWhileThereIsSomethingThatNeverEnds`, `ACanvasOfFilesBehindARelayIsPacedByTheRelayAlone`, `ACanvasOfFilesIsBoundedByItsLongestFile`, `ACanvasOfFilesWithoutARelayIsPacedOnceOnWhatComesOut`, `ACanvasStartedAgainFromWhereItStoppedStillLastsAsLong`, `ACanvasWithADeviceKeepsTheFilesInStepWithIt`, `ACompositionWithNoPictureIsRejected`, `ACompositionWithNoSizeIsRejected`, `ADeviceGetsARealTimeBufferLargerThanAFrame`, `AFileIsPacedAndSeeked`, `AMicrophoneIsHeardAndNotSeen`, `AMonitorIsTheDesktopCroppedToItsRectangle`, `AScreenCarriesNoSound`, `ASnapshotIsNotPaced`, `ATileWithNoRectangleIsRejected`, `AWebcamWithItsMicrophoneIsOneInputAndIsHeard`, `BuildStreamingUrl_JoinsWithASingleSeparator`, `Build_ComputesGopAsFrameRateTimesGopSize`, `Build_LeavesTheResolutionOfTheSourceAloneWhenTheSettingHasNone`, `Build_PassesTheInputPathAsOneVerbatimArgument`, `Build_RoundsTheScaleDownToAnEvenFrameSize`, `Build_ScalesToTheResolutionOfTheSetting`, `Build_SkipsAudioFlagsWhenTheInputHasNoAudioTrack`, `Build_TakesTheFrameRateOfTheSettingOverTheOneOfTheSource`, `Build_UsesTheVideoSettingAsEncoderConfiguration`, `Build_WithoutAPresetEncodesClean`, `Desktop_WebcamAndMicrophone_ComposeThroughOneGraph`, `DeviceList_OfFfmpeg4_IsReadBySection_AndHandlesColons`, `DeviceList_OfFfmpeg5_NamesItsOwnKind_AndHandlesColons`, `EveryInputIsOpenedBeforeTheGraph`, `EverySourceIsMixedAtItsOwnVolume`, `EveryTileStartsItsClockAtZero`, `MonitorTarget_RoundTrips`, `ResolveOutput_AnswersWhatTheCommandLineAsksFor`, `ResolveOutput_FallsBackToWhatTheSourceCarries`, `SettingFrameRate_WinsOverTheCanvasOne`, `SourcesAreStackedInTheOrderTheyWereDropped`, `TheCanvasIsABlankFrameOfTheSceneSize`, `TheEncoderIsTheSameOneAFileWouldUse`, `ThePreviewOfACanvasIsThirtySharpFramesASecond`, `ThePreviewOfAFileIsThirtySharpFramesASecond`, `TheStreamKeyNeverReachesAnErrorMessage`, `TileRectanglesAreRoundedDownToEvenPixels`, `TwoSoundsAreMixed`.

SHA-256: `ca110c4ce303ba7267408052936c3b4338a99d98d6fae7b77b49f6f07fcaf304`.

### `src/Orbis.Stream.Tests/FfmpegStreamingTests.cs`

Tipi: `FfmpegStreamingTests`, `VideoStatus`.

Metodi pubblici: `DisposeAsync`, `InitializeAsync`, `StartLive_StreamsThePlaylistWithFfmpegAndEndsTheVideo`.

SHA-256: `343b74e35a223f9f08ca1e4efadd0cf743a98f434e1feb09781ffb6126fe431f`.

### `src/Orbis.Stream.Tests/FileLoggerTests.cs`

Tipi: `FileLoggerTests`, `ManualTime`.

Proprietà: `Now`.

Metodi pubblici: `ALineCarriesItsTimeLevelCategoryAndException`, `ANewDayIsANewFileAndTheOldestOneGoes`, `Dispose`, `TheFileCanBeReadWhileTheLogIsOpen`, `TheFilesOfThreeDaysAgoAndBeforeAreDeleted`.

SHA-256: `10c1d0085bd6d38c5aa77a72e91155a1b1fbaee8cc4ce22f6faf095b8a7de8d2`.

### `src/Orbis.Stream.Tests/I18nTests.cs`

Tipi: `MessageFormatterTests`, `PropertiesBundleTests`.

Metodi pubblici: `Format_KeepsQuotedLiteralsVerbatim`, `Format_SubstitutesPositionalArguments`, `Format_UnescapesDoubledSingleQuotesLikeJavaMessageFormat`, `Format_WithoutArgumentsReturnsThePatternUnchanged`, `Parse_ReadsKeysValuesCommentsAndContinuations`, `Parse_UnescapesColonAndEqualsInsideValues`.

SHA-256: `62603695650a10cd8d2012df0361006dde0ea07a4652d2a0b3b58cf9d6ada420`.

### `src/Orbis.Stream.Tests/LiveFrameTests.cs`

Tipi: `LiveFrameTests`, `ConstantBitrateTests`.

Metodi pubblici: `ACanvasGoesOutAtTheBitrateTheNetworkCarriesToo`, `ACanvasOnAConnectionOfAnotherSizeIsFittedToo`, `ACustomIngestKeepsTheRateControlItHad`, `AKeyTheSettingNamesKeepsItsValue`, `ALandscapeLiveKeepsItsPreviewBoundedOnItsWidth`, `ALiveWithTheConnectionToItselfKeepsTheSizeOfTheFile`, `APlatformBoundsTheRateControlToOneSecondOfTheBitrate`, `APortraitCanvasIsPreviewedUprightToo`, `APortraitLiveIsPreviewedUpright`, `ASpotOfAnotherSizeIsFittedIntoThePictureOfTheLive`, `AVideoOfTheSizeOfTheLiveIsNotScaled`, `AmfIsAskedToPadToo`, `TheBitrateTheNetworkCarriesTakesThePlaceOfTheSetting`, `TheFirstEncoderOnAConnectionFixesItsPicture`, `TheFirstVideoFixesThePictureOfTheLive`, `TwitchKeepsEveryFileAtItsOwnSizeAndSound`, `TwitchKeepsTheVariableRate`, `TwitchLeavesX264ParamsAsTyped`, `X264ParamsTypedByHandGetTheConstantRateMergedIn`, `YouTubeGetsTheBitrateOfTheSettingAllTheTime`.

SHA-256: `2e6ca695e7e264ff4a6f227cad75edbaedc04ba8a9d8ec9c4ee7ffee67e65d1a`.

### `src/Orbis.Stream.Tests/LiveHistoryCleanupTests.cs`

Tipi: `LiveHistoryCleanupTests`.

Metodi pubblici: `DeletingALiveHistoryTakesItsVideosWithIt`, `DisposeAsync`, `InitializeAsync`, `RunningTheCleanupAnswersHowManyRowsItDeleted`, `SavingTheCleanupSettingsIsRememberedAndAnswersOnTheInfoBar`, `TheCleanupLeavesALiveThatIsOnAirAlone`, `ThePeriodsAreTheOnesThePicksSayTheyAre`, `ThePickOfNowTakesEveryLiveThereIs`, `TheRunButtonQueuesTheCleanupAndThePageCanFollowIt`.

SHA-256: `088e814eadc8bb30228d7bac873c678c2f1a6b43b86d020629b9e483561a796f`.

### `src/Orbis.Stream.Tests/LivePlatformEmbedTests.cs`

Tipi: `LivePlatformEmbedTests`, `RecordingHandler`.

Proprietà: `Asked`.

Metodi pubblici: `ALiveThatNamesTheChannelElsewhere_IsPlayedOnIt`, `ALiveWithoutAChannel_HasNoPlayer`, `AnIngestThisApplicationDoesNotKnow_HasNoPlayer`, `CandidateOf_TakesTheTopOfTheGrid_AndNotTheVideoElsewhereOnThePage`, `Twitch_AsksForTheNameOfThePageItIsPlayedOn`, `Twitch_IsPlayedOnTheChannelOfTheConfiguration`, `YouTube_ABroadcastThatHasEnded_HasNoPlayer`, `YouTube_AChannelThatIsNotOnAir_HasNoPlayer_AndNoReasonToBlameThePlatform`, `YouTube_APageThatNeverArrives_SaysSo_InsteadOfSayingTheChannelIsOffAir`, `YouTube_AStreamsPageThatArrivesEmpty_IsAChannelWithoutALive_NotAFailure`, `YouTube_AsksTheStreamsPageOfAChannelIdOnThePageOfAChannelId`, `YouTube_IsAskedOnce_WhileTheAnswerIsStillGood`, `YouTube_IsPlayedOnTheVideoTheChannelIsBroadcasting`, `YouTube_TrimsTheAtSignOfTheHandle`.

SHA-256: `2c0b22b83e6989267afebd374938ec37a6f070f09431a00c65bcee1bcdb47bef`.

### `src/Orbis.Stream.Tests/LivePreviewFrameFileTests.cs`

Tipi: `LivePreviewFrameFileTests`.

Metodi pubblici: `Dispose`, `Forget_TakesTheFrameAndTheOneTheRenameNeverMoved`, `Latest_AnswersTheFrameFfmpegHasWritten`, `Latest_GoesOnWithTheLastWholePictureWhenTheNextOneIsCaughtHalfWritten`, `Latest_IsNothingForAnEmptyFile`, `Latest_IsNothingWhenTheLiveHasNotWrittenAFrame`, `NextAsync_AnswersNothingWhenTheLiveWritesNothing`, `NextAsync_AnswersWithThePictureAfterTheOneThePageHolds`, `NextAsync_AnswersWithThePictureEvenWhenThePageHeldNothing`, `NextAsync_WaitsForThePictureTheLiveIsWriting`, `PathOf_IsTheJpegOfTheRowInsideThePreviewFolder`, `PathOf_MakesTheFirstPictureOfTheNextLiveOneThePageHasNotSeen`, `PathOf_TakesAwayWhatTheRenameLeftBehind`, `TheStampMovesWithThePictureAndNotWithTheWrite`.

SHA-256: `80c84902bf2221e91b054c561890c8a5f89b4afa8e67ced15f9e81193747033a`.

### `src/Orbis.Stream.Tests/LivePreviewTests.cs`

Tipi: `LivePreviewTests`, `VideoExtensionsTests`.

Metodi pubblici: `ALiveOnAPlatform_IsAlsoWatchableWithThePlayerOfThatPlatform`, `ALiveRowShowsTheMarkOfItsPlatformWithTheOtherIcons`, `ContentType_AnswersWhatTheBrowserAsksFor`, `DisposeAsync`, `InitializeAsync`, `IsBrowserPlayable_SaysWhatThePreviewCanShow`, `PreviewPage_IsServedWithAndWithoutALive`, `Preview_AnswersAnUnchangedRequestWithoutTouchingTheEncoder`, `Preview_AnswersTheStateOfTheLiveAndServesItsFile`, `Preview_AnswersWithTheFrameAsSoonAsItIsWritten`, `Preview_ChangesTheParametersOfARunningLive`, `Preview_IsEmptyWithoutALiveAndRefusesToChangeAStoppedOne`, `Preview_KeepsUpWithTheFramesFfmpegWrites`, `Preview_PutsTheResolutionBackToTheSourceWhenAskedForZero`, `Preview_RefusesAHalfResolution`, `Preview_RefusesAValueThatCannotBeEncoded`, `WithTwoLivesOnAir_ThePreviewPageOffersThemInAList`.

SHA-256: `a2906211145eccef34c452c6eca0600249e0c500aeb75e81c6be66b12addbe73`.

### `src/Orbis.Stream.Tests/ManualTextTests.cs`

Tipi: `ManualTextTests`.

Metodi pubblici: `EveryBundleCarriesTheSameSectionsInTheSameOrderAsEnglish`, `EveryLanguageHasItsOwnBundle`, `EverySectionKeepsItsFigureAndItsSteps`, `NoSectionIsLeftInEnglishInAnotherLanguage`.

SHA-256: `772d15368673eda23af8b273f1038f69f1cb8e24224a87f9e33267211c64ced1`.

### `src/Orbis.Stream.Tests/MediaProxyTests.cs`

Tipi: `MediaProxyTests`, `MediaProxyFfmpegTests`.

Metodi pubblici: `ACopyFitsTheBoxInTheShapeOfItsFileAndIsNeverBigger`, `AHeavyFileLaidOnACanvasGetsALightCopyTheLiveThenStreams`, `ALiveDecodesAFileOnTheGpuOnlyWhenItWasMeasuredFaster`, `ATrialTimesTheFirstSecondsIntoNothing`, `OnlyAFileBiggerThanFullHdGetsACopy`, `TheCopyIsScaledFirstAndToneMappedOnlyWhenHdr`, `TheGpuDecodesAndEncodesTheCopyWhenItWasTheFasterWay`, `TheProbeReadsThePixelFormatAndTheTransfer`.

SHA-256: `97d67f232b3637a0dfc087360bfc125fc70553796191a96f6fd9b224c77a9f1f`.

### `src/Orbis.Stream.Tests/MessageCatalogTests.cs`

Tipi: `MessageCatalogTests`, `in`.

Metodi pubblici: `EveryApostropheOfEveryBundleIsWrittenTwice`, `EveryBundleDefinesTheSameCodes`, `EverySupportedLanguageIsReallyTranslated`, `GetMessage_FallsBackToEnglishForUnsupportedLanguages`, `GetMessage_SubstitutesTheMessageFormatParameters`, `GetMessage_ThrowsWhenTheCodeDoesNotExist`, `GetMessage_UsesTheRequestedLanguage`, `ResolveLanguage_MapsToAnAvailableBundle`.

SHA-256: `90fa645b736a5188f2c66c077cffeb8eb5f2d05a986777a646e8ee01dc4a4f1c`.

### `src/Orbis.Stream.Tests/MessageCodeTests.cs`

Tipi: `MessageCodeTests`.

Metodi pubblici: `EveryMessageCodeTheSourcesUseIsDefined`.

SHA-256: `2d7135e986bccbd6df5f76c7c4320d07d87f4071e6d5fe8c28bc952caa15c930`.

### `src/Orbis.Stream.Tests/Orbis.Stream.Tests.csproj`

SHA-256: `da09e6f5e1d6b40bb5109e187cb5ca602aaa86317433e1fd3cd9b29b5c29ea85`.

### `src/Orbis.Stream.Tests/OverlayCommandTests.cs`

Tipi: `OverlayCommandTests`.

Metodi pubblici: `ACanvasOfFilesAndOverlaysIsPacedByTheRelayAlone`, `ACanvasStandsOnItsFirstSourceEvenWhenAnOverlayIsUnderIt`, `AStillIsDecodedOnceAndLaidWithItsAlpha`, `AStillIsOpenedOnceAndAnAnimationLoopsWithTheDecoderThatKeepsItsAlpha`, `AnAnimationLoopsForAsLongAsTheLiveThroughTheDecoderItNeeds`, `AnOverlayIsNeverHeard`, `AnOverlayIsNeverSoughtIntoWhenTheLiveResumes`, `OnlyPicturesAndVideosWithAlphaJoinTheLibrary`, `TheLibraryShowsTheNameAFileWasAddedWith`, `ThePageReadsTheFormatTheCodecTheFramesAndTheAlphaOfAWebM`, `TheStillOfAnOverlayKeepsItsAlpha`, `TheStoredNameIsSafeOnAnyDiskAndInAnyUrl`, `WithADeviceOnTheCanvasAnAnimationIsReadAtItsOwnPace`.

SHA-256: `8fa33d43f33da7a276d0bd151efb3c440ca34c9c93d71b5926cbf709534b5688`.

### `src/Orbis.Stream.Tests/OverlayLiveTests.cs`

Tipi: `OverlayLiveTests`.

Metodi pubblici: `ALayoutKeepsItsOverlaysWholeAndTheLibraryKeepsWhatALayoutUses`, `ALiveLaysItsOverlaysOverItsSourcesWithTheirAlpha`, `DisposeAsync`, `InitializeAsync`, `TheLibraryRefusesWhatIsNotAPicture`, `TheLibraryTakesAPictureOnceAndAnswersWithItByNameOnly`.

SHA-256: `bfe269cbc88afedec6660d50239d5c1142ea2028d72a143cdb83682daf4b437d`.

### `src/Orbis.Stream.Tests/PacedRelayTests.cs`

Tipi: `StreamPlatformTests`, `HybridWaiterTests`, `FlvPacedRelayTests`, `TimedStream`, `BrokenStream`, `RelayCommandTests`, `PacedRelayFfmpegTests`.

Proprietà: `Writes`, `StallOnWrite`, `Stall`.

Metodi pubblici: `Detect_ReadsThePlatformOffTheHost`, `Direct_WithoutAProfileTheCommandIsTheOneOfBefore`, `For_GivesEveryPlatformItsProfile`, `KickIsDeliveredTheWayTwitchIsAndFacebookGamingTheWayYouTubeIs`, `LiveOutput_LowersTheLevelOneStepAtATimeDownToTheLightest`, `LowLatencySwitch_KeepsTheEncoderDefaultsOnlyWhenItIsOn`, `LowLatencySwitch_ReachesTheHardwareEncoderToo`, `NormalizeIngestUrl_LeavesAnUnreadableUrlAlone`, `NormalizeIngestUrl_PutsYouTubeOnTheTlsItOnlyTakesIngestOver`, `Relay_AStreamThatMustCarrySoundMapsItAsMandatory`, `Relay_CountsWhatItSentForTheBitrateLadder`, `Relay_EncoderWritesFlvOnItsOutputAndLeavesThePacingToTheRelay`, `Relay_FailsWhenTheSenderIsGone`, `Relay_ForwardsEveryByteUnchanged`, `Relay_KeepsTheConnectionOpenBetweenEncoders`, `Relay_LaysEncodersEndToEndOnOneConnection`, `Relay_RefusesAStreamThatIsNotFlv`, `Relay_SendsEveryTagWhenItsTimestampIsDue`, `Relay_SendsFasterThanRealTimeWhileItIsBehind`, `Relay_SendsThePrerollAtOnce`, `Relay_SplicesTwoEncodersIntoOneContinuousStream`, `Relay_StartsTheClockOnAFullJitterBuffer`, `Relay_StreamsACanvasOfTwoFilesInRealTime`, `Relay_StreamsAFileInRealTimeAndIsOnAir`, `Relay_WinsTheGapBackInsteadOfWritingItOff`, `Relay_YouTubeGetsASilentTrackWhenTheFileHasNoSound`, `Relay_YouTubeSilenceFollowsTheLastSourceOfACanvas`, `Sender_CopiesThePacedStreamToTheIngestWithATimeout`, `ThePlatformsAdaptTheirBitrateAtTheirOwnPaceAndBoundTheirKeyframes`, `WaitUntil_HonoursCancellation`, `WaitUntil_ReturnsAtOnceForATargetInThePast`, `WaitUntil_WakesOnTimeAndNotBefore`.

SHA-256: `569875b9a5478b82de23557299eeec5f8ab3adb647c71b73c63503350e1caf7a`.

### `src/Orbis.Stream.Tests/PlaylistTests.cs`

Tipi: `NaturalFileNameComparerTests`, `PlaylistTests`, `where`.

Metodi pubblici: `ACanvasLastsAsLongAsItsLongestFile`, `DisposeAsync`, `EnqueueSpot_PausesCurrentVideo_PlaysSpot_AndResumesWhileRemainingLive`, `EnqueueSpot_StreamsSpotThroughAndResumesMainVideo`, `InitializeAsync`, `OrdersNumbersByValueAndIgnoresCase`, `StartPlaylist_RefusesAFileAndAMissingFolder`, `StartPlaylist_StreamsOnlyTheVideosInOrder_AndAStopHoldsForTheWholePlaylist`.

SHA-256: `5467a36c30c2e01b86b1eb7e1521ed5abe4c51b332d1fbf4b1ab0743fc3b5e7c`.

### `src/Orbis.Stream.Tests/RelayTransportTests.cs`

Tipi: `RelayTransportTests`.

Metodi pubblici: `ACustomIngestHasNoRelayWhateverTheChannelAsks`, `ATwitchSenderOnlyCopiesWhatTheRelayPaced`, `AYouTubeChannelWithTheSwitchOffGoesOutOnTheNativePublisherPacedByTheRelay`, `EveryLiveOnARelayHasTheBitrateLadderOfItsPlatform`, `FacebookGamingGoesOutThroughAnFfmpegSenderThatReadsAtRealTimeWithItsBurst`, `KickGoesOutOnTheNativePublisherPacedByTheRelayAndHasNoSwitch`, `OnlyYouTubeKeepsOneFormatForTheWholeConnection`, `TheYouTubeSenderReadsAtRealTimeWithThePrerollAsItsBurst`, `TwitchHasNoSwitch`, `TwitchKeepsTheRelayClockAndTheNativeRtmp`, `YouTubeGoesOutThroughAnFfmpegSenderThatKeepsTheTime`.

SHA-256: `4e75505f43b6847719f7bd868731f01fb6abb3eff669a7b6d3c6bad8b33a263a`.

### `src/Orbis.Stream.Tests/RtmpPublisherTests.cs`

Tipi: `Amf0Tests`, `RtmpPublisherUrlTests`, `RtmpPublisherFfmpegTests`, `OutputLogger`.

Metodi pubblici: `Amf0_ReadsBackWhatItWrites`, `Amf0_StopsAtWhatItCannotReadInsteadOfFailing`, `IsEnabled`, `Native_PublishesThePacedStreamToAnRtmpServer`, `Publisher_FailsWithinItsTimeoutWhenNobodyAnswers`, `Publisher_RefusesAUrlThatIsNotAnIngestWithAKey`.

SHA-256: `6b71de7a226108e980a65678bc8a1409dd8b2f7bae84b7dc165888854a9fbf6a`.

### `src/Orbis.Stream.Tests/RuntimeOptionsTests.cs`

Tipi: `RuntimeOptionsTests`, `TemporaryDirectory`.

Proprietà: `Path`.

Metodi pubblici: `ApplicationUrlAndEntryPoint_FollowThePort`, `DataDirectory_FallsBackToTheWorkingDirectory`, `DataDirectory_PrefersTheCommandLineOverTheEnvironment`, `Dispose`, `EmbeddedBrowser_CanBeDisabledFromBothTheCommandLineAndTheEnvironment`, `Ffmpeg_IsTakenFromTheSubdirectoryUsedByTheInstaller`, `Ffmpeg_PrefersTheEnvironmentVariable`, `Ffmpeg_WithoutConfigurationIsResolvedFromThePath`, `Ffprobe_FallsBackToTheSiblingOfFfmpeg`, `Port_IgnoresValuesOutsideTheValidRange`, `Port_PrefersTheCommandLineThenTheEnvironmentThenTheDefault`, `WebRoot_FallsBackToTheWwwrootBesideTheExecutable`, `WebRoot_PrefersTheCommandLine`.

SHA-256: `36724209ac13f384f4a7262a6eff1440f01b268d6e1a0d170fe1f5976f7ff438`.

### `src/Orbis.Stream.Tests/SceneButtonApiTests.cs`

Tipi: `SceneButtonApiTests`.

Metodi pubblici: `A_button_can_be_configured_as_in_scene_overlay`, `A_button_is_a_name_an_uploaded_file_and_a_key`, `A_live_that_is_not_running_takes_no_button`, `DisposeAsync`, `InitializeAsync`, `The_deck_refuses_what_it_could_not_put_on_air`.

SHA-256: `b839fbc513c45dfade225c9de6f3c0b162cb52903862c3e12efdc1c73cf059d1`.

### `src/Orbis.Stream.Tests/SceneDeckTests.cs`

Tipi: `LiveTakeoversTests`, `SceneHotkeyTests`, `SceneSwitchCommandTests`.

Metodi pubblici: `A_button_cuts_what_is_on_air_and_what_was_waiting`, `A_clip_in_place_of_the_live_is_given_silence_only_when_it_has_no_sound`, `A_file_on_air_or_waiting_is_in_use`, `A_key_the_page_does_not_need_is_taken`, `A_key_the_page_needs_or_does_not_know_is_refused`, `A_live_that_is_not_running_takes_no_request`, `A_picture_in_place_of_the_live_carries_a_silent_track_even_where_the_platform_needs_none`, `A_picture_is_fitted_whole_and_centred`, `A_picture_makes_way_for_a_spot_where_a_clip_would_not`, `A_picture_stays_and_a_clip_goes_back`, `An_overlay_can_be_placed_in_scene_and_stopped`, `An_overlay_without_duration_holds_indefinitely`, `Overlay_placement_calculation_computes_correct_coordinates`, `Overlay_timeline_enable_is_included_in_composition`, `Resume_ends_what_stands_in_and_drops_what_waited`, `The_close_of_an_old_run_leaves_the_new_one_alone`, `The_page_is_told_whether_what_is_on_air_stays`.

SHA-256: `7cbad58455ea7a96c73f241de65252d7e6c2d5a8b170d94f020fd1df8243ea70`.

### `src/Orbis.Stream.Tests/SceneLiveTests.cs`

Tipi: `SceneLiveTests`.

Metodi pubblici: `DisposeAsync`, `InitializeAsync`, `SceneLive_EnqueueSpot_StreamsSpotThroughAndResumesScene`, `SceneLive_IsOneRow_ShowsItsLightPreview_AndStopsFromItsRow`, `SceneLive_OfOneSource_LeavesTheConfigurationOfTheUserAlone`, `SceneLive_StopAndReplay_ResumesFromLastTimeStamp`, `SceneLive_StoppedFromAnyOfItsRows_StopsTheWholeComposition`, `SceneLive_TurnsLowLatencyOn_ForTheSettingOfTheRowsAndNotForTheOneTheUserChose`.

SHA-256: `56164d6c04adee0a0fb9a44d6ee3abe043830d4470ba8cbb69ba87f6916cda89`.

### `src/Orbis.Stream.Tests/SceneSwitchLiveTests.cs`

Tipi: `needs`, `SceneSwitchLiveTests`.

Metodi pubblici: `A_canvas_hands_over_to_a_picture_and_comes_back`, `A_picture_stands_in_until_it_is_resumed_and_a_clip_goes_back_by_itself`, `DisposeAsync`, `InitializeAsync`.

SHA-256: `c7a22cf6f60e61e7d73d69918470e9e32004d2c3b2f38aa2cab21a66c2bf47dd`.

### `src/Orbis.Stream.Tests/StyleSheetTests.cs`

Tipi: `StyleSheetTests`.

Metodi pubblici: `A_page_links_the_sheets_it_draws_and_only_those`, `Every_page_carries_the_shared_sheets_and_nothing_missing`, `Every_sheet_is_linked_by_at_least_one_page`, `No_sheet_names_a_custom_property_that_no_sheet_defines`.

SHA-256: `52541b8ed7f4ce93fe3534180d6044831c09552f969c03b4a5799e3273279d27`.

### `src/Orbis.Stream.Tests/TestPictures.cs`

Tipi: `TestPictures`.

Metodi pubblici: `Png`.

SHA-256: `7785458085062292056ded68f529a574310a2c03d8ab447c7b26b2f7c6749f30`.

### `src/Orbis.Stream.Tests/TikTokTests.cs`

Tipi: `TikTokDeliveryTests`, `TikTokKeyTests`.

Metodi pubblici: `ALandscapeCanvasOnTikTokIsFittedIntoTheUprightFrame`, `ALandscapeFileOnTikTokIsFittedIntoTheUprightFrameAndPreviewedUpright`, `A_configuration_of_TikTok_keeps_no_key`, `A_live_never_goes_out_with_the_stand_in`, `A_live_to_TikTok_started_again_goes_out_with_the_key_of_the_new_live`, `AnUprightCanvasGoesOutAsItIs`, `TheFirstVideoOfATikTokLiveFixesAPictureThatStandsUp`, `The_channel_page_saves_a_TikTok_configuration_without_a_key_and_never_shows_the_stand_in`, `The_key_of_a_live_to_TikTok_is_the_one_typed_for_it`, `The_wizard_asks_for_the_key_of_a_live_to_TikTok_only`, `TikTokGoesOutThroughAnFfmpegSenderThatReadsAtRealTimeWithItsBurst`, `TikTokIsDeliveredTheWayYouTubeIsStandingUp`, `TikTokStandsThePictureUp`.

SHA-256: `217af0fe3a905e3b7e0f6ea0337e645d15fa7a6bb5507ab7077919568a960f22`.

### `src/Orbis.Stream.Tests/UiTextTests.cs`

Tipi: `bundles`, `UiTextTests`.

Metodi pubblici: `EveryBundleDefinesTheSameKeysInTheSameOrderAsEnglish`, `EveryBundleIsReallyTranslated`, `EveryLanguageHasItsOwnFlagAndTheFileIsThere`, `NoBundlePrefixesItsStringsWithALanguageTag`, `TheSelectorOffersExactlyTheLanguagesTheMessagesAnswerIn`, `TranslationsKeepEveryParameterOfTheEnglishString`.

SHA-256: `0aa7ebc96b8071c9d9b6c343972837692eae16bb9ed675449357de1faa9994bb`.

### `src/Orbis.Stream.Tests/YouTubeAirWatchTests.cs`

Tipi: `YouTubeAirWatchTests`.

Metodi pubblici: `AChannelYouTubeKeepsFailingToAnswerForIsSaidToBeUnverified`, `ALiveFoundAgainForgetsTheEndingsCounted`, `ALiveFoundOnAirClearsTheFlagAndStartsCountingAgain`, `ALiveIsFlaggedOnlyAfterEnoughAnswersInARowSayItIsNotOnAir`, `ALiveIsForgottenOnlyAfterSomePassesWithNothingRunning`, `ALiveIsStoppedOnlyAfterEnoughAnswersInARowSayTheBroadcastEnded`, `APlatformThatCouldNotBeAskedLeavesTheMissesAsTheyWere`, `AnAnswerThatIsNotAnEndStartsTheCountAgain`, `ThePageOfAVideoSaysEndedOnlyWhenItIsALiveThatIsNoLongerLive`, `TheVideoOnAirIsRememberedAndFollowsABroadcastYouTubeReplaced`, `TheWaitRunsOnceForTheWholeLiveNotForEveryVideoOfIt`, `TheWarningStaysUntilTheUserClosesItAndComesBackOnlyAfterTheLiveWasFound`.

SHA-256: `06ab0e3ae7d39159e3497fc2550477c288d24f02b06d706e0265f6eaddd71b64`.

