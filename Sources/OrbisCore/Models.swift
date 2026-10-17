import Foundation

public enum SourceKind: String, Codable, CaseIterable, Sendable {
    case file, screen, camera, microphone, overlay
    public var title: String {
        switch self { case .file: return "Video"; case .screen: return "Schermo"; case .camera: return "Fotocamera"; case .microphone: return "Microfono"; case .overlay: return "Overlay" }
    }
}
public enum Platform: String, Codable, CaseIterable, Sendable {
    case custom, twitch, youtube, kick, facebook, tiktok
    public var title: String {
        switch self {
        case .custom: return "Personalizzata"
        case .youtube: return "YouTube"
        case .tiktok: return "TikTok"
        case .facebook: return "Facebook Gaming"
        default: return rawValue.capitalized
        }
    }
    public var sampleRate: Int { self == .twitch || self == .custom ? 44100 : 48000 }
    public var constantRate: Bool { self == .youtube || self == .facebook }
    public var retries: Int { self == .custom ? 0 : self == .youtube || self == .facebook ? 5 : 3 }
    public static func detect(_ address: String) -> Platform {
        guard let host = URL(string: address)?.host?.lowercased() else { return .custom }
        func under(_ domain: String) -> Bool { host == domain || host.hasSuffix("." + domain) }
        if under("kick.com") { return .kick }
        if host.hasSuffix(".global-contribute.live-video.net"), let prefix = host.split(separator: ".").first, prefix.count == 12, prefix.allSatisfy({ $0.isHexDigit }) { return .kick }
        if under("twitch.tv") || under("live-video.net") { return .twitch }
        if under("youtube.com") { return .youtube }
        if under("facebook.com") { return .facebook }
        if ["tiktokcdn.com", "tiktokcdn-eu.com", "tiktokcdn-us.com", "tiktokv.com", "tiktok.com"].contains(where: under) { return .tiktok }
        return .custom
    }
}
public struct Channel: Identifiable, Codable, Equatable, Sendable {
    public var id = UUID()
    public var name = "Nuovo canale"
    public var streamURL = ""
    public var streamName = ""
    public var description = ""
    public var videoFolder = ""
    public var enabled = true
    public var sceneID: UUID?
    public var profileID: UUID?
    public var cleanupEnabled = false
    public var cleanupIntervalMonths = 1
    public var cleanupOlderThanMonths = 6
    public var lastCleanup: Date?
    public var platform: Platform { Platform.detect(streamURL) }
    public init() {}
    public func destination(key: String) throws -> String {
        guard var parts = URLComponents(string: streamURL.trimmingCharacters(in: .whitespacesAndNewlines)),
              ["rtmp", "rtmps"].contains(parts.scheme?.lowercased() ?? ""), parts.host != nil,
              !key.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty else { throw OrbisError.invalid("Inserisci un server RTMP/RTMPS e una chiave di streaming.") }
        if let host = parts.host, host == "rtmp.youtube.com" || host.hasSuffix(".rtmp.youtube.com") {
            parts.host = host.replacingOccurrences(of: "rtmp.youtube.com", with: "rtmps.youtube.com")
            parts.scheme = "rtmps"
        }
        let serverQuery = parts.percentEncodedQuery
        parts.percentEncodedQuery = nil; parts.fragment = nil
        guard let base = parts.string else { throw OrbisError.invalid("Server non valido.") }
        // Keys may contain a query issued by the platform. Preserve it verbatim.
        let output = base.trimmingCharacters(in: CharacterSet(charactersIn: "/")) + "/" + key
        return output + (serverQuery.map { (key.contains("?") ? "&" : "?") + $0 } ?? "")
    }
}
public enum EncoderQuality: String, Codable, CaseIterable, Sendable { case auto, light, balanced, high }
public struct EncoderOption: Identifiable, Codable, Equatable, Sendable {
    public var id = UUID()
    public var key = ""
    public var value = ""
    public init(key: String = "", value: String = "") { self.key = key; self.value = value }
}
public struct EncoderProfile: Identifiable, Codable, Equatable, Sendable {
    public var id = UUID()
    public var title = "Apple H.264 · 1080p"
    public var codec = "h264_videotoolbox"
    public var pixelFormat = "yuv420p"
    public var format = "flv"
    public var videoBitrate = 6_000_000
    public var audioCodec = "aac"
    public var audioBitrate = 160_000
    public var width = 1920
    public var height = 1080
    public var frameRate = 30.0
    public var keyframeSeconds = 2
    public var quality: EncoderQuality = .auto
    public var lowLatency = false
    public var adaptiveBitrate = true
    public var active = true
    public var isDefault = true
    public var defaultPlatform: Platform = .custom
    public var preset = ""
    public var tune = ""
    public var options: [EncoderOption] = []
    public init() {}
    public func validate() throws {
        guard !title.isEmpty, videoBitrate >= 100_000, videoBitrate <= 100_000_000,
              audioBitrate >= 32_000, audioBitrate <= 512_000, keyframeSeconds > 0,
              (width == 0 && height == 0) || (width >= 2 && height >= 2 && width <= 7680 && height <= 7680 && width % 2 == 0 && height % 2 == 0),
              frameRate == 0 || (frameRate >= 1 && frameRate <= 240) else { throw OrbisError.invalid("Controlla risoluzione (pari, oppure 0×0), fps, bitrate e intervallo dei keyframe.") }
        guard CodecCatalog.videoEncoders.contains(codec),
              CodecCatalog.pixelFormats.contains(pixelFormat), CodecCatalog.formats.contains(format),
              CodecCatalog.audioEncoders.contains(audioCodec) else { throw OrbisError.invalid("Codec o formato non supportato.") }
        let reserved: Set<String> = ["i", "f", "map", "filter_complex", "filter_complex_script", "progress", "y", "n", "loglevel", "report", "passlogfile", "vstats_file", "attach", "dump_attachment", "nostdin", "c", "c:v", "c:a"]
        for option in options {
            guard !option.key.isEmpty, option.key.allSatisfy({ $0.isLetter || $0.isNumber || "_:-".contains($0) }), !option.key.hasPrefix("-"), !reserved.contains(option.key) else { throw OrbisError.invalid("Opzione FFmpeg non valida o riservata: \(option.key)") }
        }
    }
}
public struct SceneItem: Identifiable, Codable, Equatable, Sendable {
    public var id = UUID()
    public var kind: SourceKind = .file
    public var target = ""
    public var label = "Sorgente"
    public var x = 0
    public var y = 0
    public var width = 1920
    public var height = 1080
    public var audioEnabled = true
    public var volume = 100.0
    public init() {}
}
public struct Scene: Identifiable, Codable, Equatable, Sendable {
    public var id = UUID()
    public var name = "Nuova scena"
    public var description = ""
    public var isLayout = false
    public var width = 1920
    public var height = 1080
    public var items: [SceneItem] = []
    public init() {}
    public func validate(requireSources: Bool = true) throws {
        guard !name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty else { throw OrbisError.invalid("Inserisci il nome della scena.") }
        guard width >= 2, height >= 2, width % 2 == 0, height % 2 == 0, width <= 7680, height <= 7680 else { throw OrbisError.invalid("Dimensioni della scena non valide.") }
        guard !requireSources || items.contains(where: { $0.kind != .microphone && $0.kind != .overlay && !$0.target.isEmpty }) else { throw OrbisError.invalid("Aggiungi una sorgente video alla scena.") }
        let devices = items.filter { [.screen, .camera, .microphone].contains($0.kind) && !$0.target.isEmpty }.map { ($0.kind == .microphone ? "audio:" : "video:") + (Int($0.target).map(String.init) ?? $0.target) }
        guard Set(devices).count == devices.count else { throw OrbisError.invalid("Un dispositivo può essere aperto una sola volta nella stessa scena.") }
        if isLayout && items.isEmpty { throw OrbisError.invalid("Aggiungi almeno un riquadro al layout.") }
        for item in items {
            if [.screen, .camera, .microphone].contains(item.kind), !item.target.isEmpty {
                guard let index = Int(item.target), index >= 0 else { throw OrbisError.invalid("Seleziona un indice AVFoundation valido per \(item.label).") }
            }
            let validRectangle = item.kind == .microphone || (item.x >= 0 && item.y >= 0 && item.width >= 2 && item.height >= 2 && item.width <= width && item.height <= height && item.width % 2 == 0 && item.height % 2 == 0 && item.x <= width - item.width && item.y <= height - item.height)
            guard validRectangle, (0...200).contains(item.volume), !requireSources || !item.target.isEmpty else { throw OrbisError.invalid("Controlla sorgente, posizione, dimensioni e volume di \(item.label).") }
        }
    }
}
public struct DeckButton: Identifiable, Codable, Equatable, Sendable {
    public var id = UUID()
    public var label = "Intermezzo"
    public var mediaPath = ""
    public var hotkey = ""
    public var inScene = false
    public var placement = "bottom-right"
    public var durationSeconds = 0
    public var x = 0
    public var y = 0
    public var width = 480
    public var height = 270
    public init() {}
}
public struct PlaylistItem: Identifiable, Codable, Equatable, Sendable {
    public var id = UUID()
    public var path: String
    public var enabled = true
    public var resumeSeconds = 0.0
    public var duration = 0.0
    public var profileID: UUID?
    public var name: String { URL(fileURLWithPath: path).lastPathComponent }
    public init(path: String) { self.path = path }
    private enum CodingKeys: String, CodingKey { case id, path, enabled, resumeSeconds, duration, profileID }
    public init(from decoder: Decoder) throws {
        let values = try decoder.container(keyedBy: CodingKeys.self)
        id = try values.decodeIfPresent(UUID.self, forKey: .id) ?? UUID()
        path = try values.decodeIfPresent(String.self, forKey: .path) ?? ""
        enabled = try values.decodeIfPresent(Bool.self, forKey: .enabled) ?? true
        resumeSeconds = try values.decodeIfPresent(Double.self, forKey: .resumeSeconds) ?? 0
        duration = try values.decodeIfPresent(Double.self, forKey: .duration) ?? 0
        profileID = try values.decodeIfPresent(UUID.self, forKey: .profileID)
    }

}
public struct LiveHistory: Identifiable, Codable, Equatable, Sendable {
    public var id = UUID()
    public var channel: Channel
    public var profile: EncoderProfile
    public var scene: Scene?
    public var playlist: [PlaylistItem]
    public var start = Date()
    public var end: Date?
    public var error: String?
    public var lastVideoID: UUID?
    public init(channel: Channel, profile: EncoderProfile, scene: Scene?, playlist: [PlaylistItem]) {
        self.channel = channel; self.profile = profile; self.scene = scene; self.playlist = playlist
    }
}
public struct PlaybackCheckpoint: Identifiable, Codable, Sendable {
    public var id: UUID { historyID }
    public var historyID: UUID
    public var channelID: UUID
    public var index: Int
    public var position: Double
    public var updated: Date
    public init(historyID: UUID, channelID: UUID, index: Int, position: Double) {
        self.historyID = historyID; self.channelID = channelID; self.index = index; self.position = position; updated = Date()
    }
}
public struct Workspace: Codable, Sendable {
    public var channels: [Channel] = []
    public var profiles: [EncoderProfile] = PlatformPresets.defaults()
    public var scenes: [Scene] = []
    public var deck: [DeckButton] = []
    public var playlists: [String: [PlaylistItem]] = [:]
    public var history: [LiveHistory] = []
    public var overlays: [String] = []
    public var ffmpegPath = ""
    public var ffprobePath = ""
    public var loopPlaylist = true
    public var interruptedRuns: [PlaybackCheckpoint] = []
    public init() {}
    public func validate() throws {
        func unique(_ values: [UUID]) throws {
            guard Set(values).count == values.count else { throw OrbisError.invalid("La configurazione contiene identificatori duplicati.") }
        }
        try unique(channels.map(\.id)); try unique(profiles.map(\.id)); try unique(scenes.map(\.id))
        try unique(deck.map(\.id)); try unique(history.map(\.id)); try unique(interruptedRuns.map(\.id))
        for items in playlists.values {
            try unique(items.map(\.id))
            guard items.allSatisfy({ $0.resumeSeconds.isFinite && $0.resumeSeconds >= 0 && $0.duration.isFinite && $0.duration >= 0 }) else { throw OrbisError.invalid("Posizioni della playlist non valide.") }
        }
        for profile in profiles { try profile.validate() }
        for scene in scenes { try unique(scene.items.map(\.id)); try scene.validate(requireSources: false) }
    }
    private enum CodingKeys: String, CodingKey { case channels, profiles, scenes, deck, playlists, history, overlays, ffmpegPath, ffprobePath, loopPlaylist, interruptedRuns }
    public init(from decoder: Decoder) throws {
        let values = try decoder.container(keyedBy: CodingKeys.self)
        channels = try values.decodeIfPresent([Channel].self, forKey: .channels) ?? []
        profiles = try values.decodeIfPresent([EncoderProfile].self, forKey: .profiles) ?? PlatformPresets.defaults()
        scenes = try values.decodeIfPresent([Scene].self, forKey: .scenes) ?? []
        deck = try values.decodeIfPresent([DeckButton].self, forKey: .deck) ?? []
        playlists = try values.decodeIfPresent([String: [PlaylistItem]].self, forKey: .playlists) ?? [:]
        history = try values.decodeIfPresent([LiveHistory].self, forKey: .history) ?? []
        overlays = try values.decodeIfPresent([String].self, forKey: .overlays) ?? []
        ffmpegPath = try values.decodeIfPresent(String.self, forKey: .ffmpegPath) ?? ""
        ffprobePath = try values.decodeIfPresent(String.self, forKey: .ffprobePath) ?? ""
        loopPlaylist = try values.decodeIfPresent(Bool.self, forKey: .loopPlaylist) ?? true
        interruptedRuns = try values.decodeIfPresent([PlaybackCheckpoint].self, forKey: .interruptedRuns) ?? []
    }

}
public enum OrbisError: LocalizedError {
    case invalid(String), process(String)
    public var errorDescription: String? { switch self { case .invalid(let message), .process(let message): return message } }
}
public struct MediaInfo: Sendable {
    public var width: Int
    public var height: Int
    public var frameRate: Double
    public var duration: Double
    public var hasAudio: Bool
    public var isHDR: Bool
    public var pixelFormat: String
    public var codecName: String
    public var formatName: String
    public var frames: Int
    public var hasAlpha: Bool
    public init(width: Int = 1920, height: Int = 1080, frameRate: Double = 30, duration: Double = 0, hasAudio: Bool = false, isHDR: Bool = false, pixelFormat: String = "yuv420p", codecName: String = "", formatName: String = "", frames: Int = 0, hasAlpha: Bool = false) {
        self.width = width; self.height = height; self.frameRate = frameRate; self.duration = duration; self.hasAudio = hasAudio; self.isHDR = isHDR; self.pixelFormat = pixelFormat; self.codecName = codecName
        self.formatName = formatName; self.frames = frames; self.hasAlpha = hasAlpha
    }
}
