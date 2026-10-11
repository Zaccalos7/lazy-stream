import SwiftUI
import AppKit
import AVFoundation

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
@MainActor struct CanvasMediaPreview: View {
    let path: String
    @State private var player: AVQueuePlayer?
    @State private var looper: AVPlayerLooper?
    var body: some View {
        Group {
            if let image = NSImage(contentsOfFile: path) { Image(nsImage: image).resizable().scaledToFit() }
            else if let player { PlayerLayerView(player: player) }
            else { Color.black }
        }.allowsHitTesting(false)
        .onAppear { load() }.onChange(of: path) { load() }.onDisappear { player?.pause(); looper = nil; player = nil }
    }
    private func load() {
        player?.pause(); looper = nil; player = nil
        guard !path.isEmpty, NSImage(contentsOfFile: path) == nil else { return }
        let queue = AVQueuePlayer(); queue.isMuted = true
        looper = AVPlayerLooper(player: queue, templateItem: AVPlayerItem(url: URL(fileURLWithPath: path)))
        player = queue; queue.play()
    }
}
