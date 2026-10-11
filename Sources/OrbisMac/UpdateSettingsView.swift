import SwiftUI
import Foundation
import OrbisCore

/// A native release feed must describe a macOS arm64 package, never a Windows release.
@MainActor struct UpdateSettingsView: View {
    private struct Release: Decodable {
        let version: String
        let architecture: String
        let url: URL
    }
    @AppStorage("updateFeedURL") private var feed = ""
    @State private var checking = false
    @State private var message = ""
    @State private var download: URL?
    var body: some View {
        Section("Aggiornamenti") {
            TextField("URL del manifest macOS", text: $feed)
            HStack {
                Button("Verifica aggiornamenti") { Task { await check() } }.disabled(checking || feed.isEmpty)
                if checking { ProgressView().controlSize(.small) }
                if let download { Link("Scarica installer Apple Silicon", destination: download) }
            }
            if !message.isEmpty { Text(message).font(.caption) }
        }
    }
    private func check() async {
        checking = true; download = nil; message = ""
        defer { checking = false }
        do {
            guard let url = URL(string: feed), url.scheme == "https", url.host != nil else {
                throw OrbisError.invalid("Inserisci un URL HTTPS del manifest macOS.")
            }
            var request = URLRequest(url: url); request.timeoutInterval = 15
            let (data, response) = try await URLSession.shared.data(for: request)
            guard let http = response as? HTTPURLResponse, http.statusCode == 200 else {
                throw OrbisError.invalid("Il manifest non è disponibile.")
            }
            let release = try JSONDecoder().decode(Release.self, from: data)
            guard release.architecture == "arm64", release.url.scheme == "https", release.url.host != nil,
                  release.url.pathExtension.lowercased() == "pkg",
                  !release.version.isEmpty, release.version.split(separator: ".", omittingEmptySubsequences: false).allSatisfy({ Int($0) != nil }) else {
                throw OrbisError.invalid("Il manifest deve indicare un installer macOS arm64 e una versione numerica.")
            }
            let installed = Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "3.0.0"
            if release.version.compare(installed, options: .numeric) == .orderedDescending {
                download = release.url; message = "Versione disponibile: \(release.version)"
            } else { message = "Nessun aggiornamento disponibile." }
        } catch { message = error.localizedDescription }
    }
}
