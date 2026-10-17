import SwiftUI
import AppKit
import OrbisCore

@MainActor struct CompositionPreview: View {
    @EnvironmentObject private var model: AppModel
    let scene: OrbisCore.Scene
    @State private var image: NSImage?
    @State private var error: String?
    private var identity: Data { (try? JSONEncoder().encode(scene)) ?? Data() }
    var body: some View {
        ZStack {
            Color.black
            if let image { Image(nsImage: image).resizable().scaledToFit() }
            else if let error { Text(error).font(.caption).foregroundStyle(.white).padding() }
            else { ProgressView().tint(.white) }
        }.aspectRatio(CGFloat(scene.width) / CGFloat(scene.height), contentMode: .fit)
            .clipShape(RoundedRectangle(cornerRadius: 12))
            .task(id: identity) {
                image = nil; error = nil
                do {
                    try await Task.sleep(for: .milliseconds(300))
                    try await AppleCapture.authorize(scene.items.filter { $0.kind != .microphone })
                    let ffmpeg = try Executables.resolve("ffmpeg", configured: model.workspace.ffmpegPath)
                    let ffprobe = try Executables.resolve("ffprobe", configured: model.workspace.ffprobePath)
                    while !Task.isCancelled {
                        let data = try await MediaPreviewService.shared.composition(scene, ffmpeg: ffmpeg, ffprobe: ffprobe)
                        try Task.checkCancellation(); image = NSImage(data: data)
                        try await Task.sleep(for: .seconds(1))
                    }
                } catch is CancellationError { }
                catch { if !Task.isCancelled { self.error = error.localizedDescription } }
            }
    }
}
