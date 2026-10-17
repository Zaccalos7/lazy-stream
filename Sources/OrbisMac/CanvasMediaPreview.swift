import SwiftUI
import AppKit
import AVFoundation
import OrbisCore

private final class PlayerSurface: NSView {
    let playerLayer = AVPlayerLayer()
    override init(frame: NSRect) {
        super.init(frame: frame); wantsLayer = true; layer?.addSublayer(playerLayer)
        playerLayer.videoGravity = .resizeAspect
    }
    required init?(coder: NSCoder) { fatalError("init(coder:) non supportato") }
    override func layout() { super.layout(); playerLayer.frame = bounds }
}
@MainActor private struct PlayerLayerView: NSViewRepresentable {
    let player: AVPlayer
    func makeNSView(context: Context) -> NSView { let view = PlayerSurface(frame: .zero); view.playerLayer.player = player; return view }
    func updateNSView(_ nsView: NSView, context: Context) { (nsView as? PlayerSurface)?.playerLayer.player = player }
}
@MainActor struct AnimatedMediaImage: NSViewRepresentable {
    let image: NSImage
    func makeNSView(context: Context) -> NSImageView {
        let view = NSImageView(); view.imageScaling = .scaleProportionallyUpOrDown; view.animates = true; return view
    }
    func updateNSView(_ view: NSImageView, context: Context) { if view.image !== image { view.image = image } }
}
@MainActor struct CanvasMediaPreview: View {
    @EnvironmentObject private var model: AppModel
    let path: String
    @State private var image: NSImage?
    @State private var player: AVQueuePlayer?
    @State private var looper: AVPlayerLooper?
    @State private var failed = false
    var body: some View {
        Group {
            if let image { AnimatedMediaImage(image: image) }
            else if let player { PlayerLayerView(player: player) }
            else if failed { Image(systemName: "film").foregroundStyle(.secondary) }
            else { Color.black }
        }.allowsHitTesting(false)
        .task(id: path) { await load() }
        .onDisappear { player?.pause(); looper = nil; player = nil }
    }
    private func load() async {
        player?.pause(); looper = nil; player = nil; image = nil; failed = false
        guard !path.isEmpty else { return }
        if MediaFiles.isPicture(path), let bitmap = NSImage(contentsOfFile: path) { image = bitmap; return }
        do {
            let url = try await MediaPreviewService.shared.playable(URL(fileURLWithPath: path), ffmpeg: Executables.resolve("ffmpeg", configured: model.workspace.ffmpegPath), ffprobe: Executables.resolve("ffprobe", configured: model.workspace.ffprobePath))
            try Task.checkCancellation()
            let queue = AVQueuePlayer(); queue.isMuted = true
            looper = AVPlayerLooper(player: queue, templateItem: AVPlayerItem(url: url))
            player = queue; queue.play()
        } catch { if !Task.isCancelled { failed = true } }
    }
}
