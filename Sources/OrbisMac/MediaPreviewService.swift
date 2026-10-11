import Foundation
import AVFoundation
import CryptoKit
import OrbisCore

actor MediaPreviewService {
    static let shared = MediaPreviewService()
    private var pending: [String: Task<URL, Error>] = [:]
    private func cacheURL(_ source: String, suffix: String) throws -> URL {
        let folder = try FileManager.default.url(for: .cachesDirectory, in: .userDomainMask, appropriateFor: nil, create: true).appendingPathComponent("OrbisStream/Previews")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        let modified = (try? URL(fileURLWithPath: source).resourceValues(forKeys: [.contentModificationDateKey]).contentModificationDate?.timeIntervalSince1970) ?? 0
        let hash = SHA256.hash(data: Data("\(source)|\(modified)|\(suffix)".utf8)).map { String(format: "%02x", $0) }.joined()
        return folder.appendingPathComponent(hash + suffix)
    }
    func snapshot(_ item: SceneItem, ffmpeg: URL, ffprobe: URL) async throws -> Data {
        var args = ["-hide_banner", "-nostdin", "-loglevel", "error"]
        switch item.kind {
        case .file, .overlay:
            if item.kind == .overlay, let info = try? await Probe.media(item.target, executable: ffprobe) {
                if info.codecName == "vp9" { args += ["-c:v", "libvpx-vp9"] }
                else if info.codecName == "vp8" { args += ["-c:v", "libvpx"] }
            }
            args += ["-i", item.target]
        case .screen, .camera: args += ["-f", "avfoundation", "-framerate", "30", "-i", "\(item.target):none"]
        case .microphone: throw OrbisError.invalid("Il microfono non ha un’anteprima video.")
        }
        args += ["-frames:v", "1", "-vf", "scale=640:-2", "-an", "-c:v", "png", "-f", "image2pipe", "pipe:1"]
        let result = try await ProcessRunner.run(ffmpeg, arguments: args, timeout: 10)
        guard result.status == 0, !result.output.isEmpty else {
            throw OrbisError.process("Anteprima non disponibile: \(String(decoding: result.error.suffix(2000), as: UTF8.self))")
        }
        return result.output
    }
    func playable(_ source: URL, ffmpeg: URL, ffprobe: URL) async throws -> URL {
        let asset = AVURLAsset(url: source)
        if (try? await asset.load(.isPlayable)) == true { return source }
        let output = try cacheURL(source.path, suffix: ".mp4")
        if FileManager.default.fileExists(atPath: output.path) { return output }
        if let task = pending[output.path] { return try await task.value }
        let task = Task<URL, Error> {
            let partial = output.deletingPathExtension().appendingPathExtension("partial.mp4")
            defer { try? FileManager.default.removeItem(at: partial) }
            let info = try await Probe.media(source.path, executable: ffprobe)
            var filter = "scale=960:-2"
            if info.isHDR {
                let filters = try await ProcessRunner.run(ffmpeg, arguments: ["-hide_banner", "-filters"])
                if String(decoding: filters.output, as: UTF8.self).contains("zscale") {
                    filter = "zscale=t=linear:npl=100,format=gbrpf32le,zscale=p=bt709,tonemap=tonemap=hable:desat=0,zscale=t=bt709:m=bt709:r=tv," + filter
                }
            }
            let result = try await ProcessRunner.run(ffmpeg, arguments: ["-hide_banner", "-nostdin", "-loglevel", "error", "-y", "-i", source.path, "-map", "0:v:0", "-map", "0:a:0?", "-vf", filter, "-c:v", "h264_videotoolbox", "-allow_sw", "1", "-b:v", "2000000", "-pix_fmt", "yuv420p", "-c:a", "aac", "-b:a", "96000", "-movflags", "+faststart", partial.path], timeout: 600)
            guard result.status == 0 else { throw OrbisError.process("Conversione anteprima fallita: \(String(decoding: result.error.suffix(2000), as: UTF8.self))") }
            try FileManager.default.moveItem(at: partial, to: output); return output
        }
        pending[output.path] = task
        defer { pending.removeValue(forKey: output.path) }
        return try await task.value
    }
}
