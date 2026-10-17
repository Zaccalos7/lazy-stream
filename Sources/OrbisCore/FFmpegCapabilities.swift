import Foundation

public struct FFmpegCapabilities: Sendable {
    public var encoders: Set<String>
    public var decoders: Set<String>
    public var animatedWebP: Bool
    public init(encoders: Set<String> = [], decoders: Set<String> = [], animatedWebP: Bool = false) {
        self.encoders = encoders; self.decoders = decoders; self.animatedWebP = animatedWebP
    }
    public static func codecNames(_ text: String) -> Set<String> {
        Set(text.components(separatedBy: .newlines).compactMap { line in
            let fields = line.split(whereSeparator: \.isWhitespace)
            guard fields.count >= 2, fields[0].count == 6, let first = fields[0].first, "VAS".contains(first), fields[1] != "=" else { return nil }
            return String(fields[1])
        })
    }
    public static func inspect(_ executable: URL) async throws -> Self {
        async let encoders = ProcessRunner.run(executable, arguments: ["-hide_banner", "-encoders"])
        async let decoders = ProcessRunner.run(executable, arguments: ["-hide_banner", "-decoders"])
        async let demuxers = ProcessRunner.run(executable, arguments: ["-hide_banner", "-demuxers"])
        let result = try await (encoders, decoders, demuxers)
        guard result.0.status == 0, result.1.status == 0 else { throw OrbisError.process("FFmpeg non espone i codec disponibili.") }
        return Self(encoders: codecNames(String(decoding: result.0.output, as: UTF8.self)), decoders: codecNames(String(decoding: result.1.output, as: UTF8.self)), animatedWebP: String(decoding: result.2.output, as: UTF8.self).contains("webp_anim"))
    }
}
