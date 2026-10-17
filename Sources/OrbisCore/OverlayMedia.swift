import Foundation

public struct OverlayMedia: Equatable, Sendable {
    public var loop: Bool
    public var decoder: String?
    public var nativeWebPLoop: Bool
    public init(loop: Bool = false, decoder: String? = nil, nativeWebPLoop: Bool = false) {
        self.loop = loop; self.decoder = decoder; self.nativeWebPLoop = nativeWebPLoop
    }
    public static func inspect(_ info: MediaInfo?, path: String) -> OverlayMedia {
        if info?.codecName == "webp_anim" || info?.formatName == "webp_anim" || WebPAnimation.isAnimated(path) {
            return OverlayMedia(loop: true, nativeWebPLoop: true)
        }
        let format = info?.formatName ?? ""
        if format == "image2" || format.hasSuffix("_pipe") ||
            (format == "gif" && (info?.frames ?? 0) <= 1 && (info?.duration ?? 0) <= 0.1) || MediaFiles.isStill(path) {
            return OverlayMedia()
        }
        let codec = info?.codecName ?? ""
        let webm = URL(fileURLWithPath: path).pathExtension.lowercased() == "webm"
        let decoder: String? = (info?.hasAlpha == true || webm) ? (codec == "vp8" ? "libvpx" : codec == "vp9" || (codec.isEmpty && webm) ? "libvpx-vp9" : nil) : nil
        return OverlayMedia(loop: true, decoder: decoder)
    }
}

public enum WebPAnimation {
    public static func isAnimated(_ path: String) -> Bool {
        guard URL(fileURLWithPath: path).pathExtension.lowercased() == "webp",
              let handle = try? FileHandle(forReadingFrom: URL(fileURLWithPath: path)) else { return false }
        defer { try? handle.close() }
        return (try? loopCountOffset(handle)) != nil
    }
    public static func loopCountOffset(_ handle: FileHandle) throws -> UInt64 {
        try handle.seek(toOffset: 0)
        let header = try handle.read(upToCount: 12) ?? Data()
        guard header.count == 12, header.prefix(4) == Data("RIFF".utf8), header.suffix(4) == Data("WEBP".utf8) else {
            throw OrbisError.invalid("Header WebP non valido.")
        }
        let length = try handle.seekToEnd()
        var offset: UInt64 = 12
        while offset + 8 <= length {
            try handle.seek(toOffset: offset)
            let chunk = try handle.read(upToCount: 8) ?? Data()
            guard chunk.count == 8 else { break }
            let size = UInt64(chunk[4]) | UInt64(chunk[5]) << 8 | UInt64(chunk[6]) << 16 | UInt64(chunk[7]) << 24
            guard size <= length - offset - 8 else { break }
            if chunk.prefix(4) == Data("ANIM".utf8), size == 6 { return offset + 12 }
            offset += 8 + size + (size & 1)
        }
        throw OrbisError.invalid("Il WebP non contiene un’animazione valida.")
    }
}

/// Only the ANIM loop count changes; encoded pixels, delays and alpha are preserved.
public final class WebPInputs {
    private var directory: URL?
    private var copies: [String: String] = [:]
    public init() {}
    public func prepare(_ scene: Scene) throws -> Scene {
        var result = scene
        for index in result.items.indices {
            let item = result.items[index]
            guard (item.kind == .overlay || item.kind == .file), WebPAnimation.isAnimated(item.target) else { continue }
            result.items[index].target = try loopable(item.target)
        }
        return result
    }
    public func loopable(_ path: String) throws -> String {
        let url = URL(fileURLWithPath: path)
        let attributes = try url.resourceValues(forKeys: [.fileSizeKey, .contentModificationDateKey])
        let identity = "\(path)|\(attributes.fileSize ?? 0)|\(attributes.contentModificationDate?.timeIntervalSince1970 ?? 0)"
        if let copy = copies[identity] { return copy }
        let source = try FileHandle(forReadingFrom: url)
        defer { try? source.close() }
        let offset = try WebPAnimation.loopCountOffset(source)
        try source.seek(toOffset: offset)
        if try source.read(upToCount: 2) == Data([0, 0]) { return path }
        if directory == nil {
            let folder = FileManager.default.temporaryDirectory.appendingPathComponent("orbis-webp-\(UUID().uuidString)")
            try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true, attributes: [.posixPermissions: 0o700])
            directory = folder
        }
        let output = directory!.appendingPathComponent(UUID().uuidString + ".webp")
        try FileManager.default.copyItem(at: url, to: output)
        do {
            let target = try FileHandle(forWritingTo: output); defer { try? target.close() }
            try target.seek(toOffset: offset); try target.write(contentsOf: Data([0, 0]))
            try FileManager.default.setAttributes([.posixPermissions: 0o600], ofItemAtPath: output.path)
        } catch { try? FileManager.default.removeItem(at: output); throw error }
        copies[identity] = output.path
        return output.path
    }
    public func cleanup() { if let directory { try? FileManager.default.removeItem(at: directory) }; directory = nil; copies.removeAll() }
    deinit { cleanup() }
}
