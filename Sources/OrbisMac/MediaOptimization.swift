import Foundation
import CryptoKit
import OrbisCore

struct OptimizedMedia: Sendable { let path: String; let hardwareDecoding: Bool }
/// Prepare heavy media outside the live producer; use a proxy only if its pixels cover the tile.
actor MediaOptimization {
    static let shared = MediaOptimization()
    private var queue: [(path: String, ffmpeg: URL, ffprobe: URL)] = []
    private var queued: Set<String> = []
    private var worker: Task<Void, Never>?
    private var hardware: [String: Bool] = [:]
    private func cache(_ path: String) throws -> URL {
        let source = URL(fileURLWithPath: path)
        let attributes = try source.resourceValues(forKeys: [.contentModificationDateKey, .fileSizeKey])
        let identity = "\(path)|\(attributes.fileSize ?? 0)|\(attributes.contentModificationDate?.timeIntervalSince1970 ?? 0)"
        let digest = SHA256.hash(data: Data(identity.utf8)).map { String(format: "%02x", $0) }.joined()
        let folder = try FileManager.default.url(for: .cachesDirectory, in: .userDomainMask, appropriateFor: nil, create: true).appendingPathComponent("OrbisStream/Media")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        return folder.appendingPathComponent(digest + ".mp4")
    }
    func prepare(_ path: String, ffmpeg: URL, ffprobe: URL) {
        guard !MediaFiles.isPicture(path), queued.insert(path).inserted else { return }
        queue.append((path, ffmpeg, ffprobe))
        if worker == nil { worker = Task { [weak self] in guard let self else { return }; await self.work() } }
    }
    private func work() async {
        while !queue.isEmpty {
            let next = queue.removeFirst()
            await make(next.path, ffmpeg: next.ffmpeg, ffprobe: next.ffprobe)
            queued.remove(next.path)
        }
        worker = nil
    }
    private func make(_ path: String, ffmpeg: URL, ffprobe: URL) async {
        guard let info = try? await Probe.media(path, executable: ffprobe),
              max(info.width, info.height) > 1920 || min(info.width, info.height) > 1080,
              let output = try? cache(path), !FileManager.default.fileExists(atPath: output.path) else { return }
        var timings: [Bool: Double] = [:]
        for gpu in [false, true] {
            var args = ["-hide_banner", "-nostdin", "-loglevel", "error"]
            if gpu { args += ["-hwaccel", "videotoolbox"] }
            args += ["-i", path, "-t", "3", "-map", "0:v:0", "-an", "-f", "null", "-"]
            let start = ProcessInfo.processInfo.systemUptime
            if let result = try? await ProcessRunner.run(ffmpeg, arguments: args, timeout: 20), result.status == 0 {
                timings[gpu] = ProcessInfo.processInfo.systemUptime - start
            }
        }
        let gpu = (timings[true] ?? .infinity) < (timings[false] ?? .infinity)
        hardware[path] = gpu
        let width = info.width >= info.height ? 1920 : 1080, height = info.width >= info.height ? 1080 : 1920
        var filter = "scale=\(width):\(height):force_original_aspect_ratio=decrease:force_divisible_by=2"
        if info.isHDR {
            guard let filters = try? await ProcessRunner.run(ffmpeg, arguments: ["-hide_banner", "-filters"]), String(decoding: filters.output, as: UTF8.self).contains("zscale") else { return }
            filter = "zscale=t=linear:npl=100,format=gbrpf32le,zscale=p=bt709,tonemap=tonemap=hable:desat=0,zscale=t=bt709:m=bt709:r=tv," + filter
        }
        let partial = output.deletingPathExtension().appendingPathExtension("partial.mp4")
        defer { try? FileManager.default.removeItem(at: partial) }
        var args = ["-hide_banner", "-nostdin", "-loglevel", "error", "-y"]
        if gpu && !info.isHDR { args += ["-hwaccel", "videotoolbox"] }
        args += ["-i", path, "-map", "0:v:0", "-map", "0:a:0?", "-vf", filter, "-c:v", "h264_videotoolbox", "-allow_sw", "1", "-b:v", "8000000", "-pix_fmt", "yuv420p", "-c:a", "aac", "-b:a", "192000", "-movflags", "+faststart", partial.path]
        if let result = try? await ProcessRunner.run(ffmpeg, arguments: args, timeout: 7200), result.status == 0 {
            try? FileManager.default.moveItem(at: partial, to: output)
        }
    }
    func resolve(_ path: String, tileWidth: Int, tileHeight: Int, ffmpeg: URL, ffprobe: URL) async -> OptimizedMedia {
        if let output = try? cache(path), FileManager.default.fileExists(atPath: output.path),
           let info = try? await Probe.media(output.path, executable: ffprobe), tileWidth <= info.width, tileHeight <= info.height {
            try? FileManager.default.setAttributes([.modificationDate: Date()], ofItemAtPath: output.path)
            return OptimizedMedia(path: output.path, hardwareDecoding: false)
        }
        prepare(path, ffmpeg: ffmpeg, ffprobe: ffprobe)
        return OptimizedMedia(path: path, hardwareDecoding: hardware[path] ?? false)
    }
    func removeUnused() {
        guard let root = try? FileManager.default.url(for: .cachesDirectory, in: .userDomainMask, appropriateFor: nil, create: true).appendingPathComponent("OrbisStream/Media"),
              let urls = try? FileManager.default.contentsOfDirectory(at: root, includingPropertiesForKeys: [.contentModificationDateKey]) else { return }
        let cutoff = Date().addingTimeInterval(-30 * 86400)
        for url in urls where url.pathExtension == "mp4" && !url.lastPathComponent.contains("partial") {
            if let modified = try? url.resourceValues(forKeys: [.contentModificationDateKey]).contentModificationDate, modified < cutoff { try? FileManager.default.removeItem(at: url) }
        }
    }
}
