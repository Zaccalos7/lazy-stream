import Foundation
#if canImport(FoundationNetworking)
import FoundationNetworking
#endif

public enum AirState: String, Sendable { case onAir, offAir, ended, unverified }
public struct AirResult: Sendable { public var state: AirState; public var videoID: String? }
/// Best-effort compatibility with the public page check of the original app.
/// Only the primary player's JSON is interpreted; HTTP errors/consent pages are unknown.
public enum YouTubeAir {
    public static func check(name: String, previousVideo: String?) async -> AirResult {
        let clean = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !clean.isEmpty else { return AirResult(state: .unverified, videoID: nil) }
        do {
            var candidate: String?
            if clean.count == 11 && !clean.hasPrefix("@") && clean.allSatisfy({ $0.isLetter || $0.isNumber || "_-".contains($0) }) { candidate = clean }
            else {
                let allowed = CharacterSet.alphanumerics.union(CharacterSet(charactersIn: "@_-"))
                guard let encoded = clean.addingPercentEncoding(withAllowedCharacters: allowed) else { return AirResult(state: .unverified, videoID: nil) }
                let path = clean.hasPrefix("UC") ? "channel/\(encoded)" : clean.hasPrefix("@") ? encoded : "@\(encoded)"
                let html = try await page("https://www.youtube.com/\(path)/streams")
                guard let data = jsonVariable("ytInitialData", html: html) else { return AirResult(state: .unverified, videoID: nil) }
                candidate = candidateVideo(data)
            }
            if let candidate {
                let result = playerState(try await page("https://www.youtube.com/watch?v=\(candidate)"))
                if result == .onAir { return AirResult(state: .onAir, videoID: candidate) }
                if result == .unverified { return AirResult(state: .unverified, videoID: nil) }
            }
            if let previousVideo {
                let previous = playerState(try await page("https://www.youtube.com/watch?v=\(previousVideo)"))
                if previous == .ended { return AirResult(state: .ended, videoID: previousVideo) }
                if previous == .unverified { return AirResult(state: .unverified, videoID: previousVideo) }
            }
            return AirResult(state: .offAir, videoID: nil)
        } catch { return AirResult(state: .unverified, videoID: previousVideo) }
    }
    public static func playerState(_ html: String) -> AirState {
        guard let json = jsonVariable("ytInitialPlayerResponse", html: html),
              let data = json.data(using: .utf8), let root = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
              let details = root["videoDetails"] as? [String: Any] else { return .unverified }
        let microformat = (root["microformat"] as? [String: Any])?["playerMicroformatRenderer"] as? [String: Any]
        let broadcast = microformat?["liveBroadcastDetails"] as? [String: Any]
        if details["isLive"] as? Bool == true || broadcast?["isLiveNow"] as? Bool == true { return .onAir }
        if details["isLiveContent"] as? Bool == true && (broadcast?["isLiveNow"] as? Bool == false || broadcast?["endTimestamp"] != nil) { return .ended }
        if details["isLiveContent"] as? Bool == false { return .offAir }
        return .unverified
    }
    public static func candidateVideo(_ json: String) -> String? {
        for pattern in [#""contentId"\s*:\s*"([A-Za-z0-9_-]{11})"\s*,\s*"contentType"\s*:\s*"LOCKUP_CONTENT_TYPE_VIDEO""#,
                        #""videoId"\s*:\s*"([A-Za-z0-9_-]{11})"\s*,\s*"thumbnail""#] {
            guard let regex = try? NSRegularExpression(pattern: pattern), let match = regex.firstMatch(in: json, range: NSRange(json.startIndex..., in: json)), let range = Range(match.range(at: 1), in: json) else { continue }
            return String(json[range])
        }
        return nil
    }
    public static func jsonVariable(_ name: String, html: String) -> String? {
        guard let nameRange = html.range(of: name), let brace = html[nameRange.upperBound...].firstIndex(of: "{") else { return nil }
        var depth = 0; var quoted = false; var escaped = false
        for index in html[brace...].indices {
            let char = html[index]
            if quoted {
                if escaped { escaped = false }
                else if char == "\\" { escaped = true }
                else if char == "\"" { quoted = false }
            } else {
                if char == "\"" { quoted = true }
                else if char == "{" { depth += 1 }
                else if char == "}" { depth -= 1; if depth == 0 { return String(html[brace...index]) } }
            }
        }
        return nil
    }
    private static func page(_ address: String) async throws -> String {
        var request = URLRequest(url: URL(string: address)!); request.timeoutInterval = 15
        request.setValue("Mozilla/5.0", forHTTPHeaderField: "User-Agent")
        let (data, response) = try await URLSession.shared.data(for: request)
        guard let response = response as? HTTPURLResponse, response.statusCode == 200 else { throw OrbisError.process("Pagina YouTube non disponibile.") }
        return String(decoding: data, as: UTF8.self)
    }
}
