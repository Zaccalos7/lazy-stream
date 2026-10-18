import AppKit
import Combine
import OrbisCore

@MainActor final class AppModel: ObservableObject {
    @Published var workspace = Workspace()
    @Published var sessions: [LiveSession] = []
    @Published var metrics = SystemSnapshot()
    @Published var processMetrics: [Int32: ProcessSnapshot] = [:]
    private let processSampler = ProcessSampler()
    @Published var devices: [CaptureDevice] = []
    @Published var capabilities = FFmpegCapabilities()
    @Published var error: String?
    @Published var selectedSession: UUID?
    @Published var search = ""
    @Published var recordingHotkey = false
    @Published var deckPanelOpen = false
    @Published var page: Page = .dashboard
    @Published var startRequest: StartRequest?
    @Published var report: LiveHistory?
    @Published private(set) var navigation: [Page] = []
    @Published var cleanupProgress = CleanupProgress()
    @Published var checkingUpdates = false
    @Published var updateMessage = ""
    @Published var availableUpdate: MacRelease?
    private var checkpointTick = 0
    private var startingChannels: Set<UUID> = []
    let store: WorkspaceStore
    private let sampler = SystemSampler()
    private var sampling: Task<Void, Never>?
    private var monitor: Any?
    init() {
        do { store = try WorkspaceStore() }
        catch { fatalError("Application Support non accessibile: \(error.localizedDescription)") }
        do { workspace = try store.load() } catch { self.error = "Impossibile caricare i dati. Il file originale è conservato: \(error.localizedDescription)" }
        workspace.recoverInterruptedRuns()
        sampling = Task { [weak self] in
            while !Task.isCancelled {
                guard let self else { return }
                self.metrics = self.sampler.sample(volume: self.store.directory)
                self.processMetrics = self.processSampler.sample(pids: self.sessions.flatMap { $0.processDetails.map(\.pid) })
                self.cleanupIfNeeded()
                self.checkpointTick += 1
                if self.checkpointTick % 5 == 0 && self.sessions.contains(where: { $0.isRunning }) { self.checkpoint() }
                try? await Task.sleep(for: .seconds(1))
            }
        }
        Task { await MediaOptimization.shared.removeUnused() }
        if error == nil {
            let snapshot = workspace, storage = store
            Task.detached(priority: .utility) {
                let sceneMedia = (snapshot.scenes + snapshot.history.compactMap(\.scene)).flatMap { $0.items.map(\.target) }
                let kept = Set(snapshot.overlays + snapshot.deck.map(\.mediaPath) + sceneMedia)
                for collection in ["Overlays", "Deck"] { try? storage.sweepMedia(collection: collection, keeping: kept, olderThan: Date().addingTimeInterval(-86_400)) }
            }
        }
        refreshCapabilities()
        if UserDefaults.standard.object(forKey: "autoCheckUpdates") as? Bool ?? true { Task { await checkUpdates() } }
        monitor = NSEvent.addLocalMonitorForEvents(matching: .keyDown) { [weak self] event in
            guard let self, !self.recordingHotkey, NSApp.modalWindow == nil,
                  NSApp.keyWindow?.sheetParent == nil || self.deckPanelOpen,
                  !(NSApp.keyWindow?.firstResponder is NSTextView), let session = self.watched, session.isRunning else { return event }
            let key = DeckHotkey(MacHotkeys.value(event))
            guard let button = self.workspace.deck.first(where: { !$0.hotkey.isEmpty && DeckHotkey($0.hotkey) == key && key != nil }) else { return event }
            self.playDeck(button, session: session); return nil
        }
    }
    var watched: LiveSession? { sessions.first { $0.id == selectedSession } ?? sessions.first(where: { $0.isRunning }) ?? sessions.last }
    var knownChannels: [Channel] {
        var seen = Set<UUID>()
        return (workspace.channels + workspace.history.map(\.channel) + sessions.map(\.channel)).filter { seen.insert($0.id).inserted }
    }
    func open(_ destination: Page) {
        guard destination != page else { return }
        navigation.append(page); page = destination; search = ""
    }
    func back() {
        guard let previous = navigation.popLast() else { return }
        page = previous; search = ""
    }
    func showPreview(_ session: LiveSession) { selectedSession = session.id; open(.preview) }
    func replay(_ history: LiveHistory, resume: Bool, startingAt videoID: UUID? = nil) {
        guard !sessions.contains(where: { $0.channel.id == history.channel.id && $0.isRunning }) else {
            error = "Questo canale è già in trasmissione."; return
        }
        let checkpoint = workspace.interruptedRuns.first { $0.historyID == history.id }
        if let checkpoint { unlock(checkpoint, restart: false) }
        guard !workspace.interruptedRuns.contains(where: { $0.historyID == history.id }) else { return }
        var channel = workspace.channels.first(where: { $0.id == history.channel.id }) ?? history.channel
        channel.sceneID = nil
        let playlist = videoID.map { LiveReplay.starting(at: $0, in: history, resume: resume) } ?? LiveReplay.playlist(history, resume: resume, checkpoint: checkpoint)
        start(channel: channel, profile: history.profile, scene: videoID == nil ? history.scene : nil, playlist: playlist,
              scenePosition: resume && videoID == nil && history.scene != nil ? (checkpoint?.position ?? history.scenePosition ?? 0) : nil)
    }
    func removeHistory(_ id: UUID) {
        perform {
            guard !sessions.contains(where: { $0.historyID == id && $0.isRunning }) else {
                throw OrbisError.invalid("Termina la diretta prima di eliminarla.")
            }
            var next = workspace
            next.history.removeAll { $0.id == id }; next.interruptedRuns.removeAll { $0.historyID == id }
            try store.save(next); workspace = next
            removeSessionLog(id)
        }
    }
    func replayFrom(_ history: LiveHistory, videoID: UUID, resume: Bool) {
        replay(history, resume: resume, startingAt: videoID)
    }
    func linkVideoProfile(historyID: UUID, videoID: UUID, profileID: UUID?) {
        perform {
            if let profileID, let profile = workspace.profiles.first(where: { $0.id == profileID }) { try profile.validateForStreaming() }
            guard let history = workspace.history.firstIndex(where: { $0.id == historyID }),
                  let video = workspace.history[history].playlist.firstIndex(where: { $0.id == videoID }) else { return }
            var next = workspace; next.history[history].playlist[video].profileID = profileID
            if let session = sessions.first(where: { $0.historyID == historyID && $0.isRunning }),
               session.playlist.contains(where: { $0.id == videoID }) {
                try session.linkProfile(workspace.profiles.first(where: { $0.id == profileID }), videoID: videoID)
            }
            try store.save(next); workspace = next
        }
    }
    func assignProfile(_ profile: EncoderProfile, historyID: UUID) {
        perform {
            try profile.validateForStreaming()
            if let session = sessions.first(where: { $0.historyID == historyID && $0.isRunning }) { try session.apply(profile) }
            if let index = workspace.history.firstIndex(where: { $0.id == historyID }) {
                workspace.history[index].profile = profile; try save()
            }
        }
    }
    func playDeck(_ button: DeckButton, session: LiveSession) {
        guard session.isRunning else { return }
        if UserDefaults.standard.object(forKey: "confirmSceneButtons") as? Bool ?? true {
            let alert = NSAlert()
            alert.messageText = button.label
            alert.informativeText = button.inScene && session.takeover?.id == button.id ? "Rimuovere la sovrapposizione e riprendere il programma?" : "Mettere in onda questo media per \(session.channel.name)?"
            alert.addButton(withTitle: NativeLocalization.text("confirm")); alert.addButton(withTitle: NativeLocalization.text("cancel"))
            guard alert.runModal() == .alertFirstButtonReturn else { return }
        }
        session.play(button)
    }
    func perform(_ action: () throws -> Void) { do { try action() } catch { self.error = error.localizedDescription } }
    func save() throws { try store.save(workspace) }
    func editWorkspace(_ action: (inout Workspace) throws -> Void) {
        perform { var next = workspace; try action(&next); try store.save(next); workspace = next }
    }
    func setChannelEnabled(_ id: UUID, _ enabled: Bool) {
        editWorkspace { next in if let index = next.channels.firstIndex(where: { $0.id == id }) { next.channels[index].enabled = enabled } }
    }
    func setProfileActive(_ id: UUID, _ active: Bool) {
        editWorkspace { next in if let index = next.profiles.firstIndex(where: { $0.id == id }) { next.profiles[index].active = active } }
    }
    func persist() { perform { try save() } }
    func saveChannel(_ channel: Channel, key: String) {
        perform {
            _ = try channel.destination(key: channel.platform == .tiktok ? "temporary-key" : key)
            if channel.platform == .tiktok { KeychainStore.delete(channel.id) } else { try KeychainStore.save(key, id: channel.id) }
            if let index = workspace.channels.firstIndex(where: { $0.id == channel.id }) { workspace.channels[index] = channel }
            else { workspace.channels.append(channel) }
            try save()
        }
    }
    func deleteHistoryVideo(historyID: UUID, videoID: UUID) {
        perform {
            guard !sessions.contains(where: { $0.historyID == historyID && $0.isRunning }) else { throw OrbisError.invalid("Termina la diretta prima di modificare lo storico.") }
            guard let index = workspace.history.firstIndex(where: { $0.id == historyID }) else { return }
            var next = workspace
            next.history[index].playlist.removeAll { $0.id == videoID }
            if next.history[index].lastVideoID == videoID {
                next.history[index].lastVideoID = nil
                next.interruptedRuns.removeAll { $0.historyID == historyID }
            } else if let checkpoint = next.interruptedRuns.firstIndex(where: { $0.historyID == historyID }),
                      let video = next.history[index].playlist.firstIndex(where: { $0.id == next.history[index].lastVideoID }) {
                next.interruptedRuns[checkpoint].index = video
            }
            try store.save(next); workspace = next
        }
    }
    func removeChannel(_ id: UUID) {
        perform {
            guard !sessions.contains(where: { $0.channel.id == id && $0.isRunning }) else { throw OrbisError.invalid("Termina la diretta prima di eliminare il canale.") }
            workspace.channels.removeAll { $0.id == id }; workspace.playlists.removeValue(forKey: id.uuidString)
            try save(); KeychainStore.delete(id)
        }
    }
    func start(channel: Channel, profile: EncoderProfile? = nil, scene overrideScene: Scene? = nil, playlist overridePlaylist: [PlaylistItem]? = nil, scenePosition: Double? = nil) {
        guard !startingChannels.contains(channel.id) else { return }
        guard !workspace.interruptedRuns.contains(where: { checkpoint in checkpoint.channelID == channel.id && !sessions.contains(where: { $0.historyID == checkpoint.historyID && $0.isRunning }) }) else {
            error = "Sblocca la sessione interrotta nella sezione Recupero prima di ripartire."; return
        }
        startingChannels.insert(channel.id)
        Task {
            defer { startingChannels.remove(channel.id) }
            do {
                guard channel.enabled else { throw OrbisError.invalid("Il canale è disabilitato.") }
                guard !sessions.contains(where: { $0.channel.id == channel.id && $0.isRunning }) else { throw OrbisError.invalid("Questo canale è già in trasmissione.") }
                let profile = profile ?? workspace.profiles.first(where: { $0.id == channel.profileID }) ?? workspace.profiles.first(where: { $0.active && $0.isDefault && $0.defaultPlatform == channel.platform }) ?? workspace.profiles.first(where: { $0.active && $0.isDefault }) ?? workspace.profiles.first(where: { $0.active }) ?? EncoderProfile()
                try profile.validateForStreaming()
                guard profile.active else { throw OrbisError.invalid("Il profilo selezionato è disattivato.") }
                if let scenePosition, !scenePosition.isFinite || scenePosition < 0 { throw OrbisError.invalid("Posizione di ripresa non valida.") }
                let scene = overrideScene ?? (overridePlaylist == nil ? workspace.scenes.first(where: { $0.id == channel.sceneID }) : nil)
                var playlist = overridePlaylist ?? workspace.playlists[channel.id.uuidString] ?? []
                if scene == nil && playlist.isEmpty && overridePlaylist == nil && !channel.videoFolder.isEmpty { playlist = try MediaFiles.playlist(folder: channel.videoFolder) }
                playlist = playlist.filter { $0.enabled }
                if let scene { try scene.validate(); try await AppleCapture.authorize(scene.items) }
                else if playlist.isEmpty { throw OrbisError.invalid("Aggiungi video alla playlist o seleziona una scena.") }
                let liveKey: String
                if channel.platform == .tiktok {
                    guard let supplied = FilePicker.liveKey(platform: channel.platform.title) else { return }; liveKey = supplied
                } else { liveKey = try KeychainStore.read(channel.id) }
                let destination = try channel.destination(key: liveKey)
                let ffmpeg = try Executables.resolve("ffmpeg", configured: workspace.ffmpegPath)
                let ffprobe = try Executables.resolve("ffprobe", configured: workspace.ffprobePath)
                var history = LiveHistory(channel: channel, profile: profile, scene: scene, playlist: playlist)
                history.scenePosition = scene != nil ? scenePosition : nil
                workspace.history.insert(history, at: 0)
                workspace.interruptedRuns.append(PlaybackCheckpoint(historyID: history.id, channelID: channel.id, index: 0, position: scenePosition ?? playlist.first?.resumeSeconds ?? 0))
                try save()
                let session = LiveSession(channel: channel, profile: profile, scene: scene, playlist: playlist, historyID: history.id,
                    ffmpeg: ffmpeg, ffprobe: ffprobe, destination: destination, directory: store.directory, loop: workspace.loopPlaylist, videoProfiles: Dictionary(uniqueKeysWithValues: workspace.profiles.map { ($0.id, $0) }), streamKey: liveKey, initialPosition: scenePosition) { [weak self] session in
                        guard let self else { return }
                        if let index = self.workspace.history.firstIndex(where: { $0.id == session.historyID }) {
                            self.workspace.history[index].end = Date(); self.workspace.history[index].error = session.lastError
                            self.workspace.history[index].profile = session.profile; self.workspace.history[index].scene = session.scene
                            self.workspace.history[index].scenePosition = session.scene != nil ? session.currentPosition : nil
                            self.workspace.history[index].playlist = session.playlist
                            if self.workspace.history[index].playlist.indices.contains(session.index) {
                                self.workspace.history[index].playlist[session.index].resumeSeconds = session.playlistPosition
                                self.workspace.history[index].lastVideoID = session.playlist[session.index].id
                            }
                        }
                        self.workspace.interruptedRuns.removeAll { $0.historyID == session.historyID }
                        if session.lastError != nil {
                            self.workspace.interruptedRuns.append(PlaybackCheckpoint(historyID: session.historyID, channelID: channel.id, index: session.index, position: session.currentPosition))
                        }
                        self.persist()
                    }
                sessions.append(session); selectedSession = session.id; open(.preview); session.start()
            } catch { self.error = error.localizedDescription }
        }
    }
    func refreshCapabilities() {
        Task {
            do { capabilities = try await FFmpegCapabilities.inspect(Executables.resolve("ffmpeg", configured: workspace.ffmpegPath)) }
            catch { capabilities = FFmpegCapabilities() }
        }
    }
    func checkUpdates() async {
        guard !checkingUpdates else { return }
        checkingUpdates = true; updateMessage = ""; defer { checkingUpdates = false }
        do {
            let installed = Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "3.0.0"
            availableUpdate = try await MacUpdates.check(feed: UserDefaults.standard.string(forKey: "updateFeedURL") ?? "", installed: installed)
            updateMessage = availableUpdate.map { "Versione disponibile: \($0.version)" } ?? "Nessun aggiornamento disponibile."
        } catch { updateMessage = error.localizedDescription }
    }
    func refreshDevices() {
        Task { do { devices = try await AppleCapture.devices(ffmpeg: Executables.resolve("ffmpeg", configured: workspace.ffmpegPath)) }
               catch { self.error = error.localizedDescription } }
    }
    func importPlaylist(channel: UUID, folder: Bool = false) {
        perform {
            let urls = FilePicker.files(multiple: !folder, folder: folder)
            var items = workspace.playlists[channel.uuidString] ?? []
            for url in urls {
                let added = folder ? try MediaFiles.playlist(folder: url.path) : [PlaylistItem(path: url.path)]
                items.append(contentsOf: added.filter { candidate in !items.contains(where: { $0.path == candidate.path }) && MediaFiles.videos.contains(URL(fileURLWithPath: candidate.path).pathExtension.lowercased()) })
            }
            workspace.playlists[channel.uuidString] = items; try save()
            for item in items { prepareMedia(item.path) }
            Task {
                guard let ffprobe = try? Executables.resolve("ffprobe", configured: workspace.ffprobePath) else { return }
                var durations: [String: Double] = [:]
                for item in items where item.duration == 0 {
                    if let info = try? await Probe.media(item.path, executable: ffprobe) { durations[item.path] = info.duration }
                }
                if !durations.isEmpty {
                    editWorkspace { next in
                        guard var saved = next.playlists[channel.uuidString] else { return }
                        for index in saved.indices { if let duration = durations[saved[index].path] { saved[index].duration = duration } }
                        next.playlists[channel.uuidString] = saved
                    }
                }
            }
        }
    }
    func prepareMedia(_ path: String) {
        guard let ffmpeg = try? Executables.resolve("ffmpeg", configured: workspace.ffmpegPath), let ffprobe = try? Executables.resolve("ffprobe", configured: workspace.ffprobePath) else { return }
        Task { await MediaOptimization.shared.prepare(path, ffmpeg: ffmpeg, ffprobe: ffprobe) }
    }
    func removeOverlay(_ path: String) {
        perform {
            guard !workspace.scenes.contains(where: { $0.items.contains(where: { $0.target == path }) }),
                  !workspace.history.contains(where: { $0.scene?.items.contains(where: { $0.target == path }) == true }),
                  !sessions.contains(where: { $0.isRunning && ($0.scene?.items.contains(where: { $0.target == path }) == true || $0.takeover?.mediaPath == path || $0.queuedSpots.contains(where: { $0.mediaPath == path })) }) else {
                throw OrbisError.invalid("L’overlay è usato da una scena o da una diretta.")
            }
            workspace.overlays.removeAll { $0 == path }; try save()
            if URL(fileURLWithPath: path).deletingLastPathComponent() == store.directory.appendingPathComponent("Overlays") { try FileManager.default.removeItem(atPath: path) }
        }
    }
    func removeProfile(_ id: UUID) {
        perform {
            guard !workspace.channels.contains(where: { $0.profileID == id }), !workspace.playlists.values.contains(where: { $0.contains(where: { $0.profileID == id }) }),
                  !workspace.history.contains(where: { $0.playlist.contains(where: { $0.profileID == id }) }),
                  !sessions.contains(where: { $0.isRunning && $0.profile.id == id }) else { throw OrbisError.invalid("Il profilo è associato a un canale, a un video o a una diretta.") }
            var next = workspace; next.profiles.removeAll { $0.id == id }; try store.save(next); workspace = next
        }
    }
    func removeScene(_ id: UUID) {
        perform {
            guard !workspace.channels.contains(where: { $0.sceneID == id }), !sessions.contains(where: { $0.isRunning && $0.scene?.id == id }) else { throw OrbisError.invalid("La scena è associata a un canale o a una diretta.") }
            var next = workspace; next.scenes.removeAll { $0.id == id }; try store.save(next); workspace = next
        }
    }
    func removeDeckButton(_ id: UUID) {
        perform {
            guard !sessions.contains(where: { $0.isRunning && ($0.takeover?.id == id || $0.queuedSpots.contains(where: { $0.id == id })) }) else { throw OrbisError.invalid("Il pulsante è in onda o in coda.") }
            workspace.deck.removeAll { $0.id == id }; try save()
        }
    }
    func importOverlay() {
        let urls = FilePicker.files(multiple: true)
        guard !urls.isEmpty else { return }
        Task {
            do {
                let ffprobe = try Executables.resolve("ffprobe", configured: workspace.ffprobePath)
                for url in urls {
                    guard MediaFiles.overlays.contains(url.pathExtension.lowercased()) else { throw OrbisError.invalid("Formato overlay non supportato.") }
                    _ = try await Probe.media(url.path, executable: ffprobe)
                }
                var next = workspace
                for url in urls {
                    let path = try store.importMedia(url, collection: "Overlays")
                    if !next.overlays.contains(path) { next.overlays.append(path) }
                }
                try store.save(next); workspace = next
            } catch { error = error.localizedDescription }
        }
    }
    func checkpoint() {
        for session in sessions where session.isRunning {
            let checkpoint = PlaybackCheckpoint(historyID: session.historyID, channelID: session.channel.id, index: session.index, position: session.currentPosition)
            workspace.interruptedRuns.removeAll { $0.historyID == session.historyID }; workspace.interruptedRuns.append(checkpoint)
            if let index = workspace.history.firstIndex(where: { $0.id == session.historyID }) {
                workspace.history[index].profile = session.profile; workspace.history[index].scene = session.scene
                workspace.history[index].scenePosition = session.scene != nil ? session.currentPosition : nil
                workspace.history[index].playlist = session.playlist
                if session.playlist.indices.contains(session.index) {
                    workspace.history[index].lastVideoID = session.playlist[session.index].id
                    workspace.history[index].playlist[session.index].resumeSeconds = session.playlistPosition
                }
            }
        }
        persist()
    }
    func unlock(_ checkpoint: PlaybackCheckpoint, restart: Bool) {
        guard !sessions.contains(where: { $0.historyID == checkpoint.historyID && $0.isRunning }) else { return }
        perform {
            var next = workspace
            next.interruptedRuns.removeAll { $0.historyID == checkpoint.historyID }
            if let index = next.history.firstIndex(where: { $0.id == checkpoint.historyID }), next.history[index].end == nil {
                next.history[index].end = checkpoint.updated
                next.history[index].error = "Sessione interrotta; posizione recuperata."
            }
            try store.save(next); workspace = next
            if restart, let history = next.history.first(where: { $0.id == checkpoint.historyID }) {
                var playlist = history.playlist
                if playlist.indices.contains(checkpoint.index) {
                    playlist.removeFirst(checkpoint.index)
                    if history.scene == nil { playlist[0].resumeSeconds = checkpoint.position }
                }
                start(channel: history.channel, profile: history.profile, scene: history.scene, playlist: playlist, scenePosition: history.scene != nil ? checkpoint.position : nil)
            }
        }
    }
    func cleanup(months: Int) {
        guard !cleanupProgress.running else { return }
        let active = Set(sessions.filter { $0.isRunning }.map(\.historyID))
        let eligible = HistoryCleanup.eligible(workspace.history, months: months, active: active)
        cleanupProgress = CleanupProgress(); cleanupProgress.running = true
        cleanupProgress.totalVideos = eligible.reduce(0) { $0 + max(1, $1.playlist.count) }
        Task {
            defer { cleanupProgress.running = false }
            do {
                for entry in eligible {
                    for name in entry.playlist.isEmpty ? [entry.scene?.name ?? entry.channel.name] : entry.playlist.map(\.name) {
                        cleanupProgress.currentVideo = name; cleanupProgress.deletedVideos += 1
                        await Task.yield()
                    }
                    workspace.history.removeAll { $0.id == entry.id }
                    workspace.interruptedRuns.removeAll { $0.historyID == entry.id }
                    cleanupProgress.deletedLives += 1
                }
                try save()
                for entry in eligible { removeSessionLog(entry.id) }
            } catch { cleanupProgress.error = error.localizedDescription; self.error = error.localizedDescription }
        }
    }
    private func removeSessionLog(_ id: UUID) {
        try? FileManager.default.removeItem(at: store.directory.appendingPathComponent("Logs/live-\(id.uuidString).log"))
    }
    private func cleanupIfNeeded() {
        guard !cleanupProgress.running else { return }
        var next = workspace
        var removed = Set<UUID>()
        var changed = false
        let active = Set(sessions.filter { $0.isRunning }.map(\.historyID))
        for index in next.channels.indices {
            let channel = next.channels[index]
            guard channel.cleanupEnabled, channel.cleanupIntervalMonths > 0, channel.cleanupOlderThanMonths > 0 else { continue }
            let due = Calendar.current.date(byAdding: .month, value: channel.cleanupIntervalMonths, to: channel.lastCleanup ?? .distantPast) ?? .distantPast
            if due <= Date(), let cutoff = Calendar.current.date(byAdding: .month, value: -channel.cleanupOlderThanMonths, to: Date()) {
                next.history.removeAll { entry in
                    let eligible = entry.channel.id == channel.id && entry.end != nil && entry.start < cutoff && !active.contains(entry.id)
                    if eligible { removed.insert(entry.id) }
                    return eligible
                }
                next.channels[index].lastCleanup = Date(); changed = true
            }
        }
        if changed {
            next.interruptedRuns.removeAll { removed.contains($0.historyID) }
            perform { try store.save(next); workspace = next; for id in removed { removeSessionLog(id) } }
        }
    }
    func exportWorkspace() {
        perform {
            let encoder = JSONEncoder(); encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
            var exported = workspace; exported.interruptedRuns = []
            try FilePicker.exportJSON(encoder.encode(exported), name: "OrbisStream.json")
        }
    }
    func importWorkspace() {
        perform {
            guard !sessions.contains(where: { $0.isRunning }) else { throw OrbisError.invalid("Termina le dirette prima di importare una configurazione.") }
            guard let url = FilePicker.files().first else { return }
            let imported = try JSONDecoder().decode(Workspace.self, from: Data(contentsOf: url))
            try imported.validate()
            // Save before replacing memory; an invalid import never overwrites the current document.
            try store.save(imported); workspace = imported
        }
    }
    func stopAll() { sessions.filter { $0.isRunning }.forEach { $0.stop() } }
}
