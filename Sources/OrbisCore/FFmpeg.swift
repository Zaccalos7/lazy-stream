import Foundation
#if canImport(Darwin)
import Darwin
#else
import Glibc
#endif

public enum MediaFiles {
    public static let videos: Set<String> = ["mp4", "mov", "mkv", "avi", "webm", "m4v", "ts", "flv"]
    public static let stills: Set<String> = ["png", "jpg", "jpeg", "webp", "bmp"]
    public static func isStill(_ path: String) -> Bool { stills.contains(URL(fileURLWithPath: path).pathExtension.lowercased()) && !WebPAnimation.isAnimated(path) }
    public static func isPicture(_ path: String) -> Bool { stills.contains(URL(fileURLWithPath: path).pathExtension.lowercased()) || ["gif", "apng"].contains(URL(fileURLWithPath: path).pathExtension.lowercased()) }
    public static func playlist(folder: String) throws -> [PlaylistItem] {
        try FileManager.default.contentsOfDirectory(at: URL(fileURLWithPath: folder), includingPropertiesForKeys: [.isRegularFileKey])
            .filter { videos.contains($0.pathExtension.lowercased()) && (try? $0.resourceValues(forKeys: [.isRegularFileKey]).isRegularFile) == true }
            .sorted { $0.lastPathComponent.localizedStandardCompare($1.lastPathComponent) == .orderedAscending }
            .map { PlaylistItem(path: $0.path) }
    }
}
public enum Executables {
    public static func resolve(_ name: String, configured: String = "") throws -> URL {
        let fm = FileManager.default
        if !configured.isEmpty {
            guard fm.isExecutableFile(atPath: configured) else { throw OrbisError.invalid("Eseguibile non valido: \(configured)") }
            return URL(fileURLWithPath: configured)
        }
        let bundled = Bundle.main.bundleURL.appendingPathComponent("Contents/Helpers/\(name)").path
        let paths = [bundled, "/opt/homebrew/bin/\(name)"] + (ProcessInfo.processInfo.environment["PATH"] ?? "").split(separator: ":").map { "\($0)/\(name)" }
        guard let path = paths.first(where: { fm.isExecutableFile(atPath: $0) }) else { throw OrbisError.invalid("\(name) non trovato. Installa FFmpeg o seleziona il binario nelle impostazioni.") }
        return URL(fileURLWithPath: path)
    }
}
public struct ProcessResult: Sendable { public var status: Int32; public var output: Data; public var error: Data }
public enum ProcessRunner {
    /// Both pipes are drained concurrently, including on failure. No shell interpolation.
    public static func run(_ executable: URL, arguments: [String], timeout: TimeInterval = 30) async throws -> ProcessResult {
        let control = ProcessControl()
        return try await withTaskCancellationHandler(operation: {
            try await withCheckedThrowingContinuation { continuation in
                DispatchQueue.global(qos: .userInitiated).async {
                    let process = Process(); let stdout = Pipe(); let stderr = Pipe()
                    process.executableURL = executable; process.arguments = arguments
                    process.standardOutput = stdout; process.standardError = stderr; process.standardInput = FileHandle.nullDevice
                    do { try control.launch(process) } catch { continuation.resume(throwing: error); return }
                    let output = DataBox(); let errors = DataBox(); let group = DispatchGroup()
                    group.enter(); DispatchQueue.global().async { output.set(stdout.fileHandleForReading.readDataToEndOfFile()); group.leave() }
                    group.enter(); DispatchQueue.global().async { errors.set(stderr.fileHandleForReading.readDataToEndOfFile()); group.leave() }
                    let deadline = DispatchWorkItem { control.terminate() }
                    DispatchQueue.global().asyncAfter(deadline: .now() + timeout, execute: deadline)
                    process.waitUntilExit(); deadline.cancel(); group.wait()
                    if control.isCancelled { continuation.resume(throwing: CancellationError()) }
                    else { continuation.resume(returning: ProcessResult(status: process.terminationStatus, output: output.get(), error: errors.get())) }
                }
            }
        }, onCancel: { control.cancel() })
    }
}
private final class ProcessControl: @unchecked Sendable {
    private let lock = NSLock()
    private var process: Process?
    private var cancelled = false
    var isCancelled: Bool { lock.lock(); defer { lock.unlock() }; return cancelled }
    func launch(_ process: Process) throws {
        lock.lock(); defer { lock.unlock() }
        guard !cancelled else { throw CancellationError() }
        try process.run(); self.process = process
    }
    func cancel() { lock.lock(); cancelled = true; lock.unlock(); terminate() }
    func terminate() {
        lock.lock(); let running = process; lock.unlock()
        guard let running, running.isRunning else { return }
        running.terminate()
        DispatchQueue.global().asyncAfter(deadline: .now() + 2) {
            if running.isRunning { _ = kill(running.processIdentifier, SIGKILL) }
        }
    }
}

private final class DataBox: @unchecked Sendable {
    private let lock = NSLock(); private var data = Data()
    func set(_ value: Data) { lock.lock(); data = value; lock.unlock() }
    func get() -> Data { lock.lock(); defer { lock.unlock() }; return data }
}
public enum Probe {
    public static func media(_ path: String, executable: URL) async throws -> MediaInfo {
        let result = try await ProcessRunner.run(executable, arguments: ["-v", "error", "-show_streams", "-show_format", "-of", "json", path])
        guard result.status == 0, let root = try JSONSerialization.jsonObject(with: result.output) as? [String: Any] else { throw OrbisError.process("Impossibile analizzare \(URL(fileURLWithPath: path).lastPathComponent).") }
        let streams = root["streams"] as? [[String: Any]] ?? []
        guard let video = streams.first(where: { $0["codec_type"] as? String == "video" }) else { throw OrbisError.invalid("Il file non contiene un flusso video.") }
        let rate = (video["avg_frame_rate"] as? String ?? "30/1").split(separator: "/").compactMap { Double($0) }
        let fps = rate.count == 2 && rate[1] > 0 ? rate[0] / rate[1] : 30
        let format = root["format"] as? [String: Any] ?? [:]
        return MediaInfo(width: video["width"] as? Int ?? 1920, height: video["height"] as? Int ?? 1080, frameRate: fps > 0 ? fps : 30,
                         duration: Double(format["duration"] as? String ?? video["duration"] as? String ?? "0") ?? 0, hasAudio: streams.contains { $0["codec_type"] as? String == "audio" }, isHDR: ["smpte2084", "arib-std-b67"].contains(video["color_transfer"] as? String ?? ""), pixelFormat: video["pix_fmt"] as? String ?? "yuv420p", codecName: video["codec_name"] as? String ?? "",
                         formatName: format["format_name"] as? String ?? "", frames: Int(video["nb_frames"] as? String ?? "0") ?? 0,
                         hasAlpha: (video["pix_fmt"] as? String ?? "").contains("a") || ((video["tags"] as? [String: Any])?["alpha_mode"] as? String == "1"))
    }
}
public struct EncodingPlan: Sendable {
    public var arguments: [String]
    public var width: Int
    public var height: Int
    public var frameRate: Double
}
public enum FFmpegCommand {
    public static func build(scene: Scene, profile: EncoderProfile, platform: Platform, media: [UUID: MediaInfo],
                             preview: String?, destination: String = "pipe:1", resume: Double = 0, hardwareDecoding: Set<UUID> = []) throws -> EncodingPlan {
        try profile.validate(); try scene.validate()
        if profile.format == "flv" && !["h264_videotoolbox", "libx264"].contains(profile.codec) { throw OrbisError.invalid("La diretta FLV richiede H.264. HEVC è disponibile per altri formati.") }
        if profile.format == "flv" && !["aac", "libmp3lame"].contains(profile.audioCodec) { throw OrbisError.invalid("Per FLV scegli AAC o MP3.") }
        let first = scene.items.first(where: { $0.kind == .file }).flatMap { media[$0.id] }
        var width = profile.width == 0 ? scene.width : profile.width
        var height = profile.height == 0 ? scene.height : profile.height
        if platform == .tiktok && width > height { swap(&width, &height) }
        let fps = profile.frameRate == 0 ? (first?.frameRate ?? 30) : profile.frameRate
        let finiteDurations = scene.items.filter { $0.kind == .file && !MediaFiles.isPicture($0.target) }
            .compactMap { item -> Double? in
                guard let duration = media[item.id]?.duration, duration > 0 else { return nil }
                return max(0.04, duration - resume)
            }
        let duration = finiteDurations.max()
        var args = ["-hide_banner", "-nostdin", "-loglevel", "warning", "-y", "-threads", "2", "-filter_complex_threads", "2"]
        var filters: [String] = []; var audio: [String] = []
        // A synthetic canvas fixes the output timebase across playlist items and scene changes.
        args += ["-f", "lavfi", "-i", "color=c=black:s=\(scene.width)x\(scene.height):r=\(number(fps))" + (duration.map { ":d=" + number($0) } ?? "")]
        var base = "0:v"
        for (offset, item) in scene.items.enumerated() {
            let index = offset + 1
            switch item.kind {
            case .file, .overlay:
                let overlay = OverlayMedia.inspect(media[item.id], path: item.target)
                if item.kind == .overlay, let decoder = overlay.decoder { args += ["-c:v", decoder] }
                if hardwareDecoding.contains(item.id) { args += ["-hwaccel", "videotoolbox"] }
                if (item.kind == .overlay || MediaFiles.isPicture(item.target)) && overlay.loop {
                    if overlay.nativeWebPLoop { args += ["-ignore_loop", "0"] }
                    else { args += ["-stream_loop", "-1"] }
                }
                if item.kind == .file { if resume > 0 { args += ["-ss", number(resume)] } }
                args += ["-i", item.target]
            case .screen, .camera:
                args += ["-thread_queue_size", "512", "-f", "avfoundation", "-framerate", number(fps), "-capture_cursor", "1", "-i", "\(item.target):none"]
            case .microphone:
                args += ["-thread_queue_size", "512", "-f", "avfoundation", "-i", "none:\(item.target)"]
            }
            if item.kind != .microphone {
                let tile = "tile\(index)"; let next = "canvas\(index)"
                let alpha = item.kind == .overlay
                filters.append("[\(index):v]setpts=PTS-STARTPTS,format=\(alpha ? "rgba" : "yuv420p"),scale=\(item.width):\(item.height):force_original_aspect_ratio=decrease,pad=\(item.width):\(item.height):(ow-iw)/2:(oh-ih)/2:color=\(alpha ? "black@0" : "black"),setsar=1[\(tile)]")
                let finite = false // Hold the last frame until the longest source ends.
                filters.append("[\(base)][\(tile)]overlay=x=\(item.x):y=\(item.y):shortest=\(finite ? 1 : 0):eof_action=\(finite ? "endall" : "repeat")[\(next)]")
                base = next
            }
            let hasAudio = item.kind == .microphone || (item.kind == .file && media[item.id]?.hasAudio == true)
            if item.audioEnabled && hasAudio {
                let label = "audio\(index)"
                filters.append("[\(index):a]asetpts=PTS-STARTPTS,aresample=\(platform.sampleRate):async=1,aformat=channel_layouts=stereo,volume=\(number(item.volume / 100)),apad[\(label)]")
                audio.append("[\(label)]")
            }
        }
        if audio.isEmpty {
            args += ["-f", "lavfi", "-i", "anullsrc=r=\(platform.sampleRate):cl=stereo" + (duration.map { ":d=" + number($0) } ?? "")]
            filters.append("[\(scene.items.count + 1):a]anull[mix]")
        } else { filters.append("\(audio.joined())amix=inputs=\(audio.count):duration=longest:normalize=0[mix]") }
        let final = "[\(base)]scale=\(width):\(height):force_original_aspect_ratio=decrease,pad=\(width):\(height):(ow-iw)/2:(oh-ih)/2,setsar=1,fps=\(number(fps))"
        filters.append(final + (preview == nil ? "[out]" : ",split=2[out][previewraw]"))
        if preview != nil { filters.append("[previewraw]fps=4,scale=960:-2[preview]") }
        args += ["-filter_complex", filters.joined(separator: ";"), "-map", "[out]", "-map", "[mix]", "-c:v", profile.codec, "-pix_fmt", profile.pixelFormat,
                 "-b:v", String(profile.videoBitrate), "-maxrate", String(profile.videoBitrate), "-bufsize", String(profile.videoBitrate),
                 "-g", String(Int((fps * Double(profile.keyframeSeconds)).rounded())), "-force_key_frames", "expr:gte(t,n_forced*\(profile.keyframeSeconds))", "-fps_mode", "cfr"]
        if platform.constantRate { args += ["-minrate", String(profile.videoBitrate)] }
        if profile.codec.hasSuffix("_videotoolbox") {
            args += ["-allow_sw", "0", "-realtime", profile.lowLatency ? "1" : "0", "-prio_speed", profile.quality == .light ? "1" : "0"]
            if platform.constantRate { args += ["-constant_bit_rate", "1"] }
            if profile.codec == "h264_videotoolbox" { args += ["-profile:v", "high"] }
        } else if profile.codec == "libx264" || profile.codec == "libx265" {
            let preset = profile.preset.isEmpty ? (profile.quality == .light ? "superfast" : profile.quality == .high ? "faster" : "veryfast") : profile.preset
            args += ["-preset", preset]
            if !profile.tune.isEmpty { args += ["-tune", profile.tune] }
            else if profile.lowLatency { args += ["-tune", "zerolatency"] }
            if platform.constantRate && profile.codec == "libx264" && !profile.options.contains(where: { $0.key == "x264-params" }) { args += ["-x264-params", "nal-hrd=cbr:force-cfr=1"] }
        }
        for option in profile.options { args += ["-" + option.key, option.value] }
        args += ["-c:a", profile.audioCodec, "-b:a", String(profile.audioBitrate), "-ar", String(platform.sampleRate), "-ac", "2", "-f", profile.format]
        if profile.format == "flv" { args += ["-flvflags", "no_duration_filesize", "-flush_packets", "1"] }
        if let duration { args += ["-t", number(duration)] }
        args += [destination]
        if preview != nil {
            args += ["-map", "[preview]", "-c:v", "mjpeg", "-threads", "1", "-q:v", "5", "-f", "image2", "-update", "1", "-atomic_writing", "1"]
            if let duration { args += ["-t", number(duration)] }
            args += [preview!]
        }
        return EncodingPlan(arguments: args, width: width, height: height, frameRate: fps)
    }
    public static func sender(destination: String, policy: StreamingPolicy? = nil) -> [String] {
        var args = ["-hide_banner", "-nostdin", "-loglevel", "warning", "-progress", "pipe:1", "-nostats", "-stats_period", "0.5", "-probesize", "65536", "-analyzeduration", "500000", "-f", "flv"]
        if let policy, policy.senderPacing { args += ["-readrate", "1", "-readrate_initial_burst", number(policy.preroll)] }
        args += ["-i", "pipe:0", "-map", "0", "-c", "copy", "-f", "flv", "-flvflags", "no_duration_filesize", "-flush_packets", "1", "-rw_timeout", String(Int((policy?.connectTimeout ?? 30) * 1_000_000)), destination]
        return args
    }
    public static func number(_ value: Double) -> String { String(format: "%.6f", locale: Locale(identifier: "en_US_POSIX"), value) }
}
public struct StreamProgress: Sendable {
    public var frame = 0
    public var fps = 0.0
    public var bitrate = 0.0
    public var seconds = 0.0
    public var speed = 0.0
    public var bytes: Int64 = 0
    public init() {}
    public mutating func consume(_ line: String) -> Bool {
        let pair = line.split(separator: "=", maxSplits: 1, omittingEmptySubsequences: false)
        guard pair.count == 2 else { return false }
        let key = String(pair[0]); let value = pair[1].trimmingCharacters(in: .whitespacesAndNewlines)
        switch key {
        case "frame": frame = Int(value) ?? frame
        case "fps": fps = Double(value) ?? fps
        case "bitrate": bitrate = Double(value.replacingOccurrences(of: "kbits/s", with: "").trimmingCharacters(in: .whitespaces)) ?? bitrate
        case "out_time_us": seconds = (Double(value) ?? 0) / 1_000_000
        case "speed": speed = Double(value.replacingOccurrences(of: "x", with: "")) ?? speed
        case "total_size": bytes = Int64(value) ?? bytes
        case "progress": return true
        default: break
        }
        return false
    }
}
