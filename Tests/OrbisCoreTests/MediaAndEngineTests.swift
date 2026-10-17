import XCTest
@testable import OrbisCore

final class MediaAndEngineTests: XCTestCase {
    private func tools() throws -> (URL, URL) { (try Executables.resolve("ffmpeg"), try Executables.resolve("ffprobe")) }
    private func temporary() throws -> URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("orbis-test-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true); return url
    }
    private func video(_ path: URL, seconds: Double, ffmpeg: URL) async throws {
        let result = try await ProcessRunner.run(ffmpeg, arguments: ["-hide_banner", "-loglevel", "error", "-f", "lavfi", "-i", "testsrc2=size=128x64:rate=10", "-t", String(seconds), "-c:v", "mpeg4", "-y", path.path])
        XCTAssertEqual(result.status, 0, String(decoding: result.error, as: UTF8.self))
    }
    private func composition(_ path: URL) -> (Scene, EncoderProfile) {
        var scene = Scene(); scene.width = 128; scene.height = 64
        var item = SceneItem(); item.target = path.path; item.width = 128; item.height = 64; scene.items = [item]
        var profile = EncoderProfile(); profile.codec = "libx264"; profile.width = 128; profile.height = 64
        profile.frameRate = 10; profile.preset = "ultrafast"; profile.quality = .light; profile.videoBitrate = 300_000
        return (scene, profile)
    }
    func testOverlayClassificationUsesProbeRatherThanExtension() {
        XCTAssertFalse(OverlayMedia.inspect(MediaInfo(formatName: "png_pipe"), path: "one.png").loop)
        XCTAssertFalse(OverlayMedia.inspect(MediaInfo(duration: 0.05, formatName: "gif", frames: 1), path: "one.gif").loop)
        XCTAssertTrue(OverlayMedia.inspect(MediaInfo(duration: 2, formatName: "gif", frames: 8), path: "many.gif").loop)
        XCTAssertEqual(OverlayMedia.inspect(MediaInfo(codecName: "webp_anim"), path: "animation.webp"), OverlayMedia(loop: true, nativeWebPLoop: true))
        XCTAssertEqual(OverlayMedia.inspect(MediaInfo(codecName: "vp9", hasAlpha: true), path: "alpha.webm").decoder, "libvpx-vp9")
        XCTAssertEqual(OverlayMedia.inspect(MediaInfo(codecName: "vp8", hasAlpha: true), path: "alpha.webm").decoder, "libvpx")
    }
    func testWebPCopyPreservesEveryByteExceptFiniteLoopCount() throws {
        let folder = try temporary(); defer { try? FileManager.default.removeItem(at: folder) }
        let source = folder.appendingPathComponent("finite.webp")
        // Odd-sized unknown chunks are padded before ANIM, as required by RIFF.
        let data = Data("RIFF".utf8) + Data([28,0,0,0]) + Data("WEBPJUNK".utf8) + Data([1,0,0,0,42,0]) + Data("ANIM".utf8) + Data([6,0,0,0,1,2,3,4,2,0])
        try data.write(to: source)
        XCTAssertTrue(WebPAnimation.isAnimated(source.path)); XCTAssertFalse(MediaFiles.isStill(source.path)); XCTAssertTrue(MediaFiles.isPicture(source.path))
        let inputs = WebPInputs(); let copy = try inputs.loopable(source.path)
        XCTAssertNotEqual(copy, source.path); XCTAssertEqual(try inputs.loopable(source.path), copy)
        let modified = try Data(contentsOf: URL(fileURLWithPath: copy))
        var expected = data; expected[expected.count - 2] = 0; expected[expected.count - 1] = 0
        XCTAssertEqual(modified, expected); XCTAssertEqual(try Data(contentsOf: source), data)
        inputs.cleanup(); XCTAssertFalse(FileManager.default.fileExists(atPath: copy))
        try expected.write(to: source)
        XCTAssertEqual(try inputs.loopable(source.path), source.path)
        try Data("RIFFbad".utf8).write(to: source)
        XCTAssertFalse(WebPAnimation.isAnimated(source.path)); XCTAssertThrowsError(try inputs.loopable(source.path))
    }
    func testSnapshotSharesCompositionAndPreservesTransparentPadding() async throws {
        let (ffmpeg, ffprobe) = try tools(), folder = try temporary()
        defer { try? FileManager.default.removeItem(at: folder) }
        let base = folder.appendingPathComponent("blue.mp4"), overlay = folder.appendingPathComponent("alpha.png")
        for (path, filter, codec) in [(base, "color=c=blue:s=128x64:r=10", "mpeg4"), (overlay, "color=c=red@0:s=64x32,format=rgba,drawbox=x=0:y=0:w=32:h=32:color=red@1:t=fill:replace=1", "png")] {
            let result = try await ProcessRunner.run(ffmpeg, arguments: ["-hide_banner", "-loglevel", "error", "-f", "lavfi", "-i", filter, "-frames:v", "1", "-threads", "1", "-c:v", codec, "-y", path.path])
            XCTAssertEqual(result.status, 0, String(decoding: result.error, as: UTF8.self))
        }
        var (scene, profile) = composition(base)
        var image = SceneItem(); image.kind = .overlay; image.target = overlay.path; image.x = 32; image.width = 64; image.height = 64; image.audioEnabled = false
        scene.items.append(image)
        let media = try await [scene.items[0].id: Probe.media(base.path, executable: ffprobe), image.id: Probe.media(overlay.path, executable: ffprobe)]
        let plan = try FFmpegCommand.build(scene: scene, profile: profile, platform: .custom, media: media, preview: nil, snapshot: true)
        XCTAssertFalse(plan.arguments.contains("-loop")); XCTAssertFalse(plan.arguments.contains("-c:a"))
        let result = try await ProcessRunner.run(ffmpeg, arguments: plan.arguments)
        XCTAssertEqual(result.status, 0, String(decoding: result.error, as: UTF8.self))
        let jpeg = folder.appendingPathComponent("preview.jpg"); try result.output.write(to: jpeg)
        let decoded = try await ProcessRunner.run(ffmpeg, arguments: ["-v", "error", "-i", jpeg.path, "-frames:v", "1", "-f", "rawvideo", "-pix_fmt", "rgb24", "pipe:1"])
        XCTAssertEqual(decoded.status, 0)
        guard decoded.output.count == 128 * 64 * 3 else { XCTFail("Snapshot size"); return }
        func pixel(_ x: Int, _ y: Int) -> [UInt8] { Array(decoded.output[((y * 128 + x) * 3)..<((y * 128 + x) * 3 + 3)]) }
        XCTAssertGreaterThan(pixel(40, 30)[0], 180)
        XCTAssertGreaterThan(pixel(85, 30)[2], 180)
        XCTAssertGreaterThan(pixel(40, 5)[2], 180)
    }
    func testNativeAnimatedWebPRepeatsPastItsFiniteLoop() async throws {
        let (ffmpeg, ffprobe) = try tools(), folder = try temporary()
        defer { try? FileManager.default.removeItem(at: folder) }
        let formats = try await ProcessRunner.run(ffmpeg, arguments: ["-hide_banner", "-demuxers"])
        guard String(decoding: formats.output + formats.error, as: UTF8.self).contains("webp_anim") else { throw XCTSkip("Requires FFmpeg 9 webp_anim") }
        let source = folder.appendingPathComponent("base.mp4"), overlay = folder.appendingPathComponent("finite.webp")
        try await video(source, seconds: 1.5, ffmpeg: ffmpeg)
        let created = try await ProcessRunner.run(ffmpeg, arguments: ["-hide_banner", "-loglevel", "error", "-f", "lavfi", "-i", "testsrc2=size=64x32:rate=4", "-t", "0.5", "-c:v", "libwebp_anim", "-loop", "1", "-y", overlay.path])
        XCTAssertEqual(created.status, 0, String(decoding: created.error, as: UTF8.self))
        var (scene, profile) = composition(source)
        var image = SceneItem(); image.kind = .overlay; image.target = overlay.path; image.width = 64; image.height = 32; image.audioEnabled = false; scene.items.append(image)
        let inputs = WebPInputs(); defer { inputs.cleanup() }; scene = try inputs.prepare(scene)
        let repeated = try await ProcessRunner.run(ffmpeg, arguments: ["-v", "error", "-ignore_loop", "0", "-i", scene.items[1].target, "-t", "1.5", "-f", "framemd5", "-"])
        XCTAssertEqual(repeated.status, 0, String(decoding: repeated.error, as: UTF8.self))
        let hashes = String(decoding: repeated.output, as: UTF8.self).split(separator: "\n").filter { !$0.hasPrefix("#") }.compactMap { $0.split(separator: ",").last?.trimmingCharacters(in: .whitespaces) }
        XCTAssertGreaterThanOrEqual(hashes.count, 6)
        if hashes.count >= 6 {
            XCTAssertNotEqual(hashes[0], hashes[1]); XCTAssertEqual(hashes[0], hashes[2]); XCTAssertEqual(hashes[0], hashes[4])
            XCTAssertEqual(hashes[1], hashes[3]); XCTAssertEqual(hashes[1], hashes[5])
        }
        let media = try await [scene.items[0].id: Probe.media(source.path, executable: ffprobe), image.id: Probe.media(scene.items[1].target, executable: ffprobe)]
        XCTAssertEqual(media[image.id]?.codecName, "webp_anim")
        let output = folder.appendingPathComponent("out.flv")
        let plan = try FFmpegCommand.build(scene: scene, profile: profile, platform: .custom, media: media, preview: nil, destination: output.path)
        XCTAssertTrue(plan.arguments.contains("-ignore_loop")); XCTAssertFalse(plan.arguments.contains("-stream_loop"))
        let result = try await ProcessRunner.run(ffmpeg, arguments: plan.arguments, timeout: 10)
        XCTAssertEqual(result.status, 0, String(decoding: result.error, as: UTF8.self))
        let info = try await Probe.media(output.path, executable: ffprobe); XCTAssertGreaterThan(info.duration, 1.3)
    }
    func testPersistentPublisherSurvivesProducerSwitchAndOutputDecodes() async throws {
        let (ffmpeg, ffprobe) = try tools(), folder = try temporary()
        defer { try? FileManager.default.removeItem(at: folder) }
        let source = folder.appendingPathComponent("source.mp4"); try await video(source, seconds: 4, ffmpeg: ffmpeg)
        let (scene, profile) = composition(source)
        let info = try await Probe.media(source.path, executable: ffprobe)
        let plan = try FFmpegCommand.build(scene: scene, profile: profile, platform: .custom, media: [scene.items[0].id: info], preview: nil)
        let output = folder.appendingPathComponent("published.flv")
        let relay = RelayEngine(executable: ffmpeg, destination: output.path, platform: .twitch, progress: { _ in }, log: { _ in })
        defer { relay.stop() }; try relay.start()
        let publisher = relay.processDetails.first { $0.role == "Publisher" }?.pid
        XCTAssertNotNil(publisher)
        let first = Task { try await relay.encode(arguments: plan.arguments, frameRate: 10) }
        try await Task.sleep(for: .milliseconds(600)); relay.interruptProducer()
        do { _ = try await first.value } catch is CancellationError { }
        XCTAssertEqual(relay.processDetails.first { $0.role == "Publisher" }?.pid, publisher)
        let second = try await relay.encode(arguments: plan.arguments, frameRate: 10)
        XCTAssertEqual(second, 0)
        try await relay.finish()
        let result = try await Probe.media(output.path, executable: ffprobe)
        XCTAssertEqual(result.width, 128); XCTAssertTrue(result.hasAudio); XCTAssertGreaterThan(result.duration, 3.5)
        let decode = try await ProcessRunner.run(ffmpeg, arguments: ["-v", "error", "-i", output.path, "-f", "null", "-"])
        XCTAssertEqual(decode.status, 0, String(decoding: decode.error, as: UTF8.self))
        let packets = try await ProcessRunner.run(ffprobe, arguments: ["-v", "error", "-show_packets", "-show_entries", "packet=codec_type,dts_time", "-of", "json", output.path])
        let object = try XCTUnwrap(JSONSerialization.jsonObject(with: packets.output) as? [String: Any])
        let values = try XCTUnwrap(object["packets"] as? [[String: Any]])
        var last: [String: Double] = [:]
        for packet in values {
            guard let type = packet["codec_type"] as? String, let value = packet["dts_time"] as? String, let time = Double(value) else { continue }
            if let previous = last[type] { XCTAssertGreaterThanOrEqual(time, previous, "Non-monotonic \(type) after producer switch") }
            last[type] = time
        }
        XCTAssertEqual(Set(last.keys), ["video", "audio"])
    }
    func testFacebookSenderPreservesQueryAndEncodedKeyAsPlaypath() {
        let key = "123%2Fkey?s_bl=1&s_sw=0&token=secret-token-value"
        let args = FFmpegCommand.sender(destination: "rtmps://live-api-s.facebook.com:443/rtmp/" + key)
        XCTAssertEqual(args.last, "rtmps://live-api-s.facebook.com:443/rtmp")
        XCTAssertEqual(args[args.firstIndex(of: "-rtmp_playpath")! + 1], key)
        XCTAssertEqual(args[args.firstIndex(of: "-rtmp_app")! + 1], "rtmp")
    }
    func testDroppingVideoFillsSlotAndOverlayRemainsNewLayer() {
        var scene = Scene(); var slot = SceneItem(); slot.width = 640; slot.height = 360; slot.target = ""
        var mic = SceneItem(); mic.kind = .microphone; mic.target = "1"; scene.items = [slot, mic]
        var video = SceneItem(); video.target = "/program.mp4"
        let filled = SceneGeometry.placing(video, in: scene, x: 100, y: 100)
        XCTAssertEqual(filled.selection, slot.id); XCTAssertEqual(filled.scene.items.count, 2)
        XCTAssertEqual(filled.scene.items[0].target, video.target); XCTAssertEqual(filled.scene.items[0].width, 640)
        var overlay = video; overlay.kind = .overlay; overlay.target = "/banner.png"; overlay.width = 100; overlay.height = 100
        let added = SceneGeometry.placing(overlay, in: filled.scene, x: 100, y: 100)
        XCTAssertEqual(added.scene.items.count, 3); XCTAssertEqual(added.scene.items[1], mic)
        XCTAssertEqual(added.selection, overlay.id)
    }
}
