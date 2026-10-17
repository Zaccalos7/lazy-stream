import Foundation

public extension Platform {
    static var destinations: [Platform] { [.twitch, .youtube, .kick, .facebook, .tiktok] }
    var ingest: String {
        switch self {
        case .custom: return ""
        case .twitch: return "rtmp://live.twitch.tv/app"
        case .youtube: return "rtmps://a.rtmps.youtube.com/live2"
        case .kick: return "rtmps://fa723fc1b171.global-contribute.live-video.net/app"
        case .facebook: return "rtmps://live-api-s.facebook.com:443/rtmp"
        case .tiktok: return "rtmp://push-rtmp-l11-va01.tiktokcdn.com/stage"
        }
    }
}

public extension EncoderProfile {
    func validateForStreaming() throws {
        try validate()
        guard format == "flv", ["h264_videotoolbox", "libx264"].contains(codec), ["aac", "libmp3lame"].contains(audioCodec) else {
            throw OrbisError.invalid("Per RTMP scegli FLV, H.264 e audio AAC/MP3.")
        }
    }
}

/// The output settings match the Windows presets; Apple uses VideoToolbox for encoding.
public enum PlatformPresets {
    public static func profile(_ platform: Platform, light: Bool = false) -> EncoderProfile {
        var profile = EncoderProfile()
        profile.title = "Apple H.264 · \(platform.title) · \(light ? "Leggero" : "Standard")"
        profile.defaultPlatform = platform
        profile.isDefault = !light
        profile.quality = light ? .light : .balanced
        profile.lowLatency = platform == .twitch
        profile.options = [EncoderOption(key: "profile:v", value: light || platform == .facebook ? "main" : "high")]
        profile.width = light ? 1280 : 1920; profile.height = light ? 720 : 1080
        profile.frameRate = 30
        profile.videoBitrate = light ? 3_000_000 : 6_000_000
        profile.audioBitrate = platform == .twitch || (platform == .kick && !light) ? 160_000 : 128_000
        if platform == .twitch {
            if light { profile.audioBitrate = 96_000 }
            else { profile.width = 0; profile.height = 0; profile.frameRate = 0 }
        }
        if platform == .facebook { profile.videoBitrate = light ? 3_000_000 : 4_500_000 }
        if platform == .tiktok {
            profile.width = light ? 720 : 1080; profile.height = light ? 1280 : 1920
            profile.videoBitrate = light ? 2_500_000 : 4_500_000
        }
        return profile
    }
    public static func defaults() -> [EncoderProfile] {
        Platform.destinations.flatMap { [profile($0), profile($0, light: true)] }
    }
}

public enum LiveReplay {
    public static func starting(at videoID: UUID, in history: LiveHistory, resume: Bool) -> [PlaylistItem] {
        guard let index = history.playlist.firstIndex(where: { $0.id == videoID }) else { return [] }
        var items = Array(history.playlist[index...])
        if !resume { for index in items.indices { items[index].resumeSeconds = 0 } }
        if !items.isEmpty { items[0].enabled = true }
        return items
    }
    /// Resume from the recorded video and time; restart resets every saved seek position.
    public static func playlist(_ history: LiveHistory, resume: Bool, checkpoint: PlaybackCheckpoint? = nil) -> [PlaylistItem] {
        var items = history.playlist
        if resume {
            let index = checkpoint?.index ?? items.firstIndex(where: { $0.id == history.lastVideoID }) ?? 0
            if items.indices.contains(index) {
                items.removeFirst(index)
                if let checkpoint { items[0].resumeSeconds = max(0, checkpoint.position) }
            }
        } else {
            for index in items.indices { items[index].resumeSeconds = 0 }
        }
        return items
    }
}
