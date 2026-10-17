import SwiftUI
import AppKit
import AVKit
import OrbisCore

struct MediaSelection: Identifiable {
    var id: String { path }
    let path: String
}

@MainActor struct MediaPreviewPanel: View {
    @EnvironmentObject private var model: AppModel
    @Environment(\.dismiss) private var dismiss
    let path: String
    @State private var image: NSImage?
    @State private var player: AVPlayer?
    @State private var info: MediaInfo?
    @State private var error: String?
    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            HStack {
                Text(MediaFiles.displayName(path)).font(.title2.bold()).lineLimit(1)
                Spacer(); Button("Chiudi") { dismiss() }
            }
            ZStack {
                Color.black
                if let image { AnimatedMediaImage(image: image) }
                else if let player { VideoPlayer(player: player) }
                else if let error { Text(error).foregroundStyle(.white).padding() }
                else { ProgressView().tint(.white) }
            }.clipShape(RoundedRectangle(cornerRadius: 12))
            if let info {
                Text("\(info.width)×\(info.height) · \(info.frameRate.formatted()) fps · \(clock(info.duration)) · \(info.codecName)")
                    .font(.caption).foregroundStyle(.secondary)
            }
            HStack {
                Text(path).font(.caption).foregroundStyle(.secondary).textSelection(.enabled).lineLimit(2)
                Spacer(); Button("Mostra nel Finder") { NSWorkspace.shared.selectFile(path, inFileViewerRootedAtPath: "") }
            }
        }.padding(28).frame(width: 800, height: 620)
        .task(id: path) {
            image = nil; player?.pause(); player = nil; error = nil
            if MediaFiles.isPicture(path), let value = NSImage(contentsOfFile: path) { image = value }
            do {
                let ffmpeg = try Executables.resolve("ffmpeg", configured: model.workspace.ffmpegPath)
                let ffprobe = try Executables.resolve("ffprobe", configured: model.workspace.ffprobePath)
                info = try await Probe.media(path, executable: ffprobe)
                if image == nil {
                    let url = try await MediaPreviewService.shared.playable(URL(fileURLWithPath: path), ffmpeg: ffmpeg, ffprobe: ffprobe)
                    try Task.checkCancellation(); player = AVPlayer(url: url)
                }
            } catch { if !Task.isCancelled { self.error = error.localizedDescription } }
        }.onDisappear { player?.pause() }
    }
}
