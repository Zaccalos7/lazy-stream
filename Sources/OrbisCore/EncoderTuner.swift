import Foundation

public actor EncoderTuner {
    public static let shared = EncoderTuner()
    private var cache: [String: EncoderQuality] = [:]
    public func resolve(profile: EncoderProfile, executable: URL) async throws -> EncoderQuality {
        if profile.quality != .auto { return profile.quality }
        let width = profile.width == 0 ? 1920 : profile.width
        let height = profile.height == 0 ? 1080 : profile.height
        let fps = profile.frameRate == 0 ? 30 : profile.frameRate
        let key = "\(executable.path)|\(profile.codec)|\(width)|\(height)|\(fps)|\(profile.videoBitrate)|\(profile.pixelFormat)"
        if let cached = cache[key] { return cached }
        var arguments = ["-hide_banner", "-nostdin", "-loglevel", "error", "-f", "lavfi", "-i", "testsrc2=size=\(width)x\(height):rate=\(FFmpegCommand.number(fps))",
                         "-t", "2", "-threads", "2", "-c:v", profile.codec, "-pix_fmt", profile.pixelFormat, "-b:v", String(profile.videoBitrate)]
        if profile.codec.hasSuffix("_videotoolbox") { arguments += ["-allow_sw", "0", "-realtime", "1", "-prio_speed", "0"] }
        else if profile.codec == "libx264" || profile.codec == "libx265" { arguments += ["-preset", profile.preset.isEmpty ? "veryfast" : profile.preset] }
        arguments += ["-f", "null", "-"]
        let start = ProcessInfo.processInfo.systemUptime
        let result = try await ProcessRunner.run(executable, arguments: arguments, timeout: 20)
        guard result.status == 0 else { throw OrbisError.process("L’encoder \(profile.codec) non supera la verifica iniziale: \(String(decoding: result.error.suffix(2000), as: UTF8.self))") }
        let elapsed = ProcessInfo.processInfo.systemUptime - start
        let selected: EncoderQuality = elapsed < 1.0 ? .balanced : .light
        cache[key] = selected
        return selected
    }
}
