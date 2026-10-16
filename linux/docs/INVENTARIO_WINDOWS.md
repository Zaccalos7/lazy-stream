# Inventario dei sorgenti Windows

272 file censiti, 62 route API. Snapshot SHA-256 disponibile in `windows-inventory.json`.

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
| GET | `/video/{pkid:int}/error-report` |
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

SHA-256: `9ffc0816dd8d6b7dedce3d9583fbc95a174f2e688056befe8cd3a5a11d0c1a37`.

### `Installer/fetch-ffmpeg.sh`

SHA-256: `8062f59267b1263c6a5de6aaab36170080fffa71bdb7b65a29f5dfd00f129994`.

### `Installer/installer.nsi`

SHA-256: `1239d703e57e4b5e79e474b375c39d69893297023a60dfc7a22ba55524cb010f`.

### `LICENSE`

SHA-256: `559fd2eab22fe11af69707fb6d0be5fe2d03e7dfc8fe8ff37071e5c2a04474f0`.

### `README.md`

SHA-256: `5b3a75405baa171eb7101071d2aec8249195d56ffae54e6e65dc360e638fcea1`.

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

SHA-256: `00acaad9f97bb8daf98282f8a79fbc20f8fedd6eba32b41426aa87156f0820c6`.

### `src/Orbis.Stream.App/Orbis.Stream.App.csproj`

SHA-256: `47c879e7516a4ac1d80b29f62f433a7151a751dab436e94352d7d58c154a01a8`.

### `src/Orbis.Stream.App/wwwroot/css/base.css`

SHA-256: `5db6e559e01eb48fedcc6145d8eb15758e9a5753bdf6154e7f9ee0680823c349`.

### `src/Orbis.Stream.App/wwwroot/css/cleanup.css`

SHA-256: `9a4a2966b57d52c816549ae0c331f734032c363bc89f70633c5f3f52b52219d1`.

### `src/Orbis.Stream.App/wwwroot/css/composer.css`

SHA-256: `455d58d4c3360f0f651cbf31b7bd785cc0900c0a31b12ebef5701a69520806de`.

### `src/Orbis.Stream.App/wwwroot/css/controls.css`

SHA-256: `df9caed8b4762d3138e30253e568b049b3163c3d7f403e585f5592ab8f6544f7`.

### `src/Orbis.Stream.App/wwwroot/css/countdown.css`

SHA-256: `fcbd69f01e27006b12c547844e3057a57a3b3631f87eb09060da114ccf61162a`.

### `src/Orbis.Stream.App/wwwroot/css/dashboard.css`

SHA-256: `13961623f1f2605fec24a1250040a1e6e65055c9a13f9b99e4a2febdeae2a3e8`.

### `src/Orbis.Stream.App/wwwroot/css/feedback.css`

SHA-256: `d9cba1c879c3567cce9e7afcb6e0ef9be9522f44393db030c768fd247258c801`.

### `src/Orbis.Stream.App/wwwroot/css/layout.css`

SHA-256: `55b191daaea922b3138e5ea38a1265147b12c35a72149b3aecad64286bd9d5ee`.

### `src/Orbis.Stream.App/wwwroot/css/manual.css`

SHA-256: `90306123fa2ea0c7fe6b303d79992a6eade8e1e09df184b71aa186903524a28e`.

### `src/Orbis.Stream.App/wwwroot/css/meters.css`

SHA-256: `f64ce9cd08c12dba83396388d97ccf54fe0ef333362897bb2ab53f590bbb125e`.

### `src/Orbis.Stream.App/wwwroot/css/overlays.css`

SHA-256: `3c47dc0d47fdb8c428298ad004a28f124f247b1f70d939348630d42f689fa699`.

### `src/Orbis.Stream.App/wwwroot/css/preview.css`

SHA-256: `921fc6a25460a05560aa3cc7bad39513b447ab6c855ff6a8a33b746e5d5d8842`.

### `src/Orbis.Stream.App/wwwroot/css/scenes.css`

SHA-256: `72e1e16e57f59c8bc1c8eec41d702ea1fc7894b14d76b0686c270067d9cb18cb`.

### `src/Orbis.Stream.App/wwwroot/css/surfaces.css`

SHA-256: `0b2f7af39ad659344a14e8a65c599ca155edb54ae4dcff6b6d24b3167be37396`.

### `src/Orbis.Stream.App/wwwroot/css/tokens.css`

SHA-256: `31e9770fc711c25607cf13f7b1ab3b54d3629d8f8ad0742a158619b69c436280`.

### `src/Orbis.Stream.App/wwwroot/js/composer.js`

SHA-256: `a0007790f4bb0e3ce8c3a6d0a3c831d8cbed6f88f7c4037270d76902f455cf46`.

### `src/Orbis.Stream.App/wwwroot/js/scenes.js`

SHA-256: `e783d7c11821f51b06db49b76d16f7aab51f7f6d55dd1ea2949b483b6b1ebf02`.

### `src/Orbis.Stream.App/wwwroot/js/site.js`

SHA-256: `03660218c06885edcc34a03c416c9c4606be8b2aa694bd8e1a5ea06c3c260c95`.

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

Metodi pubblici: `Delete`, `DeleteByLiveHistoryId`, `FindActive`, `FindByLiveHistoryId`, `FindByLiveStatus`, `FindByLiveStatusAndChannelName`, `FindByPkid`, `FindByScenePkid`, `FindByVideoPathAndVideoLiveHistoryPkid`, `FindChannelNames`, `FindLivePage`, `FindPaged`, `FindPkidsByVideoSettingId`, `From`, `Insert`, `SetStopFlag`, `SetVideoSetting`, `SetVolume`, `ToBoolean`, `ToInt64`, `ToNullableDateTime`, `ToNullableDouble`, `ToNullableInt32`, `ToNullableInt64`, `ToText`, `Update`.

SHA-256: `084b050db76c6b2421aae8ace5be3fea06772431cf9e5b05d7a5dfde3b92c011`.

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

Metodi pubblici: `FromStorage`, `IsActive`, `ToStorageValue`, `ToWireValue`, `TryParseWireValue`.

SHA-256: `e64bfa04b0710ba8fca46df983c22ca63eb176ff6f3db0d3d7ddc738511c6d5e`.

### `src/Orbis.Stream.Core/Domain/SceneButton.cs`

Tipi: `SceneButtonKind`, `SceneButtonEntity`, `SceneButtonKindExtensions`, `SceneHotkey`.

Proprietà: `Pkid`, `Position`, `Label`, `MediaName`, `Hotkey`, `DisplayMode`, `Placement`, `DurationSeconds`, `X`, `Y`, `Width`, `Height`, `LastModified`.

Metodi pubblici: `IsValid`, `ToWireValue`.

SHA-256: `7c1db8246cd511f90f7032edbed5871ac77107f818efd4a5bb5f1ec9f8cd241d`.

### `src/Orbis.Stream.Core/Domain/SourceKind.cs`

Tipi: `is`, `SourceKind`, `SourceKindExtensions`.

Metodi pubblici: `HasPicture`, `IsCaptureDevice`, `IsDefined`, `IsOverlay`, `ToWireValue`, `TryParse`, `TryParseWireValue`.

SHA-256: `96bf971f8624aee957c1c2fe28d8f28bf80268e353c34defadc23e2154b3f05e`.

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

SHA-256: `659fcd9a0a7f0c3170d94a453cf961e2ccf7a7c02c04e74b6a056736deff01d6`.

### `src/Orbis.Stream.Core/Hosting/EncoderTuningStartup.cs`

Tipi: `EncoderTuningStartup`.

Metodi pubblici: `StartAsync`, `StopAsync`.

SHA-256: `86630f7803fb907d165c43866eecefd34395ce7d362d222088bc7b56631c7377`.

### `src/Orbis.Stream.Core/Hosting/OrbisEndpoints.cs`

Tipi: `OrbisEndpoints`.

Metodi pubblici: `MapOrbisEndpoints`.

SHA-256: `9d9d43c245fc0509f148a99912e7cfa325e4405c8f59feeb4156a2eef33e9eef`.

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

SHA-256: `c0c1daa9b68bb0f179d892299321d504deedddc71c5bef17866253e6eabc3af3`.

### `src/Orbis.Stream.Core/Pages/Countdown.cshtml`

SHA-256: `c9e582af32867a6b06b3d2c189f9c05bfab678b8b536d2bac17dc4a3930ed933`.

### `src/Orbis.Stream.Core/Pages/MainChannelSetting.cshtml`

SHA-256: `a1f126e04049546aca63d3e8aac06335bf920252b0661bd15cd9a8f889900dcf`.

### `src/Orbis.Stream.Core/Pages/MainChannelSetting.cshtml.cs`

Tipi: `bundles`, `PlatformChoice`, `MainChannelSettingModel`.

Proprietà: `Configurations`, `Platform`, `Active`, `Editing`.

Metodi pubblici: `ChoiceOf`, `ChoiceOfSetting`, `OnGet`, `OnPostDelete`, `OnPostSave`, `OnPostToggle`.

SHA-256: `2de8ec6d74abceadc8d096411f7e35f95305b7c6ba4bd4d425bceba713dc7e6c`.

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

SHA-256: `799efcc684da6289255c04497eac33f6e164fa8e3f59b021c96aab1c66652c14`.

### `src/Orbis.Stream.Core/Pages/MainLive.cshtml.cs`

Tipi: `MainLiveModel`.

Proprietà: `LiveStatus`, `ChannelName`, `Platform`, `PageIndex`, `Videos`, `Channels`, `Locked`, `ActiveSettings`, `ActiveConfigurations`, `LinkPkid`, `LinkHistory`, `Form`, `Start`, `ComposeScene`, `SettingId`, `ConfigurationId`, `Playlist`, `Folder`, `Details`, `OpenedPlaylist`.

Metodi pubblici: `OnGet`, `OnGetDetailsRows`, `OnGetRows`, `OnPostDelete`, `OnPostDeletePlaylist`, `OnPostLink`, `OnPostPlaylist`, `OnPostReplay`, `OnPostReplayFrom`, `OnPostRestart`, `OnPostSpot`, `OnPostStartScene`, `OnPostStop`, `OnPostUnlock`.

SHA-256: `b8ba64eabda5249998d7ca948fccd0be79711294edc57cea8906f28aa6b3eaf0`.

### `src/Orbis.Stream.Core/Pages/MainLiveHistory.cshtml`

SHA-256: `279800c96455bd2940c3d8806dd8cb0faf64ddfd30b0c8f175ce37cabf124115`.

### `src/Orbis.Stream.Core/Pages/MainLiveHistory.cshtml.cs`

Tipi: `MainLiveHistoryModel`, `the`, `of`.

Proprietà: `PageIndex`, `LiveStatus`, `ChannelName`, `TitleSearch`, `Videos`, `Channels`, `AutoCleanupEnabled`, `AutoCleanupIntervalMonths`, `AutoCleanupOlderThanMonths`.

Metodi pubblici: `OnGet`, `OnPostDelete`, `OnPostSaveAutoCleanup`.

SHA-256: `37827f538bde80ce4bfeee6e98e4aeaa5e80cd94d8e34171e68b3ff008210ed5`.

### `src/Orbis.Stream.Core/Pages/MainLiveMenu.cshtml`

SHA-256: `d293f2ca8f858fa4864785623a1836bc9d137e4e0287fa9351372fdf7fe02d85`.

### `src/Orbis.Stream.Core/Pages/MainLiveMenu.cshtml.cs`

Tipi: `MainLiveMenuModel`.

Metodi pubblici: `OnGet`.

SHA-256: `5e73dc0c738495feb7fc135ae32383a0b6908ef09bb1cab0740a03e09a73b21e`.

### `src/Orbis.Stream.Core/Pages/MainManual.cshtml`

SHA-256: `f1fe933e3a68b7d2d17d3edd1b2a2669167b8421a0f57e02f35faf39b4fab8bd`.

### `src/Orbis.Stream.Core/Pages/MainMenu.cshtml`

SHA-256: `9aa25a715fc12ae0ea7e3d7154da44ce215514416f538b28bb7ca1f5a812b33c`.

### `src/Orbis.Stream.Core/Pages/MainPreview.cshtml`

SHA-256: `0632dc643c8437fefce610b13624d4dc4698c37cc5ffdc25b6c6afe51b08bebe`.

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

SHA-256: `e13d12b75bfaa77a226af169dff5f5ef97bd121d93a0f318076eb35e5c8390ff`.

### `src/Orbis.Stream.Core/Pages/OrbisPageModel.cs`

Tipi: `NoticeKind`, `OrbisPageModel`.

Proprietà: `NoticeText`, `NoticeKindName`.

SHA-256: `b48f328ee53b5d452bc31847400448da7f3cd9c065565fc6ea85b7fdeb7fc723`.

### `src/Orbis.Stream.Core/Pages/PageHelpers.cs`

Tipi: `PagerModel`, `SceneComposerModel`, `LiveStatusView`, `SettingTitleView`, `LiveLinkView`, `that`, `names`.

Proprietà: `Window`.

Metodi pubblici: `ClassOf`, `Css`, `Date`, `Key`, `LabelOf`, `MarkupOf`, `Of`, `PlatformOf`, `UrlOf`, `YouTubeChannelUrl`, `YouTubeStudioUrl`.

SHA-256: `5440effbe298b52c2a221203c3a39464ecb8a7afacd78fe0ff99069e894fc931`.

### `src/Orbis.Stream.Core/Pages/Shared/_Layout.cshtml`

SHA-256: `2933577f35de263a3e69929ee5fced668e68339b7f5593f58de0649aec511036`.

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

### `src/Orbis.Stream.Core/Pages/_ErrorReportButton.cshtml`

SHA-256: `bbc6b26d72dce9be2f5e3af30682e2a32d9ae11406400eea5653c75a4d46977e`.

### `src/Orbis.Stream.Core/Pages/_ErrorReportDialog.cshtml`

SHA-256: `b2c54eba23d0c3f7beaa85686aa770fb1802d38d8bea4f2988992b2aa99fa0d1`.

### `src/Orbis.Stream.Core/Pages/_FacebookLiveNotice.cshtml`

SHA-256: `86c000edfadbbbd86623ab20d4365aeadbd547388a5c9c80aed84cd48057f87f`.

### `src/Orbis.Stream.Core/Pages/_LiveRows.cshtml`

SHA-256: `106076c789d88bb32ddfba7f2ff5a7c89ee0bcf8f2461fa9b8b176929e8cd273`.

### `src/Orbis.Stream.Core/Pages/_PlaylistVideos.cshtml`

SHA-256: `c2114899553e8f6fd80d3817e4346379669ca3dc467b13220fd682a4aa6afbae`.

### `src/Orbis.Stream.Core/Pages/_SceneComposer.cshtml`

SHA-256: `07b1dfe98aabc6633feb1f17bb8a6103f112c7cdea398fe69f92beb155f77061`.

### `src/Orbis.Stream.Core/Pages/_SceneDeck.cshtml`

SHA-256: `49272363885d652a2376e8b72ed1249033aef7f2d39b7d26533da7709ad5329b`.

### `src/Orbis.Stream.Core/Pages/_StartWizardConfiguration.cshtml`

SHA-256: `84f4ccd84b96f14996e3e350aaeee669607175a82c98c1a98c948e4620374cee`.

### `src/Orbis.Stream.Core/Pages/_StartWizardSetting.cshtml`

SHA-256: `9fd3b6e00f20a14234fda173b1ac4b94e5986301386e13ae1c2f91cb7581c3a8`.

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

### `src/Orbis.Stream.Core/Services/ErrorReport.cs`

Tipi: `ErrorReport`.

Metodi pubblici: `Image`, `Parts`, `Text`.

SHA-256: `1707cf808fc6a919e2dca6c54a18487fbb6cec7a746e8119d9149e2fbf8b8ccc`.

### `src/Orbis.Stream.Core/Services/FfmpegVideoPlaylistStreamer.cs`

Tipi: `FfmpegVideoPlaylistStreamer`, `StreamOutcome`, `ReconnectBudget`, `PlannedVideo`, `PausedProgram`, `VideoGroup`, `IVideoPlaylistStreamer`.

Proprietà: `Used`, `StopReason`.

Metodi pubblici: `StreamPlaylistAsync`.

SHA-256: `8723f585dfd4826d03c27b533e2494afde18991dd9540c84ffa397936f50c06a`.

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

SHA-256: `e466d6528540056316a0d2f3d2e87b6c0314b0b7149268ae203ace5704418903`.

### `src/Orbis.Stream.Core/Services/LivePreviewService.cs`

Tipi: `LiveMedia`, `LiveParameters`, `LiveSource`, `LiveOption`, `LiveSnapshot`, `LivePreviewService`.

Metodi pubblici: `ApplyParameters`, `ApplyVolume`, `EmbedAsync`, `Offline`, `Snapshot`.

SHA-256: `e395a8fafd55c58aa4b2e077ff4756844db42c443a8528d2bfdb72cecede7209`.

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

SHA-256: `d7036800eb60223fc0d92abf191f857e71dcaa57ca0f55d1b23a1936f9c162e6`.

### `src/Orbis.Stream.Core/Services/SystemInfoService.cs`

Tipi: `SystemInfoService`.

Metodi pubblici: `GetAllSystemInfo`, `GetAppCpuPercent`, `GetAppDiskPercent`, `GetAppGpuPercent`, `GetAppNetworkPercent`, `GetAppRamPercent`, `GetCpuPercent`, `GetCpuTemperature`, `GetFfmpegProcessStats`, `GetGpuTemperature`, `GetRamPercent`, `GetSwapPercent`, `GetSystemFacts`.

SHA-256: `c88c3c1cac5f4747a3dc82b6494071de01d607cccbabd2f62d827d92d9cfe3c5`.

### `src/Orbis.Stream.Core/Services/VideoService.cs`

Tipi: `VideoService`.

Metodi pubblici: `DeleteLiveHistory`, `DeleteOldLiveHistory`, `DeletePlaylist`, `DeleteVideo`, `FindVideo`, `GetAllChannelOnline`, `GetAllVideoList`, `GetLivePage`, `GetLockedVideos`, `GetPlaylist`, `GetVideoChannelNames`, `GetVideoList`, `UnlockVideo`.

SHA-256: `39fa78e88985073b9c6247ac717fa3326d89c01f8d3c557923112d09267545f9`.

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

SHA-256: `263611b9e5abe5c14fe388f6040ad7bc91b3e4ac28dd47ae30a6e2800dd30549`.

### `src/Orbis.Stream.Core/Streaming/FfmpegProbe.cs`

Tipi: `MediaProbeResult`, `MediaOutput`, `FfmpegProbe`, `FfmpegToolLocator`.

Metodi pubblici: `CreateStartInfo`, `Parse`, `ProbeAsync`, `RunCapturedAsync`.

SHA-256: `acb2bc49ff0fa6801e36f4650c9f69b70d17ed8f084b56b758b8b805a16fa53d`.

### `src/Orbis.Stream.Core/Streaming/FfmpegProcessLog.cs`

Tipi: `FfmpegProcessLog`.

Metodi pubblici: `ReadAsync`.

SHA-256: `639c0b2062935a5c7adc12ee33b7b987f52e460526cb03dfc8fbf98e8629e5d3`.

### `src/Orbis.Stream.Core/Streaming/FfmpegStreamingSession.cs`

Tipi: `FfmpegStreamingSession`, `StreamingSessionRegistry`.

Proprietà: `Profile`, `OnAirFor`, `AdaptiveQuality`, `NominalBitrate`, `AudioBitrate`, `AdaptsBitrate`, `BehindFor`, `VideoPkid`, `InputPath`, `Probe`, `Output`, `StartedAt`, `StopReason`.

Metodi pubblici: `AbortAsync`, `AdaptBitrate`, `DisposeAsync`, `EndNaturallyAsync`, `HandOverAsync`, `ReadErrorAsync`, `RedactStreamKey`, `Register`, `Remove`, `RequestRestart`, `Start`, `StartComposition`, `StopAllAsync`, `StopAsync`, `TryGet`, `WaitForExitAsync`, `Watched`.

SHA-256: `27a58730fabc9f757adb5335e0bb1505e9105157f58bd9dbd42381e2c1f67c2d`.

### `src/Orbis.Stream.Core/Streaming/FlvPacedRelay.cs`

Tipi: `FlvPacedRelay`, `struct`, `RelaySegment`.

Proprietà: `IsOpen`.

Metodi pubblici: `Attach`, `Cancel`, `Close`, `Detach`, `Sample`, `Start`.

SHA-256: `fb7cb62aa3675df5687e708b88a8287b54cb3db37b8b7b6b1a58514fa120650f`.

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

Proprietà: `OutputUrl`, `SenderHasExited`, `Frame`, `Profile`, `QualityCap`, `Ladder`.

Metodi pubblici: `Abort`, `AdaptBitrate`, `Attach`, `Cap`, `CompleteAsync`, `DisposeAsync`, `For`, `Pin`, `ReadSenderErrorAsync`, `TryLower`.

SHA-256: `fe8328cf3947d333dccc8e311967d0a5819853d708346a03a267ac50af777a41`.

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

### `src/Orbis.Stream.Core/Streaming/NvencOptions.cs`

Tipi: `NvencOptions`.

SHA-256: `1db4ee9be4335ad9a03011d502307c6c638612e8a9f059a9344e72ecefb5604c`.

### `src/Orbis.Stream.Core/Streaming/OverlayMedia.cs`

Tipi: `OverlayMedia`.

Metodi pubblici: `Of`.

SHA-256: `6f43024675d5daeabd5e8107dcbd47856e40a626b0544ae4f4baaa672ecd4b42`.

### `src/Orbis.Stream.Core/Streaming/Rtmp/Amf0.cs`

Tipi: `Amf0Writer`, `Amf0Reader`.

Metodi pubblici: `Boolean`, `Null`, `Number`, `Object`, `ReadAll`, `String`, `ToArray`.

SHA-256: `e8e9217a19453e174752f73bd50562dc4eb9b0554b5d6e7b285b19f5e146bc50`.

### `src/Orbis.Stream.Core/Streaming/Rtmp/RtmpPublisher.cs`

Tipi: `RtmpException`, `RtmpPublisher`, `InboundChannel`.

Metodi pubblici: `Abort`, `Complete`, `Dispose`, `Open`, `WriteHeader`, `WriteTag`.

SHA-256: `c00dd2455ef6840ad36d9bdf11c798c4719eeb8262b9b68c0170ac12ff11d5bc`.

### `src/Orbis.Stream.Core/Streaming/StreamPlatform.cs`

Tipi: `StreamPlatform`, `RelayTransport`, `RelayPacing`, `StreamPlatformProfile`, `and`, `StreamPlatforms`.

Metodi pubblici: `Detect`, `For`, `NormalizeIngestUrl`, `Orient`, `WithTransport`.

SHA-256: `b0f3024c88144f938434928c322f0a4dd3879a89d13608e8c52caa87705cb32c`.

### `src/Orbis.Stream.Core/Streaming/WebpOverlayInputs.cs`

Tipi: `WebpOverlayInputs`.

Proprietà: `Items`.

Metodi pubblici: `Dispose`, `Prepare`.

SHA-256: `6a6474308c9255ea59a812bb6d86ffc6683ceeab2ee5ffaf3f2768bf30e0d8c9`.

### `src/Orbis.Stream.Core/SystemInfo/AppInfo.cs`

Tipi: `AppInfo`.

SHA-256: `25db9d2ee8f32eb308258389dc9e3dd531c0773ee861d8886a92fc0d761ccb9b`.

### `src/Orbis.Stream.Core/SystemInfo/SystemInfoProviders.cs`

Tipi: `SystemFact`, `FfmpegProcessStat`, `ISystemInfoProvider`, `SystemInfoProviderFactory`, `AppMetricsHelper`, `TemperatureCache`, `Reading`, `WindowsSystemInfoProvider`, `at`, `in`, `that`, `carries`, `PortableSystemInfoProvider`.

Metodi pubblici: `CreateDefault`, `FormatBytes`, `FormatUptime`, `GetAppCpuPercent`, `GetAppDiskPercent`, `GetAppGpuPercent`, `GetAppNetworkMbps`, `GetAppNetworkPercent`, `GetAppRamPercent`, `GetCpuPercent`, `GetCpuTemperature`, `GetFacts`, `GetFfmpegProcessStats`, `GetGpuTemperature`, `GetRamPercent`, `GetSwapPercent`, `IsUsableAt`.

SHA-256: `0c4e8ab6c32da392a5f5c9e5c4f2c6d9d239903099794a710f1116d1d4e0ece0`.

### `src/Orbis.Stream.Core/Ui/manual.ar.json`

SHA-256: `5d7ca8f057e0cb97f1c8b706914659131167d173cd81c9ffb2ee54ca45d6a6c6`.

### `src/Orbis.Stream.Core/Ui/manual.da.json`

SHA-256: `ea3b90448aaac986921e49a2fab88a7e2c39268d29cf4bc43b994ea394e4966e`.

### `src/Orbis.Stream.Core/Ui/manual.de.json`

SHA-256: `b092b1e9c9e95b408bc6110735b24bdf16968569a77df019c9dc964eb3526e38`.

### `src/Orbis.Stream.Core/Ui/manual.el.json`

SHA-256: `5a8df4f23d0b0df4e7f6f2f52561d5a2ead52d99d050a5533092eac899ab34ea`.

### `src/Orbis.Stream.Core/Ui/manual.en.json`

SHA-256: `928d18add074e8a813867d75baab7fe0f90b1131e68b065b75a2041d6d64af8c`.

### `src/Orbis.Stream.Core/Ui/manual.es.json`

SHA-256: `8e0a1bf4ef4cc1990edc64c25e3b7bdbc5677ddf8fa09f505fdcb786db1d3091`.

### `src/Orbis.Stream.Core/Ui/manual.fil.json`

SHA-256: `2efeee8d5b9d6127501193be3ae27539aa57d1f6b9fb7114139167006e1c26ee`.

### `src/Orbis.Stream.Core/Ui/manual.fr.json`

SHA-256: `f327270b404f0f830db0f7292ea0c004bf3e601ac3bef638bfc2a31f82fdc1f6`.

### `src/Orbis.Stream.Core/Ui/manual.hi.json`

SHA-256: `7586a040cd726dc33461d038db4cdd2fd786d1650e799da304430e45cc4d7c35`.

### `src/Orbis.Stream.Core/Ui/manual.hod.json`

SHA-256: `c781cc9e18405d08aca7b96aa1c331ca2b130a6aecd7eb6e69fabdfb62becc1f`.

### `src/Orbis.Stream.Core/Ui/manual.it.json`

SHA-256: `1fc5e8576e4b03f9446e30322e463711e1dd41c2f7a3eafbd134928ac09493d0`.

### `src/Orbis.Stream.Core/Ui/manual.ja.json`

SHA-256: `0ae529d9057f966a387284305d2e2f4973ee3b4f7f9ab4367c00555c96e9f03d`.

### `src/Orbis.Stream.Core/Ui/manual.ko.json`

SHA-256: `7696137ef0e57d30120271e7b7957ecf1c54296717d3ea28f002295ee692ad10`.

### `src/Orbis.Stream.Core/Ui/manual.la.json`

SHA-256: `14b8b3d1802f6c813e8e5cd1a91c43fe4a57b1262a71aeb5502a6c9cbf73d396`.

### `src/Orbis.Stream.Core/Ui/manual.nl.json`

SHA-256: `cd83973bd7a426d45ae83c710fe099e0f64ccbeabac678fd5e13593d5775342a`.

### `src/Orbis.Stream.Core/Ui/manual.pt.json`

SHA-256: `db25d894db914dd00fa459774762cfaae4d03286a081dfb65267362d93eefa48`.

### `src/Orbis.Stream.Core/Ui/manual.ro.json`

SHA-256: `76e37c08bd521688821c9d1606bde84934a1609a80268cae0153d603b1ed5470`.

### `src/Orbis.Stream.Core/Ui/manual.ru.json`

SHA-256: `6cc400828b1d0e5a61c79c23833a5eedda9d627026b3ebd9a5349d6c51b31b72`.

### `src/Orbis.Stream.Core/Ui/manual.tlh.json`

SHA-256: `d8906802baa5254fb83cf4dbf92d26fc2927942b8eab760136e4a406f2827109`.

### `src/Orbis.Stream.Core/Ui/manual.tr.json`

SHA-256: `3d5a71bc98d11dbd05a14bb2cb555e89d3aa77550cf5191cc9f27dac6d16813b`.

### `src/Orbis.Stream.Core/Ui/manual.zh.json`

SHA-256: `d98cd5498437b47ccefdf5e171a1f14163f94efe49a4059bc7c89073ff936800`.

### `src/Orbis.Stream.Core/Ui/ui.ar.json`

SHA-256: `848b3be5216f826c09e82800f092861aeee49f087e60d35011fbbe26c656c892`.

### `src/Orbis.Stream.Core/Ui/ui.da.json`

SHA-256: `7228ecc25869faa64e32b12600f931e4a65592a4aeca7c17f7b91175a0fd15c0`.

### `src/Orbis.Stream.Core/Ui/ui.de.json`

SHA-256: `dda04cba3ceeb8df8d1698cdd69e82e11e33102c05bcebb7c9e0db36fab16a41`.

### `src/Orbis.Stream.Core/Ui/ui.el.json`

SHA-256: `7e804d778caa936f4c65d1097dfa7b925c0f81982c3383a9c37cf7330710e367`.

### `src/Orbis.Stream.Core/Ui/ui.en.json`

SHA-256: `c11eedaeaf001b56872a0d2c9f7a5321ce95e1f3cc6b82f70eb7625e50c020a1`.

### `src/Orbis.Stream.Core/Ui/ui.es.json`

SHA-256: `6294018f164643f668d0e45bee3165112921fc59808140e64a109ae219211464`.

### `src/Orbis.Stream.Core/Ui/ui.fil.json`

SHA-256: `af148299b1a42fa5ae6aa7dfbdac66fa33e4ee2286da3ba29eeedeaab4ec625d`.

### `src/Orbis.Stream.Core/Ui/ui.fr.json`

SHA-256: `3cda01d235024956ead69ef69e27b535fcba2a948e96b6ea8c83d3e5a5ea4f8a`.

### `src/Orbis.Stream.Core/Ui/ui.hi.json`

SHA-256: `9dedf4ab7e03a466eb5a384a26c68e757c3d06c91c9655d3461c2a32077590f3`.

### `src/Orbis.Stream.Core/Ui/ui.hod.json`

SHA-256: `07b3c3036d9b69417ee9d62c4104e938975aa5f404b6f40b1d6d9347e1aaf602`.

### `src/Orbis.Stream.Core/Ui/ui.it.json`

SHA-256: `f72273b642b4a3abf86a4cb1e24d62f0cc2f7332c6d2571a6e43b764ccb0b777`.

### `src/Orbis.Stream.Core/Ui/ui.ja.json`

SHA-256: `1907e33ba9249a47fa204449a3a9dfd377ed0df3558aeb72c50941e4e2076a58`.

### `src/Orbis.Stream.Core/Ui/ui.ko.json`

SHA-256: `6e47983814d8db5e73c3ef2f684aa616cac42a8d28749401509ddc49b510820f`.

### `src/Orbis.Stream.Core/Ui/ui.la.json`

SHA-256: `fce05409d0b96d56c6fa787880e83b86b457faa5158f934abb75d07a7a873ad7`.

### `src/Orbis.Stream.Core/Ui/ui.nl.json`

SHA-256: `a71a19fa40b9cca729f928c4b6ff2dc30dc2341619227746fb03dd9dac92424c`.

### `src/Orbis.Stream.Core/Ui/ui.pt.json`

SHA-256: `6c2eec42f641b151df126f47caa5adf16f4b77c187e4238df0205160d458ab28`.

### `src/Orbis.Stream.Core/Ui/ui.ro.json`

SHA-256: `70417c6fc3f47603d73b98dfce913d8c643d19b70d9b2e6c2a8d3c8711ce5687`.

### `src/Orbis.Stream.Core/Ui/ui.ru.json`

SHA-256: `e0287f885cd3a86932f3f6169e44b83c0e3199478453760f0b2b7fc718da17bb`.

### `src/Orbis.Stream.Core/Ui/ui.tlh.json`

SHA-256: `acf80379122c02c10458db365c5818ac0accc51ada26bfa441e924bb59cf0215`.

### `src/Orbis.Stream.Core/Ui/ui.tr.json`

SHA-256: `9c4ba8f5a1cb4abee13b5335c6e1b16f4d31618328998a969907b4de848801e3`.

### `src/Orbis.Stream.Core/Ui/ui.zh.json`

SHA-256: `2fe8d533d8c494e9439327e6d5832ee62dae82e798f5e1bf41571864391a065d`.

### `src/Orbis.Stream.Tests/AdaptiveBitrateSwitchTests.cs`

Tipi: `AdaptiveBitrateSwitchTests`.

Metodi pubblici: `OffTheLiveGoesOutAtTheBitrateOfTheSettingWhateverTheLadderDecided`, `OnlyAnExplicitNoTurnsItOff`, `SetReplacesWhatTheSettingSaidBefore`, `TheFormStartsOnAndCarriesTheSwitchBothWays`, `TheSwitchIsADecisionOfThisApplicationNotAnOptionOfFfmpeg`.

SHA-256: `9c394fce1a0a012a64fa952ba3a708d3b786a2adc97c715fdbf022438d827a41`.

### `src/Orbis.Stream.Tests/AnimatedWebpTests.cs`

Tipi: `AnimatedWebpTests`.

Metodi pubblici: `Ffmpeg9DecodesAndLoopsAnimatedWebpWithTransparencyOnTheCanvas`.

SHA-256: `56432b4ce11b27b2681177f4cb5a8383b374481f0b28e020ca1e187cb05c8052`.

### `src/Orbis.Stream.Tests/BitrateLadderTests.cs`

Tipi: `BitrateLadderTests`, `Relay`.

Proprietà: `Nominal`, `Elapsed`.

Metodi pubblici: `ADipTheRelayWinsBackIsNoReasonToSwitch`, `ALadderWithoutRulesOrASettingWithoutBitrateNeverMoves`, `ALiveTheNetworkCarriesKeepsTheBitrateOfTheSetting`, `ANetworkThatCarriesLessThanTheLiveBringsTheBitrateDownToWhatItCarries`, `ANetworkThatStoppedIsAStallNotABitrate`, `ANewConnectionKeepsTheBitrateTheNetworkLeftTheLiveAt`, `ASettingBroughtUnderTheCapIsTheBitrateOfTheLive`, `ASlowLeakIsCaughtOnceTheLiveHasSlippedTheMaxDrift`, `AStepUpTakenBackDoublesTheWaitBeforeTheNext`, `AnEncoderThatDoesNotKeepUpIsNotTheNetworksFault`, `OneStepNeverMoreThanHalvesTheBitrateAndTheFloorHoldsIt`, `Reconnect`, `Run`, `TheLiveClimbsBackInStepsOnceItWentOutClean`, `YouTubeRidesOutWhatTwitchAnswers`.

SHA-256: `394d5c7ff3d4d47594b32acf6e1c48e31d37357c3db3a746083bd090a3f90a22`.

### `src/Orbis.Stream.Tests/DatabaseTests.cs`

Tipi: `TemporaryDatabase`, `DatabaseSchemaTests`, `SceneRepositoryTests`, `VideoRepositoryTests`, `SettingRepositoryTests`, `LivePagePlatformTests`.

Proprietà: `LiveHistoryId`, `Directory`, `DatabasePath`.

Metodi pubblici: `Delete_RemovesTheRow`, `Delete_TakesTheItemsWithIt`, `Dispose`, `EnsureCreated_IsIdempotent`, `EnsureCreated_KeepsExistingData`, `EnsureCreated_MigratesAPreviousDatabase`, `EnsureCreated_MovesTheOldYouTubeIngest`, `EnsureCreated_TurnsTheOldScenesIntoLayouts`, `FindAll_ReturnsTheItemsInStackingOrder`, `FindAll_SupportsDynamicFilters`, `FindByLiveStatusAndChannelName_CombinesBothConditions`, `FindChannelNames_ListsEveryChannelOfTheVideosOnceAndInOrder`, `FindLayouts_LeavesTheScenesOfTheLivesOut`, `FindPaged_AppliesFiltersSortingAndPaging`, `FindPaged_CombinesTheChannelTheStatusAndTheSearch`, `FindPaged_KeepsOnlyTheRowsWhoseContainerIsAVideo`, `FindPaged_LooksTheWildcardsOfASearchForThemselves`, `FindPaged_SearchesTheTitleAsAPartOfItAndNotAsAllOfIt`, `FindPaged_SupportsTheVideoLiveHistoryJoin`, `FindPaged_WithoutTheWhitelistStillAnswerEveryRow`, `InsertAndFind_KeepsTheTileAndTheResolutionOfTheFile`, `InsertAndFind_RoundTripsEveryColumn`, `Insert_GeneratesTheNextIdentifier`, `Insert_ThenUpdate_ReplacesTheItemsInOrder`, `PendingPublishIsActiveButDoesNotAppearInTheLiveFilter`, `SetStopFlag_UpdatesThePollingFlagOfTheStreamer`, `SetVideoSetting_UpdatesOnlyTheConfigurationReference`, `SetVolume_SurvivesTheStreamerWritingTheRowItHolds`, `StartAsync_ReleasesTheLivesLeftRunning`, `TheLivePageListsTheLivesOfOnePlatform`, `Update_ReplacesEveryField`.

SHA-256: `fb05441e02c00efe9da810bf969d8fd9c8f2e91684ca74ba10946ee08fb0b615`.

### `src/Orbis.Stream.Tests/DomainAndPagingTests.cs`

Tipi: `LiveStatusTests`, `PageRequestTests`, `FilterValueConverterTests`, `AutoCleanupServiceTests`, `DatabaseBootstrapperTests`, `FfmpegCodecCatalogTests`, `values`, `UserPathTests`, `LiveLinkTests`, `SystemFactTests`, `TemperatureTests`, `LiveChangeNotifierTests`.

Metodi pubblici: `AChangeThatArrivesBetweenTheTestAndTheWaitIsNotLost`, `AListenerResumesFromTheChangeItAlreadyHas`, `AListenerThatArrivesLateIsAnsweredAtOnce`, `AListenerThatGoesAwayDoesNotHoldTheOnesThatStay`, `AListenerWaitsForAChangeAndIsToldWhichOneItIs`, `AMachineWithoutSensorsIsNotAskedAgainEverySecond`, `AudioCodecs_FollowTheFfmpegEnum`, `ChoiceOfSetting_ReadsThePlatformADefaultSettingWasSeededFor`, `ChoiceOf_ReadsTheNameThenTheIngestOfAConfiguration`, `Convert_ParsesTheSupportedKinds`, `Convert_ThrowsTheOrbisQueryExceptionOfTheDynamicSpecification`, `DelayAsync_TakesAnIntervalOfMonthsWithoutStoppingTheApplication`, `EachPlatformHasItsOwnMarkAndItsOwnColour`, `EveryChangeHasItsOwnNumber`, `Facts_DescribeTheMachineThatRunsThem`, `Facts_NeverCarryAnEmptyValue`, `FormatBytes_UsesTheBinaryDivisorAndNoLocale`, `FormatUptime_IsReadableWithoutAnyLanguage`, `FormatUptime_OfAClockThatWentBackwardsIsZero`, `FromStorage_AlsoReadsTheWireNameAndTheOrdinalAsText`, `FromStorage_FallsBackToOfflineForUnknownValues`, `FromStorage_ReadsTheHibernateOrdinal`, `KickAndFacebookGamingHaveTheirNamesMarksAndColours`, `LabelAndMark_AreTheOnesOfTheConfigurationForm`, `ManyListenersAreAllWokenByTheSameChange`, `NormalizeUserPath_StripsCopyAsPathQuotes`, `Ordinals_MatchTheHibernateColumnOfTheJavaEntity`, `Parse_ClampsThePagingParameters`, `Parse_ReadsTheSpringSortSyntax`, `Parse_UsesTheDefaultPageSizeAndSort`, `PixelFormats_FollowTheFfmpegEnum`, `PlatformOf_HasNoAnswerForAnIngestItDoesNotKnow`, `PlatformOf_KnowsKickAndFacebookByTheDomainOfTheirIngest`, `PlatformOf_ReadsTheIngestOfTheLive`, `PlatformOf_StillWantsThePresetForTheIngestsThatAreTheSameForEverybody`, `Schema_TurnsTheFfmpegDeliveryOnForYouTubeOnceAndOffForTwitch`, `SpringPageMetadata_IsConsistentWithTheContent`, `StartAsync_AddsTheNewPlatformsOnceToAnInstallationThatHadTheOldOnes`, `StartAsync_CreatesOneDefaultConfigurationPerPlatform`, `StartAsync_KeepsASingleDefaultConfigurationPerPlatform`, `StartAsync_MovesTheLegacyYouTubeDefaultsButNotOnesTheUserEdited`, `StartAsync_RenamesThePlaceholderDefaultInsteadOfAddingAnother`, `StartAsync_SeedsKickAndFacebookGamingWithTheirOwnDefaults`, `StartAsync_SeedsTikTokStandingUp`, `StartAsync_SeedsYouTubeAtAFixedPictureWithNothingThatReplacesTheConstantRate`, `Temperatures_AnswerAPlausibleReadingOrNoneAtAll`, `Temperatures_AreReadOnceForAsManyAsksAsThereAre`, `TheChannelSettingsOfferTheFivePlatformsWithTheirIngest`, `TheLowDefaultsAreShownInTheLanguageOfThePage`, `UrlOf_EscapesWhatWouldBreakTheAddress`, `UrlOf_FallsBackToTheNameOfTheApiWhenTheChannelIsNotThere`, `UrlOf_HasNoLinkWithoutAPlatformOrAChannel`, `UrlOf_KickIsTheChannelAndFacebookGamingThePage`, `UrlOf_TakesTheChannelOfTheConfigurationNotThePlatform`, `UrlOf_TikTokIsTheLiveOfTheAccount`, `UrlOf_TwitchIsTheChannelPage`, `UrlOf_YouTubeBeforeTheBroadcastIsSeenIsTheControlRoomOfTheChannel`, `UrlOf_YouTubeIsTheLiveControlRoomOfTheBroadcastOnAir`, `VideoCodecName_WinsOverTheNumericCodec`, `VideoExtensions_AcceptTheSupportedContainers`, `YouTubeChannelUrl_IsThePageOfTheHandleOrOfTheId`.

SHA-256: `9910787dc860d9ca2c92b566c802c60a0fae155efaec7742e5718be5279c62ee`.

### `src/Orbis.Stream.Tests/EncoderQualityTests.cs`

Tipi: `EncoderQualityTests`.

Metodi pubblici: `ALevelChosenByTheUserIsNeverMeasured`, `APresetTypedByHandWinsOverTheLevel`, `Auto_FallsBackToTheLightLevelOnAGpuThatDoesNotKeepUp`, `Auto_GetsTheBalancedLevelOnAMachineThatKeepsUp`, `EveryEncoderSpellsTheLevelItsOwnWay`, `LegacyNvencPresetsStillStartWithFfmpeg9`, `NvencCompatibilityLeavesOtherEncodersAndModernPresetsAlone`, `RemovedNvencFlagsAreConvertedAndModernOverridesWin`, `RemovedNvencRateControlsUseTheModernModeAndMultipass`, `TheDecisionsOfTheApplicationNeverReachFfmpeg`, `TheDefaultsAreLeftAloneWhenFfmpegDoesNotRun`, `TheDefaultsGoToTheGpuAndLeaveTheSettingsOfTheUserAlone`, `TheFormKeepsTheLevel`, `TheMeasuredLevelOverridesTheAutomaticOne`, `WithoutLowLatencyThePresetAndTheTuneAreRequired`.

SHA-256: `91b146afd077331475b93eb8babcea94016da80296a9605da6c725006daca8b4`.

### `src/Orbis.Stream.Tests/EndToEndApiTests.cs`

Tipi: `TestHostRunner`, `ApplicationFixture`, `EndToEndApiTests`, `VideoSnapshot`.

Proprietà: `DataDirectory`, `Port`, `WebRootPath`, `Client`, `NoRedirectClient`.

Metodi pubblici: `ALayout_KeepsOnlyItsSlots_AndTheScenesOfTheLivesAreNotListed`, `ANewScene_KeepsItsSourcesAndAnswersWithItsId`, `DisposeAsync`, `EveryPageSaysWhichBuildThisIsAndWhoWroteIt`, `FfmpegSenderSwitch_IsOnForANewConfiguration`, `FfmpegSenderSwitch_IsSavedAndSurvivesTheActiveToggle`, `GetAllVideo_IgnoresThePagingSortAndLanguageParameters`, `GetAllVideo_RejectsAnUnknownFilterProperty`, `GetPage_ReturnsTheSpringPageEnvelope`, `InitializeAsync`, `LanguageQueryParameterIsNotAFilter`, `LanguageSelector_NeverRedirectsOutsideTheApplication`, `LanguageSelector_StoresTheChoiceAndTranslatesThePages`, `LinkingASetting_EditsTheOneOfTheRow_AndCopiesASharedOneOnce`, `LowLatencySwitch_SurvivesASaveAndReadsBackAsItWasLeft`, `MainLive_RowFormsCarryTheVideoKeyInTheirAction`, `MainLive_StartLiveOpensWithUrlParameter`, `Pages_RenderOnTheServer`, `Root_RedirectsToTheClientRouter`, `Settings_AreCreatedListedAndDeleted`, `Start`, `StartLive_ValidatesTheRequestAndRecordsTheFailureOfTheStreamer`, `StaticFiles_AreAlwaysRevalidated`, `Swagger_IsExposedUnderTheDocumentedPaths`, `SystemInformation_IsServedAsPlainNumbers`, `TaskManager_PushesTheMetersInsteadOfBeingAskedForThem`, `TaskManager_ShowsTheMetersStraightAwayAndTheFactsOnTheirOwn`, `Temperatures_AreServedAsPlainNumbersOrAsNothingAtAll`, `TheDashboardAsksForAWeaselWithABanner`, `TheDefaultSettingsAreNamedAfterTheirPlatform`, `TheLiveHistoryPagesLikeTheLiveGridDoes`, `TheLiveHistory_LeavesTheImagesAndOtherOverlaysOut`, `UnknownPaths_AreNotFound`, `Updates_SaysWhenARowMovedAndOnlyWhenSomeoneIsListening`, `Updates_SendsRowsOnlyWhenARowReallyMoved`, `VideoSettings_ExposeTheDefaultPlatformConfigurations`.

SHA-256: `be497cd7572edf730eb93ed0774858938d9e7baf30b5efc0c4b984e58f6fd757`.

### `src/Orbis.Stream.Tests/ErrorReportTests.cs`

Tipi: `ErrorReportTests`.

Metodi pubblici: `FailedLiveHasReportButtonsAndALocalQrReportWithoutItsStreamKey`, `LongUnicodeErrorsRemainCompleteAcrossQrPages`.

SHA-256: `a0f8ab0728de2469a50fe444565be853d8d4773478447c2eed29b25223b5f015`.

### `src/Orbis.Stream.Tests/FfmpegCommandBuilderTests.cs`

Tipi: `FfmpegCommandBuilderTests`, `FfmpegCompositionTests`, `SourceCatalogTests`.

Metodi pubblici: `ACameraOpenedForItsPictureCarriesNoSound`, `ACanvasIsNotBoundedWhileThereIsSomethingThatNeverEnds`, `ACanvasOfFilesBehindARelayIsPacedByTheRelayAlone`, `ACanvasOfFilesIsBoundedByItsLongestFile`, `ACanvasOfFilesWithoutARelayIsPacedOnceOnWhatComesOut`, `ACanvasStartedAgainFromWhereItStoppedStillLastsAsLong`, `ACanvasWithADeviceKeepsTheFilesInStepWithIt`, `ACompositionWithNoPictureIsRejected`, `ACompositionWithNoSizeIsRejected`, `ADeviceGetsARealTimeBufferLargerThanAFrame`, `AFileIsPacedAndSeeked`, `AMicrophoneIsHeardAndNotSeen`, `AMonitorIsTheDesktopCroppedToItsRectangle`, `AScreenCarriesNoSound`, `ASnapshotIsNotPaced`, `ATileWithNoRectangleIsRejected`, `AWebcamWithItsMicrophoneIsOneInputAndIsHeard`, `BuildStreamingUrl_JoinsWithASingleSeparator`, `Build_ComputesGopAsFrameRateTimesGopSize`, `Build_LeavesTheResolutionOfTheSourceAloneWhenTheSettingHasNone`, `Build_PassesTheInputPathAsOneVerbatimArgument`, `Build_RoundsTheScaleDownToAnEvenFrameSize`, `Build_ScalesToTheResolutionOfTheSetting`, `Build_SkipsAudioFlagsWhenTheInputHasNoAudioTrack`, `Build_TakesTheFrameRateOfTheSettingOverTheOneOfTheSource`, `Build_UsesTheVideoSettingAsEncoderConfiguration`, `Build_WithoutAPresetEncodesClean`, `Desktop_WebcamAndMicrophone_ComposeThroughOneGraph`, `DeviceList_OfFfmpeg4_IsReadBySection_AndHandlesColons`, `DeviceList_OfFfmpeg5_NamesItsOwnKind_AndHandlesColons`, `EveryInputIsOpenedBeforeTheGraph`, `EverySourceIsMixedAtItsOwnVolume`, `EveryTileStartsItsClockAtZero`, `MonitorTarget_RoundTrips`, `ResolveOutput_AnswersWhatTheCommandLineAsksFor`, `ResolveOutput_FallsBackToWhatTheSourceCarries`, `SettingFrameRate_WinsOverTheCanvasOne`, `SourcesAreStackedInTheOrderTheyWereDropped`, `TheCanvasIsABlankFrameOfTheSceneSize`, `TheEncoderIsTheSameOneAFileWouldUse`, `ThePreviewOfACanvasIsThirtySharpFramesASecond`, `ThePreviewOfAFileIsThirtySharpFramesASecond`, `TheStreamKeyNeverReachesAnErrorMessage`, `TileRectanglesAreRoundedDownToEvenPixels`, `TwoSoundsAreMixed`.

SHA-256: `f9cbfb3009f40b0b46704a69d18f0465b30ccd1c768e318cb62b480f2e901dc4`.

### `src/Orbis.Stream.Tests/FfmpegProcessLogTests.cs`

Tipi: `FfmpegProcessLogTests`, `CapturingLogger`.

Metodi pubblici: `AWarningIsLoggedBeforeTheRunningProcessClosesItsPipe`, `DiagnosticsRetainABoundedTailAndTheirSeverity`, `FacebookGamingIsGivenTheSameCodecProbeAsYouTube`, `FacebookGamingReportsItsEncoderAndSenderLikeYouTube`, `IsEnabled`, `SenderKeepsConnectionDebugButDropsPacketDebugAfterOpeningOutput`, `YouTubeEncoderAndSenderExposeCodecInformationAndWarnings`.

SHA-256: `9a5d9a6bc762aab8171641944176c8b66d1f5b86d10d88bf94d6aa8cc707792b`.

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

SHA-256: `ab6d424c820035d62342bc470d0d2217a05fc1aca6c66a786d150c76f3ed99e3`.

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

Metodi pubblici: `ACanvasOfFilesAndOverlaysIsPacedByTheRelayAlone`, `ACanvasStandsOnItsFirstSourceEvenWhenAnOverlayIsUnderIt`, `AStillIsDecodedOnceAndLaidWithItsAlpha`, `AStillIsOpenedOnceAndAnAnimationLoopsWithTheDecoderThatKeepsItsAlpha`, `AnAnimatedWebPFromFfmpeg9LoopsWithoutForcingTheStillDecoder`, `AnAnimationLoopsForAsLongAsTheLiveThroughTheDecoderItNeeds`, `AnOverlayIsNeverHeard`, `AnOverlayIsNeverSoughtIntoWhenTheLiveResumes`, `OnlyPicturesAndVideosWithAlphaJoinTheLibrary`, `TheLibraryShowsTheNameAFileWasAddedWith`, `ThePageReadsTheFormatTheCodecTheFramesAndTheAlphaOfAWebM`, `TheStillOfAnOverlayKeepsItsAlpha`, `TheStoredNameIsSafeOnAnyDiskAndInAnyUrl`, `WithADeviceOnTheCanvasAnAnimationIsReadAtItsOwnPace`.

SHA-256: `1c7299401d4e2d0f7b67fb18cf25d837b7024d5515ae0216fc8d0c6befac0e75`.

### `src/Orbis.Stream.Tests/OverlayLiveTests.cs`

Tipi: `OverlayLiveTests`.

Metodi pubblici: `ALayoutKeepsItsOverlaysWholeAndTheLibraryKeepsWhatALayoutUses`, `ALiveLaysItsOverlaysOverItsSourcesWithTheirAlpha`, `DisposeAsync`, `InitializeAsync`, `TheLibraryRefusesWhatIsNotAPicture`, `TheLibraryTakesAPictureOnceAndAnswersWithItByNameOnly`.

SHA-256: `bfe269cbc88afedec6660d50239d5c1142ea2028d72a143cdb83682daf4b437d`.

### `src/Orbis.Stream.Tests/PacedRelayTests.cs`

Tipi: `StreamPlatformTests`, `HybridWaiterTests`, `FlvPacedRelayTests`, `TimedStream`, `BrokenStream`, `RelayCommandTests`, `PacedRelayFfmpegTests`.

Proprietà: `Writes`, `StallOnWrite`, `Stall`.

Metodi pubblici: `Detect_ReadsThePlatformOffTheHost`, `Direct_WithoutAProfileTheCommandIsTheOneOfBefore`, `For_GivesEveryPlatformItsProfile`, `KickIsDeliveredTheWayTwitchIsAndFacebookGamingTheWayYouTubeIs`, `LiveOutput_LowersTheLevelOneStepAtATimeDownToTheLightest`, `LowLatencySwitch_KeepsTheEncoderDefaultsOnlyWhenItIsOn`, `LowLatencySwitch_ReachesTheHardwareEncoderToo`, `NormalizeIngestUrl_LeavesAnUnreadableUrlAlone`, `NormalizeIngestUrl_PutsYouTubeOnTheTlsItOnlyTakesIngestOver`, `NormalizeIngestUrl_UsesLiveProducerForLegacyFacebookConfigurations`, `Relay_AStreamThatMustCarrySoundMapsItAsMandatory`, `Relay_CountsWhatItSentForTheBitrateLadder`, `Relay_EncoderWritesFlvOnItsOutputAndLeavesThePacingToTheRelay`, `Relay_FailsWhenTheSenderIsGone`, `Relay_ForwardsEveryByteUnchanged`, `Relay_KeepsTheConnectionOpenBetweenEncoders`, `Relay_LaysEncodersEndToEndOnOneConnection`, `Relay_RefusesAStreamThatIsNotFlv`, `Relay_SendsEveryTagWhenItsTimestampIsDue`, `Relay_SendsFasterThanRealTimeWhileItIsBehind`, `Relay_SendsThePrerollAtOnce`, `Relay_SplicesTwoEncodersIntoOneContinuousStream`, `Relay_StartsTheClockOnAFullJitterBuffer`, `Relay_StreamsACanvasOfTwoFilesInRealTime`, `Relay_StreamsAFileInRealTimeAndIsOnAir`, `Relay_WinsTheGapBackInsteadOfWritingItOff`, `Relay_YouTubeGetsASilentTrackWhenTheFileHasNoSound`, `Relay_YouTubeSilenceFollowsTheLastSourceOfACanvas`, `Sender_CopiesThePacedStreamToTheIngestWithATimeout`, `ThePlatformsAdaptTheirBitrateAtTheirOwnPaceAndBoundTheirKeyframes`, `WaitUntil_HonoursCancellation`, `WaitUntil_ReturnsAtOnceForATargetInThePast`, `WaitUntil_WakesOnTimeAndNotBefore`.

SHA-256: `1b85e0e6c1e1bbc7e1eaeefb6701c4648a7080bed587d2b8ee555ffb689c7848`.

### `src/Orbis.Stream.Tests/PlaylistTests.cs`

Tipi: `NaturalFileNameComparerTests`, `PlaylistTests`, `where`.

Metodi pubblici: `ACanvasLastsAsLongAsItsLongestFile`, `DisposeAsync`, `EnqueueSpot_PausesCurrentVideo_PlaysSpot_AndResumesWhileRemainingLive`, `EnqueueSpot_StreamsSpotThroughAndResumesMainVideo`, `InitializeAsync`, `OrdersNumbersByValueAndIgnoresCase`, `StartPlaylist_RefusesAFileAndAMissingFolder`, `StartPlaylist_StreamsOnlyTheVideosInOrder_AndAStopHoldsForTheWholePlaylist`.

SHA-256: `0b796250ace8ccc1cabf5af3543088f6349ef2acbe03b69bc55b2ae107e5dcea`.

### `src/Orbis.Stream.Tests/RelayTransportTests.cs`

Tipi: `RelayTransportTests`.

Metodi pubblici: `ACustomIngestHasNoRelayWhateverTheChannelAsks`, `ATwitchSenderOnlyCopiesWhatTheRelayPaced`, `AYouTubeChannelWithTheSwitchOffGoesOutOnTheNativePublisherPacedByTheRelay`, `EveryLiveOnARelayHasTheBitrateLadderOfItsPlatform`, `FacebookGamingGoesOutThroughAnFfmpegSenderThatReadsAtRealTimeWithItsBurst`, `FacebookKeepsTheWholeKeyOutOfTheApplicationName`, `KickGoesOutOnTheNativePublisherPacedByTheRelayAndHasNoSwitch`, `OnlyYouTubeKeepsOneFormatForTheWholeConnection`, `TheYouTubeSenderReadsAtRealTimeWithThePrerollAsItsBurst`, `TwitchHasNoSwitch`, `TwitchKeepsTheRelayClockAndTheNativeRtmp`, `YouTubeGoesOutThroughAnFfmpegSenderThatKeepsTheTime`.

SHA-256: `962903d6f9c9ba9ce81a6b927816881c70e2e0d21de6248c2aaad6956a06de9f`.

### `src/Orbis.Stream.Tests/RtmpPublisherTests.cs`

Tipi: `Amf0Tests`, `RtmpPublisherUrlTests`, `RtmpPublisherFfmpegTests`, `OutputLogger`.

Metodi pubblici: `Amf0_ReadsBackWhatItWrites`, `Amf0_StopsAtWhatItCannotReadInsteadOfFailing`, `IsEnabled`, `Native_PublishesThePacedStreamToAnRtmpServer`, `Publisher_FailsWithinItsTimeoutWhenNobodyAnswers`, `Publisher_RefusesAUrlThatIsNotAnIngestWithAKey`.

SHA-256: `ed48799d08938e257cd91b3bc1b6304405a9c1e7a52a10e2c476f19678aa02de`.

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

SHA-256: `00afc1d9cc8de1c2686ea0979fc3dab5e64e621e9b867af75605ddd0618f5e26`.

### `src/Orbis.Stream.Tests/SceneSwitchLiveTests.cs`

Tipi: `needs`, `SceneSwitchLiveTests`.

Metodi pubblici: `A_canvas_hands_over_to_a_picture_and_comes_back`, `A_picture_stands_in_until_it_is_resumed_and_a_clip_goes_back_by_itself`, `DisposeAsync`, `InitializeAsync`.

SHA-256: `c7a22cf6f60e61e7d73d69918470e9e32004d2c3b2f38aa2cab21a66c2bf47dd`.

### `src/Orbis.Stream.Tests/SharedPublisherStateTests.cs`

Tipi: `SharedPublisherStateTests`.

Metodi pubblici: `BufferedFramesDoNotMakeABlockedPublishLive`, `PreviewTracksTheSenderUntilItExits`.

SHA-256: `30495b970bec9a3be58def75db72032c442c5edc17fa02cb332377f16f27d322`.

### `src/Orbis.Stream.Tests/StreamingPlatformPageTests.cs`

Tipi: `StreamingPlatformPageTests`.

Metodi pubblici: `Presets_of_a_retired_platform_are_hidden_and_cannot_prefill_the_editor`, `The_channel_settings_show_the_personal_ingest_help`, `The_wizard_lays_the_settings_out_in_columns_with_the_mark_of_their_platform`.

SHA-256: `7bd551f0465885be511afa7c3af64e856cfe3c467c3f4d9a87b9e7cbe7ee425f`.

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

SHA-256: `0ff2bc64666d6023462849e8ecaaea64749940561acafb78a9c7c476145b78e0`.

### `src/Orbis.Stream.Tests/UiTextTests.cs`

Tipi: `bundles`, `UiTextTests`.

Metodi pubblici: `EveryBundleDefinesTheSameKeysInTheSameOrderAsEnglish`, `EveryBundleIsReallyTranslated`, `EveryLanguageHasItsOwnFlagAndTheFileIsThere`, `NoBundlePrefixesItsStringsWithALanguageTag`, `TheSelectorOffersExactlyTheLanguagesTheMessagesAnswerIn`, `TranslationsKeepEveryParameterOfTheEnglishString`.

SHA-256: `0aa7ebc96b8071c9d9b6c343972837692eae16bb9ed675449357de1faa9994bb`.

### `src/Orbis.Stream.Tests/WebpOverlayInputsTests.cs`

Tipi: `WebpOverlayInputsTests`.

Metodi pubblici: `FiniteAnimationsGetOneOwnedCopyWithOnlyTheirRepeatCountChanged`, `InfiniteAnimationsNeedNoCopyAndMalformedInputsAreRefused`.

SHA-256: `943d47c9b9f86977a97e04534b46bf8397f521e40eee2c0020977cb7caabe12f`.

### `src/Orbis.Stream.Tests/YouTubeAirWatchTests.cs`

Tipi: `YouTubeAirWatchTests`.

Metodi pubblici: `AChannelYouTubeKeepsFailingToAnswerForIsSaidToBeUnverified`, `ALiveFoundAgainForgetsTheEndingsCounted`, `ALiveFoundOnAirClearsTheFlagAndStartsCountingAgain`, `ALiveIsFlaggedOnlyAfterEnoughAnswersInARowSayItIsNotOnAir`, `ALiveIsForgottenOnlyAfterSomePassesWithNothingRunning`, `ALiveIsStoppedOnlyAfterEnoughAnswersInARowSayTheBroadcastEnded`, `APlatformThatCouldNotBeAskedLeavesTheMissesAsTheyWere`, `AnAnswerThatIsNotAnEndStartsTheCountAgain`, `ThePageOfAVideoSaysEndedOnlyWhenItIsALiveThatIsNoLongerLive`, `TheVideoOnAirIsRememberedAndFollowsABroadcastYouTubeReplaced`, `TheWaitRunsOnceForTheWholeLiveNotForEveryVideoOfIt`, `TheWarningStaysUntilTheUserClosesItAndComesBackOnlyAfterTheLiveWasFound`.

SHA-256: `06ab0e3ae7d39159e3497fc2550477c288d24f02b06d706e0265f6eaddd71b64`.

### `src/Orbis.Stream.Tests/YouTubeDeliveryTests.cs`

Tipi: `YouTubeDeliveryTests`, `OutputLogger`.

Metodi pubblici: `IsEnabled`, `YouTubeSenderPublishesDecodableAudioAndVideoAcrossEncoderChanges`.

SHA-256: `fbf0df0191d0ce5a594131a78c69a3bcc4c40e0a9c7553d05f67d61d504625e8`.

### `src/Orbis.Stream.Tests/YouTubeProbeTests.cs`

Tipi: `YouTubeProbeTests`.

Metodi pubblici: `TheSenderRecognizes48KhzAacAfterALargeInitialVideoKeyframe`.

SHA-256: `e8bffd72b0edd6999df764d0cbad36aa2c3ba5ce1851862de6801311c374192f`.

