import XCTest
@testable import OrbisCore

final class OrbisCoreTests: XCTestCase {
    func testPlatformDetectionUsesDomainBoundaries() {
        XCTAssertEqual(Platform.detect("rtmps://a.rtmp.youtube.com/live2"), .youtube)
        XCTAssertEqual(Platform.detect("rtmps://123456abcdef.global-contribute.live-video.net/app"), .kick)
        XCTAssertEqual(Platform.detect("rtmp://ingest.global-contribute.live-video.net/app"), .twitch)
        XCTAssertEqual(Platform.detect("rtmp://push.tiktokcdn-eu.com/live"), .tiktok)
        XCTAssertEqual(Platform.detect("rtmp://youtube.com.attacker.test/live"), .custom)
    }
    func testDestinationPreservesKeyQueryAndNormalizesYouTube() throws {
        var channel = Channel(); channel.streamURL = "rtmp://a.rtmp.youtube.com/live2/"
        XCTAssertEqual(try channel.destination(key: "secret?token=value"), "rtmps://a.rtmps.youtube.com/live2/secret?token=value")
        channel.streamURL = "https://example.com"
        XCTAssertThrowsError(try channel.destination(key: "secret"))
    }
    func testValidationRejectsHalfResolutionAndReservedOptions() {
        var profile = EncoderProfile(); profile.width = 0
        XCTAssertThrowsError(try profile.validate())
        profile.height = 0; XCTAssertNoThrow(try profile.validate())
        profile.options = [EncoderOption(key: "i", value: "/tmp/private")]
        XCTAssertThrowsError(try profile.validate())
        profile.options = [EncoderOption(key: "preset", value: "veryfast")]
        XCTAssertNoThrow(try profile.validate())
    }
    func testCompositorIncludesMixedAudioAndAppleDevices() throws {
        var scene = Scene()
        var file = SceneItem(); file.target = "/tmp/a file with 'quotes'.mp4"
        var camera = SceneItem(); camera.kind = .camera; camera.target = "2"; camera.audioEnabled = false; camera.width = 640; camera.height = 360
        var microphone = SceneItem(); microphone.kind = .microphone; microphone.target = "1"; microphone.volume = 50; microphone.width = 0; microphone.height = 0
        var overlay = SceneItem(); overlay.kind = .overlay; overlay.target = "/tmp/alpha.png"; overlay.audioEnabled = false
        scene.items = [file, camera, microphone, overlay]
        let plan = try FFmpegCommand.build(scene: scene, profile: EncoderProfile(), platform: .youtube,
            media: [file.id: MediaInfo(hasAudio: true)], preview: "/tmp/preview.jpg")
        XCTAssertTrue(plan.arguments.contains(file.target))
        XCTAssertTrue(plan.arguments.contains("2:none"))
        XCTAssertTrue(plan.arguments.contains("none:1"))
        XCTAssertTrue(plan.arguments.contains("h264_videotoolbox"))
        let graph = plan.arguments[plan.arguments.firstIndex(of: "-filter_complex")! + 1]
        XCTAssertTrue(graph.contains("volume=0.500000"))
        XCTAssertTrue(graph.contains("amix=inputs=2"))
        XCTAssertTrue(graph.contains("format=rgba"))
        XCTAssertTrue(plan.arguments.contains("-atomic_writing"))
        XCTAssertFalse(plan.arguments.contains("gdigrab"))
        XCTAssertFalse(plan.arguments.contains("dshow"))
    }
    func testTikTokPortraitAndSilence() throws {
        var scene = Scene(); var item = SceneItem(); item.target = "/tmp/a.mp4"; scene.items = [item]
        let plan = try FFmpegCommand.build(scene: scene, profile: EncoderProfile(), platform: .tiktok, media: [:], preview: nil)
        XCTAssertEqual(plan.width, 1080); XCTAssertEqual(plan.height, 1920)
        XCTAssertTrue(plan.arguments.contains("anullsrc=r=48000:cl=stereo"))
    }
    func testProgressUsesMicrosecondsAndIgnoresNA() {
        var state = StreamProgress()
        XCTAssertFalse(state.consume("out_time_us=2500000"))
        XCTAssertFalse(state.consume("bitrate=6000.0kbits/s"))
        _ = state.consume("speed=1.01x"); _ = state.consume("fps=N/A")
        XCTAssertTrue(state.consume("progress=continue"))
        XCTAssertEqual(state.seconds, 2.5); XCTAssertEqual(state.bitrate, 6000); XCTAssertEqual(state.speed, 1.01)
    }
    func testRelayRebasesAcrossChunkBoundariesAndKeepsOneHeader() throws {
        var relay = FLVRebaser(); relay.beginSegment()
        let segment = header + tag(timestamp: 0) + tag(timestamp: 100)
        var first: [(data: Data, timestamp: UInt32?)] = []
        for byte in segment { first += try relay.consume(Data([byte])) }
        XCTAssertEqual(first.count, 3); XCTAssertNil(first[0].timestamp)
        XCTAssertEqual(first[2].timestamp, 100)
        relay.beginSegment()
        let second = try relay.consume(header + tag(timestamp: 20) + tag(timestamp: 70))
        XCTAssertEqual(second.count, 2); XCTAssertEqual(second[0].timestamp, 134); XCTAssertEqual(second[1].timestamp, 184)
        XCTAssertEqual(second[0].data[6], 134)
    }
    func testRelayRejectsCorruptPreviousTagSize() {
        var relay = FLVRebaser(); relay.beginSegment()
        var corrupt = tag(timestamp: 0); corrupt[corrupt.count - 1] = 0
        XCTAssertThrowsError(try relay.consume(header + corrupt))
    }
    func testWorkspaceRoundTripAndNaturalPlaylistOrdering() throws {
        let directory = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        defer { try? FileManager.default.removeItem(at: directory) }
        let store = try WorkspaceStore(directory: directory)
        var workspace = Workspace(); workspace.channels = [Channel()]
        try store.save(workspace)
        XCTAssertEqual(try store.load().channels, workspace.channels)
        for file in ["clip10.mp4", "clip2.mp4", "clip1.mp4", "not-video.txt"] {
            try Data().write(to: directory.appendingPathComponent(file))
        }
        try FileManager.default.createDirectory(at: directory.appendingPathComponent("directory.mp4"), withIntermediateDirectories: true)
        XCTAssertEqual(try MediaFiles.playlist(folder: directory.path).map(\.name), ["clip1.mp4", "clip2.mp4", "clip10.mp4"])
        let data = try Data(contentsOf: store.document)
        XCTAssertFalse(String(decoding: data, as: UTF8.self).contains("streamKey"))
    }
    func testFFmpegSoftwareOutputAndPreview() async throws {
        guard let executable = try? Executables.resolve("ffmpeg") else { throw XCTSkip("FFmpeg non installato") }
        let directory = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: directory) }
        let source = directory.appendingPathComponent("source.mp4")
        let create = try await ProcessRunner.run(executable, arguments: ["-hide_banner", "-loglevel", "error", "-f", "lavfi", "-i", "testsrc2=size=320x180:rate=25", "-t", "1", "-c:v", "mpeg4", "-y", source.path])
        XCTAssertEqual(create.status, 0, String(decoding: create.error, as: UTF8.self))
        var scene = Scene(); scene.width = 320; scene.height = 180
        var item = SceneItem(); item.target = source.path; item.width = 320; item.height = 180; scene.items = [item]
        var profile = EncoderProfile(); profile.width = 320; profile.height = 180; profile.frameRate = 25; profile.codec = "libx264"
        let output = directory.appendingPathComponent("out.flv")
        let preview = directory.appendingPathComponent("preview.jpg")
        let plan = try FFmpegCommand.build(scene: scene, profile: profile, platform: .youtube, media: [item.id: MediaInfo(width: 320, height: 180, frameRate: 25, duration: 1)], preview: preview.path, destination: output.path)
        let result = try await ProcessRunner.run(executable, arguments: plan.arguments, timeout: 15)
        XCTAssertEqual(result.status, 0, String(decoding: result.error, as: UTF8.self))
        XCTAssertTrue(FileManager.default.fileExists(atPath: preview.path))
        let firstSegment = try Data(contentsOf: output)
        XCTAssertGreaterThan(firstSegment.count, 1000)
        var rebaser = FLVRebaser(); var combined = Data()
        for _ in 0..<2 {
            rebaser.beginSegment()
            for packet in try rebaser.consume(firstSegment) { combined.append(packet.data) }
        }
        let combinedURL = directory.appendingPathComponent("combined.flv")
        try combined.write(to: combinedURL)
        let decode = try await ProcessRunner.run(executable, arguments: ["-hide_banner", "-loglevel", "error", "-i", combinedURL.path, "-f", "null", "-"], timeout: 10)
        XCTAssertEqual(decode.status, 0, String(decoding: decode.error, as: UTF8.self))
        guard let ffprobe = try? Executables.resolve("ffprobe") else { return }
        let info = try await Probe.media(combinedURL.path, executable: ffprobe)
        XCTAssertEqual(info.width, 320); XCTAssertEqual(info.height, 180)
        XCTAssertTrue(info.hasAudio); XCTAssertGreaterThan(info.duration, 1.5)
    }
    func testYouTubePrimaryPlayerDistinguishesEndedUnknownAndLive() {
        let live = #"var ytInitialPlayerResponse = {"videoDetails":{"isLive":true}};"#
        let ended = #"var ytInitialPlayerResponse = {"videoDetails":{"isLiveContent":true},"microformat":{"playerMicroformatRenderer":{"liveBroadcastDetails":{"isLiveNow":false}}}};"#
        let unknown = #"var ytInitialPlayerResponse = {"videoDetails":{"isLiveContent":true}};"#
        XCTAssertEqual(YouTubeAir.playerState(live), .onAir)
        XCTAssertEqual(YouTubeAir.playerState(ended), .ended)
        XCTAssertEqual(YouTubeAir.playerState(unknown), .unverified)
        XCTAssertEqual(YouTubeAir.playerState("consent page \"isLiveNow\":true"), .unverified)
        XCTAssertEqual(YouTubeAir.jsonVariable("data", html: #"var data = {"text":"brace } and escaped \" quote", "object":{}};"#), #"{"text":"brace } and escaped \" quote", "object":{}}"#)
    }
    private var header: Data { Data([70, 76, 86, 1, 5, 0, 0, 0, 9, 0, 0, 0, 0]) }
    private func tag(timestamp: UInt32) -> Data {
        Data([9, 0, 0, 2, UInt8(truncatingIfNeeded: timestamp >> 16), UInt8(truncatingIfNeeded: timestamp >> 8), UInt8(truncatingIfNeeded: timestamp), UInt8(truncatingIfNeeded: timestamp >> 24), 0, 0, 0, 0x17, 0, 0, 0, 0, 13])
    }
}
