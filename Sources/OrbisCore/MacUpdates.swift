import Foundation
#if canImport(FoundationNetworking)
import FoundationNetworking
#endif

public struct MacRelease: Codable, Sendable {
    public let version: String
    public let architecture: String
    public let url: URL
    public init(version: String, architecture: String = "arm64", url: URL) { self.version = version; self.architecture = architecture; self.url = url }
}

public enum MacUpdates {
    public static let defaultFeed = "https://api.github.com/repos/Zaccalos7/lazy-stream-mac/releases/latest"
    private static func components(_ version: String) -> [Int]? {
        let clean = version.hasPrefix("v") ? String(version.dropFirst()) : version
        let fields = clean.split(separator: ".", omittingEmptySubsequences: false)
        guard (1...4).contains(fields.count), fields.allSatisfy({ !$0.isEmpty && $0.allSatisfy { $0.isASCII && $0.isNumber } }) else { return nil }
        let values = fields.compactMap { Int($0) }
        guard values.count == fields.count, values.allSatisfy({ $0 >= 0 }) else { return nil }
        return values + Array(repeating: 0, count: 4 - values.count)
    }
    public static func isNewer(_ version: String, than installed: String) -> Bool {
        guard let next = components(version), let current = components(installed) else { return false }
        return current.lexicographicallyPrecedes(next)
    }
    public static func manifest(_ data: Data) throws -> MacRelease {
        let release = try JSONDecoder().decode(MacRelease.self, from: data)
        guard release.architecture == "arm64", components(release.version) != nil,
              release.url.scheme == "https", release.url.host != nil, release.url.pathExtension.lowercased() == "pkg" else {
            throw OrbisError.invalid("Il manifest deve indicare una versione numerica e un installer macOS arm64 HTTPS.")
        }
        return release
    }
    public static func github(_ data: Data) throws -> MacRelease? {
        struct Release: Decodable {
            struct Asset: Decodable { let name: String; let browser_download_url: URL }
            let tag_name: String
            let draft: Bool
            let prerelease: Bool
            let assets: [Asset]
        }
        let release = try JSONDecoder().decode(Release.self, from: data)
        guard !release.draft, !release.prerelease, components(release.tag_name) != nil,
              let asset = release.assets.first(where: { $0.name.lowercased().hasSuffix(".pkg") && $0.name.lowercased().contains("arm64") && $0.browser_download_url.scheme == "https" && $0.browser_download_url.host != nil && $0.browser_download_url.pathExtension.lowercased() == "pkg" }) else { return nil }
        let version = release.tag_name.hasPrefix("v") ? String(release.tag_name.dropFirst()) : release.tag_name
        return MacRelease(version: version, url: asset.browser_download_url)
    }
    public static func check(feed: String, installed: String) async throws -> MacRelease? {
        let custom = !feed.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty
        guard let url = URL(string: custom ? feed : defaultFeed), url.scheme == "https", url.host != nil else { throw OrbisError.invalid("Inserisci un URL HTTPS del manifest macOS.") }
        var request = URLRequest(url: url); request.timeoutInterval = 15
        request.setValue("OrbisStream-macOS", forHTTPHeaderField: "User-Agent")
        request.setValue("application/json", forHTTPHeaderField: "Accept")
        let (data, response) = try await URLSession.shared.data(for: request)
        guard let http = response as? HTTPURLResponse else { throw OrbisError.process("Risposta aggiornamenti non valida.") }
        if !custom && http.statusCode == 404 { return nil }
        guard http.statusCode == 200 else { throw OrbisError.process("Aggiornamenti non disponibili (HTTP \(http.statusCode)).") }
        let release = try custom ? manifest(data) : github(data)
        guard let release, isNewer(release.version, than: installed) else { return nil }
        return release
    }
}
