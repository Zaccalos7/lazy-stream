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
    @Published var cleanupProgress = CleanupProgress()
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
        refreshCapabilities()
        monitor = NSEvent.addLocalMonitorForEvents(matching: .keyDown) { [weak self] event in
            guard let self, !(NSApp.keyWindow?.firstResponder is NSTextView), let session = self.watched, session.isRunning else { return event }
            let flags = event.modifierFlags.intersection(.deviceIndependentFlagsMask)
            let prefix = (flags.contains(.command) ? "Cmd+" : "") + (flags.contains(.option) ? "Option+" : "") + (flags.contains(.shift) ? "Shift+" : "") + (flags.contains(.control) ? "Ctrl+" : "")
            var character = (event.charactersIgnoringModifiers ?? "").uppercased()
            if let scalar = character.unicodeScalars.first {
                if (0xf704...0xf71b).contains(scalar.value) { character = "F\(scalar.value - 0xf704 + 1)" }
                else if let named = [UInt32(0xf700): "ARROWUP", 0xf701: "ARROWDOWN", 0xf702: "ARROWLEFT", 0xf703: "ARROWRIGHT", 0xf72c: "PAGEUP", 0xf72d: "PAGEDOWN", 0xf729: "HOME", 0xf72b: "END"][scalar.value] { character = named }
            }
            let key = DeckHotkey(prefix + character)
            guard let button = self.workspace.deck.first(where: { !$0.hotkey.isEmpty && DeckHotkey($0.hotkey) == key && key != nil }) else { return event }
            session.play(button); return nil
        }
    }
    var watched: LiveSession? { sessions.first { $0.id == selectedSession } ?? sessions.first(where: { $0.isRunning }) ?? sessions.last }
    func perform(_ action: () throws -> Void) { do { try action() } catch { self.error = error.localizedDescription } }
    func save() throws { try store.save(workspace) }
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
            workspace.history[index].playlist.removeAll { $0.id == videoID }; try save()
        }
    }
    func removeChannel(_ id: UUID) {
        perform {
            guard !sessions.contains(where: { $0.channel.id == id && $0.isRunning }) else { throw OrbisError.invalid("Termina la diretta prima di eliminare il canale.") }
            workspace.channels.removeAll { $0.id == id }; workspace.playlists.removeValue(forKey: id.uuidString)
            try save(); KeychainStore.delete(id)
        }
    }
    func start(channel: Channel, profile: EncoderProfile? = nil, scene overrideScene: Scene? = nil, playlist overridePlaylist: [PlaylistItem]? = nil) {
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
                try profile.validate()
                guard profile.format == "flv" else { throw OrbisError.invalid("Per RTMP scegli un profilo FLV/H.264.") }
                let scene = overrideScene ?? workspace.scenes.first(where: { $0.id == channel.sceneID })
                var playlist = overridePlaylist ?? workspace.playlists[channel.id.uuidString] ?? []
                if scene == nil && playlist.isEmpty { playlist = try MediaFiles.playlist(folder: channel.videoFolder) }
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
                let history = LiveHistory(channel: channel, profile: profile, scene: scene, playlist: playlist)
                workspace.history.insert(history, at: 0)
                workspace.interruptedRuns.append(PlaybackCheckpoint(historyID: history.id, channelID: channel.id, index: 0, position: playlist.first?.resumeSeconds ?? 0))
                try save()
                let session = LiveSession(channel: channel, profile: profile, scene: scene, playlist: playlist, historyID: history.id,
                    ffmpeg: ffmpeg, ffprobe: ffprobe, destination: destination, directory: store.directory, loop: workspace.loopPlaylist, videoProfiles: Dictionary(uniqueKeysWithValues: workspace.profiles.map { ($0.id, $0) })) { [weak self] session in
                        guard let self else { return }
                        if let index = self.workspace.history.firstIndex(where: { $0.id == session.historyID }) {
                            self.workspace.history[index].end = Date(); self.workspace.history[index].error = session.lastError
                            self.workspace.history[index].profile = session.profile; self.workspace.history[index].scene = session.scene
                            self.workspace.history[index].playlist = session.playlist
                            if self.workspace.history[index].playlist.indices.contains(session.index) {
                                self.workspace.history[index].playlist[session.index].resumeSeconds = session.currentPosition
                            }
                        }
                        self.workspace.interruptedRuns.removeAll { $0.historyID == session.historyID }
                        if session.lastError != nil {
                            self.workspace.interruptedRuns.append(PlaybackCheckpoint(historyID: session.historyID, channelID: channel.id, index: session.index, position: session.currentPosition))
                        }
                        self.persist()
                    }
                sessions.append(session); selectedSession = session.id; session.start()
            } catch { self.error = error.localizedDescription }
        }
    }
    func refreshCapabilities() {
        Task {
            do { capabilities = try await FFmpegCapabilities.inspect(Executables.resolve("ffmpeg", configured: workspace.ffmpegPath)) }
            catch { capabilities = FFmpegCapabilities() }
        }
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
                  !sessions.contains(where: { $0.isRunning && ($0.scene?.items.contains(where: { $0.target == path }) == true || $0.takeover?.mediaPath == path) }) else {
                throw OrbisError.invalid("L’overlay è usato da una scena o da una diretta.")
            }
            workspace.overlays.removeAll { $0 == path }; try save()
            if URL(fileURLWithPath: path).deletingLastPathComponent() == store.directory.appendingPathComponent("Overlays") { try FileManager.default.removeItem(atPath: path) }
        }
    }
    func removeProfile(_ id: UUID) {
        perform {
            guard !workspace.channels.contains(where: { $0.profileID == id }), !workspace.playlists.values.contains(where: { $0.contains(where: { $0.profileID == id }) }),
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
        perform {
            for url in FilePicker.files(multiple: true) {
                let ext = url.pathExtension.lowercased()
                guard MediaFiles.stills.contains(ext) || ["gif", "apng", "webm", "mov"].contains(ext) else { throw OrbisError.invalid("Formato overlay non supportato.") }
                workspace.overlays.append(try store.importMedia(url, collection: "Overlays"))
            }
            try save()
        }
    }
    func checkpoint() {
        for session in sessions where session.isRunning {
            let checkpoint = PlaybackCheckpoint(historyID: session.historyID, channelID: session.channel.id, index: session.index, position: session.currentPosition)
            workspace.interruptedRuns.removeAll { $0.historyID == session.historyID }; workspace.interruptedRuns.append(checkpoint)
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
                    playlist.removeFirst(checkpoint.index); playlist[0].resumeSeconds = checkpoint.position
                }
                start(channel: history.channel, profile: history.profile, scene: history.scene, playlist: playlist)
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
            } catch { cleanupProgress.error = error.localizedDescription; self.error = error.localizedDescription }
        }
    }
    private func cleanupIfNeeded() {
        var changed = false
        for index in workspace.channels.indices {
            let channel = workspace.channels[index]
            guard channel.cleanupEnabled, channel.cleanupIntervalMonths > 0, channel.cleanupOlderThanMonths > 0 else { continue }
            let due = Calendar.current.date(byAdding: .month, value: channel.cleanupIntervalMonths, to: channel.lastCleanup ?? .distantPast) ?? .distantPast
            if due <= Date(), let cutoff = Calendar.current.date(byAdding: .month, value: -channel.cleanupOlderThanMonths, to: Date()) {
                workspace.history.removeAll { $0.channel.id == channel.id && $0.end != nil && $0.start < cutoff }
                workspace.channels[index].lastCleanup = Date(); changed = true
            }
        }
        if changed { persist() }
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
