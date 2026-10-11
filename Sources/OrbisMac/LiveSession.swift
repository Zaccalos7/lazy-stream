import AppKit
import Combine
import OrbisCore

@MainActor final class LiveSession: ObservableObject, Identifiable {
    enum Status: String { case starting = "Avvio", live = "Trasmissione", reconnecting = "Riconnessione", stopped = "Terminata", failed = "Errore" }
    let id = UUID()
    let historyID: UUID
    let channel: Channel
    @Published var profile: EncoderProfile
    @Published var scene: Scene?
    @Published var playlist: [PlaylistItem]
    @Published var index = 0
    @Published var status: Status = .starting
    @Published var progress = StreamProgress()
    @Published var preview: NSImage?
    @Published var logs: [String] = []
    @Published var takeover: DeckButton?
    @Published var queuedSpots: [DeckButton] = []
    @Published var lastError: String?
    @Published var airState: AirState = .unverified
    @Published var onAirVideo: String?
    @Published var airWarningDismissed = false
    @Published var currentPosition = 0.0
    @Published var outputWidth = 1920
    @Published var outputHeight = 1080
    var isRunning: Bool { status == .starting || status == .live || status == .reconnecting }
    var sourceName: String { takeover?.label ?? scene?.name ?? (playlist.indices.contains(index) ? playlist[index].name : "Diretta") }
    var sources: [SceneItem] { scene?.items.filter { $0.audioEnabled && $0.kind != .overlay } ?? [] }
    private let ffmpeg: URL
    private let ffprobe: URL
    private let destination: String
    private let previewURL: URL
    private let loop: Bool
    private var engine: RelayEngine?
    private var task: Task<Void, Never>?
    private var takeoverTimer: Task<Void, Never>?
    private var activity: NSObjectProtocol?
    private var shouldStop = false
    private var requestedRestart = false
    private var activeSegmentSeek = 0.0
    private var activeSegmentIsProgram = false
    private var resumePosition = 0.0
    private var lastProgressTime = Date()
    private var connectionStarted = Date()
    private var watchdog: Task<Void, Never>?
    private var airWatch: Task<Void, Never>?
    private var bitrateController: BitrateController
    private let policy: StreamingPolicy
    private let videoProfiles: [UUID: EncoderProfile]
    private var appliedVideoID: UUID?
    private var takeoverGeneration = UUID()
    private var automaticQuality: Bool
    private var lowLeadSince: Double?
    private var warnedSlow = false
    private var requestedBitrate: Int
    private let fileLog: SessionLog
    private let finished: (LiveSession) -> Void
    init(channel: Channel, profile: EncoderProfile, scene: Scene?, playlist: [PlaylistItem], historyID: UUID,
         ffmpeg: URL, ffprobe: URL, destination: String, directory: URL, loop: Bool, videoProfiles: [UUID: EncoderProfile] = [:], finished: @escaping (LiveSession) -> Void) {
        self.channel = channel; self.profile = profile; self.scene = scene; self.playlist = playlist; self.historyID = historyID
        self.ffmpeg = ffmpeg; self.ffprobe = ffprobe; self.destination = destination; self.loop = loop; self.finished = finished
        policy = StreamingPolicy(platform: channel.platform); bitrateController = BitrateController(policy: policy); self.videoProfiles = videoProfiles
        automaticQuality = profile.quality == .auto
        self.requestedBitrate = profile.videoBitrate
        self.resumePosition = playlist.first?.resumeSeconds ?? 0
        fileLog = SessionLog(directory: directory.appendingPathComponent("Logs"))
        previewURL = directory.appendingPathComponent("preview-\(UUID().uuidString).jpg")
    }
    func start() {
        activity = ProcessInfo.processInfo.beginActivity(options: [.userInitiated, .idleSystemSleepDisabled], reason: "Trasmissione OrbisStream")
        task = Task { await run() }
        if channel.platform == .youtube {
            airWatch = Task { [weak self] in
                do { try await Task.sleep(for: .seconds(90)) } catch { return }
                var absent = 0; var ended = 0
                while !Task.isCancelled {
                    guard let self, self.isRunning else { return }
                    let result = await YouTubeAir.check(name: self.channel.streamName, previousVideo: self.onAirVideo)
                    if Task.isCancelled { return }
                    if result.state == .onAir {
                        self.airState = .onAir; self.onAirVideo = result.videoID; self.airWarningDismissed = false; absent = 0; ended = 0
                    } else if result.state == .unverified { self.airState = .unverified }
                    else {
                        absent += 1; ended = result.state == .ended ? ended + 1 : 0
                        if absent >= 2 { self.airState = result.state }
                        if ended >= 2 { self.appendLog("YouTube conferma due volte che la trasmissione pubblica è terminata."); self.stop(); return }
                    }
                    do { try await Task.sleep(for: .seconds(30)) } catch { return }
                }
            }
        }
        watchdog = Task { [weak self] in
            while !Task.isCancelled {
                try? await Task.sleep(for: .seconds(1))
                guard let self, self.isRunning else { return }
                self.updatePreview()
                if Date().timeIntervalSince(self.lastProgressTime) > (self.status == .starting ? self.policy.connectTimeout : self.policy.stallTimeout) {
                    self.appendLog("Il publisher non conferma avanzamento entro il timeout della piattaforma.")
                    self.engine?.stop()
                }
            }
        }
    }
    func stop() { updateProgramPosition(); shouldStop = true; takeoverTimer?.cancel(); engine?.stop() }
    func apply(_ newProfile: EncoderProfile) throws {
        try newProfile.validate()
        guard newProfile.format == "flv" else { throw OrbisError.invalid("Le dirette RTMP usano il contenitore FLV.") }
        profile = newProfile; automaticQuality = newProfile.quality == .auto; lowLeadSince = nil; warnedSlow = false; bitrateController = BitrateController(policy: policy); requestedBitrate = newProfile.videoBitrate; restart()
    }
    func setVolume(id: UUID, value: Double) {
        guard let item = scene?.items.firstIndex(where: { $0.id == id }) else { return }
        scene?.items[item].volume = min(200, max(0, value)); restart()
    }
    func switchScene(_ value: Scene) throws { try value.validate(); takeoverTimer?.cancel(); takeoverGeneration = UUID(); queuedSpots.removeAll(); scene = value; takeover = nil; resumePosition = 0; currentPosition = 0; restart(preservePosition: false) }
    var processDetails: [StreamingProcess] { engine?.processDetails ?? [] }
    func enqueueSpot(_ path: String) throws {
        guard isRunning, FileManager.default.fileExists(atPath: path),
              MediaFiles.videos.contains(URL(fileURLWithPath: path).pathExtension.lowercased()) || MediaFiles.isPicture(path) else {
            throw OrbisError.invalid("Seleziona un media valido per una diretta attiva.")
        }
        var button = DeckButton(); button.mediaPath = path; button.label = URL(fileURLWithPath: path).lastPathComponent
        if let takeover, !takeover.inScene && !MediaFiles.isPicture(takeover.mediaPath) { queuedSpots.append(button) }
        else { beginTakeover(button) }
    }
    func play(_ button: DeckButton) {
        guard isRunning else { return }
        if button.inScene && takeover?.id == button.id { resume(); return }
        queuedSpots.removeAll(); beginTakeover(button)
    }
    private func beginTakeover(_ button: DeckButton) {
        updateProgramPosition()
        if activeSegmentIsProgram { resumePosition = currentPosition }
        takeover = button; takeoverGeneration = UUID(); let generation = takeoverGeneration
        takeoverTimer?.cancel(); restart()
        if button.durationSeconds > 0 {
            takeoverTimer = Task { [weak self] in
                do { try await Task.sleep(for: .seconds(button.durationSeconds)) } catch { return }
                guard self?.takeoverGeneration == generation else { return }
                self?.finishTakeover()
            }
        }
    }
    func resume() { queuedSpots.removeAll(); takeover = nil; takeoverGeneration = UUID(); takeoverTimer?.cancel(); restart() }
    private func finishTakeover() {
        takeoverTimer?.cancel(); takeover = nil; takeoverGeneration = UUID()
        if !queuedSpots.isEmpty { beginTakeover(queuedSpots.removeFirst()) } else { restart() }
    }
    func skip(to next: Int? = nil) {
        guard !playlist.isEmpty else { return }
        takeoverTimer?.cancel(); takeoverGeneration = UUID(); queuedSpots.removeAll(); scene = nil
        index = next.map { min(max(0, $0), playlist.count - 1) } ?? ((index + 1) % playlist.count)
        takeover = nil; resumePosition = 0; restart(preservePosition: false)
    }
    private func restart(preservePosition: Bool = true) {
        if preservePosition { updateProgramPosition(); if activeSegmentIsProgram { resumePosition = currentPosition } }
        requestedRestart = true; engine?.interruptProducer()
    }
    private func run() async {
        var retries = 0
        while !shouldStop {
            let relay = RelayEngine(executable: ffmpeg, destination: destination, platform: channel.platform,
                progress: { [weak self] update in Task { @MainActor in self?.receive(update) } },
                log: { [weak self] message in Task { @MainActor in self?.appendLog(message) } })
            engine = relay
            do {
                try relay.start(); lastProgressTime = Date(); connectionStarted = Date()
                while !shouldStop {
                    requestedRestart = false
                    if scene == nil, takeover == nil, playlist.indices.contains(index), appliedVideoID != playlist[index].id {
                        appliedVideoID = playlist[index].id
                        if let id = playlist[index].profileID, var selected = videoProfiles[id] {
                            if policy.uniformFormat && profile.width > 0 { selected.width = profile.width; selected.height = profile.height; selected.frameRate = profile.frameRate }
                            try selected.validate(); automaticQuality = selected.quality == .auto; profile = selected; requestedBitrate = selected.videoBitrate
                            bitrateController = BitrateController(policy: policy)
                        }
                    }
                    var program = try makeScene()
                    try await AppleCapture.authorize(program.items)
                    var media: [UUID: MediaInfo] = [:]
                    for item in program.items where item.kind == .file || item.kind == .overlay {
                        if shouldStop { throw CancellationError() }
                        media[item.id] = try await Probe.media(item.target, executable: ffprobe)
                    }
                    if shouldStop { break }; if requestedRestart { continue }
                    // Freeze output format after first source, including profiles that preserve source dimensions/fps.
                    if profile.width == 0 {
                        if scene == nil, takeover == nil, let item = program.items.first, let info = media[item.id] {
                            program.width = info.width - info.width % 2; program.height = info.height - info.height % 2
                            program.items[0].width = program.width; program.items[0].height = program.height
                        }
                        if policy.uniformFormat { profile.width = program.width; profile.height = program.height }
                    }
                    if profile.frameRate == 0 && policy.uniformFormat { profile.frameRate = program.items.first.flatMap { media[$0.id]?.frameRate } ?? 30 }
                    if profile.quality == .auto {
                        profile.quality = try await EncoderTuner.shared.resolve(profile: profile, executable: ffmpeg)
                        appendLog("Qualità automatica: \(profile.quality.rawValue)")
                    }
                    let seek = (takeover == nil || takeover?.inScene == true) ? resumePosition : 0
                    var hardwareDecoding: Set<UUID> = []
                    for index in program.items.indices where program.items[index].kind == .file && !MediaFiles.isPicture(program.items[index].target) {
                        let item = program.items[index]
                        let optimized = await MediaOptimization.shared.resolve(item.target, tileWidth: item.width, tileHeight: item.height, ffmpeg: ffmpeg, ffprobe: ffprobe)
                        program.items[index].target = optimized.path
                        if optimized.hardwareDecoding { hardwareDecoding.insert(item.id) }
                    }
                    if shouldStop { break }; if requestedRestart { continue }
                    let plan = try FFmpegCommand.build(scene: program, profile: profile, platform: channel.platform, media: media, preview: previewURL.path, resume: seek, hardwareDecoding: hardwareDecoding)
                    outputWidth = plan.width; outputHeight = plan.height
                    activeSegmentSeek = seek; activeSegmentIsProgram = takeover == nil || takeover?.inScene == true
                    var exit: Int32 = 0
                    do { exit = try await relay.encode(arguments: plan.arguments, frameRate: plan.frameRate) }
                    catch is CancellationError { if shouldStop { break }; if !requestedRestart { throw OrbisError.process("Trasmissione interrotta.") } }
                    updateProgramPosition()
                    if shouldStop { break }
                    if requestedRestart { continue }
                    guard exit == 0 else { throw OrbisError.process("FFmpeg ha terminato la codifica (\(exit)). Consulta il registro.") }
                    if takeover?.inScene == false { finishTakeover(); continue }
                    if takeover != nil { takeover = nil; takeoverTimer?.cancel() }
                    if scene != nil { shouldStop = true; break }
                    resumePosition = 0
                    repeat { index += 1 } while index < playlist.count && !playlist[index].enabled
                    if index < playlist.count { resumePosition = playlist[index].resumeSeconds }
                    if index >= playlist.count {
                        if loop, let first = playlist.firstIndex(where: { $0.enabled }) { index = first; resumePosition = playlist[first].resumeSeconds }
                        else { shouldStop = true; break }
                    }
                }
                await relay.finish()
                if shouldStop { break }
            } catch {
                await relay.finish()
                if shouldStop { break }
                if Date().timeIntervalSince(connectionStarted) > 60 && progress.seconds > 30 { retries = 0 }
                if retries >= channel.platform.retries {
                    lastError = error.localizedDescription; status = .failed; appendLog(error.localizedDescription); break
                }
                retries += 1; status = .reconnecting; appendLog("Riconnessione \(retries): \(error.localizedDescription)")
                resumePosition = currentPosition; progress = StreamProgress()
                try? await Task.sleep(for: .seconds(min(10, retries * 2)))
            }
        }
        if status != .failed { status = .stopped }
        engine?.stop(); engine = nil; watchdog?.cancel(); airWatch?.cancel(); takeoverTimer?.cancel()
        if let activity { ProcessInfo.processInfo.endActivity(activity) }; activity = nil
        try? FileManager.default.removeItem(at: previewURL)
        fileLog.flush(); finished(self)
    }
    private func makeScene() throws -> Scene {
        var result: Scene
        if let scene { result = scene }
        else {
            guard playlist.indices.contains(index), playlist[index].enabled else { throw OrbisError.invalid("La playlist non contiene video abilitati.") }
            result = Scene(); result.name = playlist[index].name
            result.width = profile.width == 0 ? 1920 : profile.width; result.height = profile.height == 0 ? 1080 : profile.height
            var item = SceneItem(); item.target = playlist[index].path; item.label = playlist[index].name
            item.width = result.width; item.height = result.height; result.items = [item]
        }
        if let button = takeover {
            var item = SceneItem(); item.target = button.mediaPath; item.label = button.label
            if button.inScene {
                item.kind = .overlay; item.audioEnabled = false
                item.width = min(button.width, result.width); item.height = min(button.height, result.height)
                switch button.placement {
                case "top-left": item.x = 0; item.y = 0
                case "top-right": item.x = result.width - item.width; item.y = 0
                case "bottom-left": item.x = 0; item.y = result.height - item.height
                case "center": item.x = (result.width - item.width) / 2; item.y = (result.height - item.height) / 2
                case "custom": item.x = min(button.x, result.width - item.width); item.y = min(button.y, result.height - item.height)
                default: item.x = result.width - item.width; item.y = result.height - item.height
                }
                result.items.append(item)
            } else { item.width = result.width; item.height = result.height; result.items = [item] }
        }
        return result
    }
    private func updateProgramPosition() {
        if activeSegmentIsProgram, let engine { currentPosition = activeSegmentSeek + engine.currentSegmentPosition }
    }
    private func receive(_ value: StreamProgress) {
        guard isRunning else { return }
        let advanced = value.seconds - progress.seconds
        let moved = advanced > 0
        progress = value
        if moved {
            lastProgressTime = Date(); status = .live
            updateProgramPosition()
            guard let sample = engine?.deliverySample(media: value.seconds, bytes: value.bytes) else { return }
            if sample.lead < policy.maxLead * 0.25 {
                if lowLeadSince == nil { lowLeadSince = sample.time }
                if sample.time - (lowLeadSince ?? sample.time) > 10 {
                    if automaticQuality && profile.quality != .light {
                        profile.quality = profile.quality == .high ? .balanced : .light; lowLeadSince = nil
                        appendLog("L’encoder non mantiene il margine: qualità automatica \(profile.quality.rawValue).")
                        restart(); return
                    } else if !warnedSlow { warnedSlow = true; appendLog("L’encoder non mantiene il margine. Riduci la risoluzione o usa sorgenti più leggere.") }
                }
            } else { lowLeadSince = nil }
            if profile.adaptiveBitrate, let decision = bitrateController.observe(sample, nominal: requestedBitrate, audioBitrate: profile.audioBitrate) {
                profile.videoBitrate = decision.to
                appendLog("Bitrate adattivo: \(decision.from / 1000) → \(decision.to / 1000) kbit/s; throughput \(decision.throughput / 1000) kbit/s")
                restart()
            }
        }
    }
    private func updatePreview() {
        if let data = try? Data(contentsOf: previewURL), let image = NSImage(data: data) { preview = image }
    }
    private func appendLog(_ value: String) {
        guard !value.isEmpty else { return }
        let safe = value.replacingOccurrences(of: destination, with: "[server protetto]")
        fileLog.append("[\(channel.name)] \(safe)")
        logs.append("\(Date().formatted(date: .omitted, time: .standard)) \(safe)")
        if logs.count > 300 { logs.removeFirst(logs.count - 300) }
    }
}
