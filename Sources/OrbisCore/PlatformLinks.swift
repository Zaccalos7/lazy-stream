import Foundation

public enum PlatformLinks {
    public static func publicURL(_ channel: Channel, onAirVideo: String? = nil) -> URL? {
        func videoID(_ value: String) -> Bool { value.count == 11 && value.allSatisfy { $0.isLetter || $0.isNumber || "_-".contains($0) } }
        if channel.platform == .youtube, let onAirVideo, videoID(onAirVideo) {
            return URL(string: "https://www.youtube.com/watch?v=\(onAirVideo)")
        }
        let name = channel.streamName.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty else { return nil }
        if let full = URL(string: name), let host = full.host?.lowercased(), full.scheme == "https" {
            func under(_ domain: String) -> Bool { host == domain || host.hasSuffix("." + domain) }
            let valid: Bool
            switch channel.platform {
            case .youtube: valid = under("youtube.com") || host == "youtu.be"
            case .twitch: valid = under("twitch.tv")
            case .kick: valid = under("kick.com")
            case .facebook: valid = under("facebook.com") || host == "fb.watch"
            case .tiktok: valid = under("tiktok.com")
            case .custom: valid = false
            }
            return valid ? full : nil
        }
        guard !name.contains("://"), !name.contains("/"), !name.contains("?"), !name.contains("#") else { return nil }
        let allowed = CharacterSet.urlPathAllowed.subtracting(CharacterSet(charactersIn: "/?#"))
        let bare = name.hasPrefix("@") ? String(name.dropFirst()) : name
        guard let escaped = bare.addingPercentEncoding(withAllowedCharacters: allowed), !escaped.isEmpty else { return nil }
        switch channel.platform {
        case .youtube:
            if videoID(name), !name.hasPrefix("@") { return URL(string: "https://www.youtube.com/watch?v=\(escaped)") }
            if name.hasPrefix("UC"), name.count == 24 { return URL(string: "https://www.youtube.com/channel/\(escaped)/live") }
            return URL(string: "https://www.youtube.com/@\(escaped)/live")
        case .twitch: return URL(string: "https://www.twitch.tv/\(escaped)")
        case .kick: return URL(string: "https://kick.com/\(escaped)")
        case .facebook: return URL(string: "https://www.facebook.com/\(escaped)/live")
        case .tiktok: return URL(string: "https://www.tiktok.com/@\(escaped)/live")
        case .custom: return nil
        }
    }
}
