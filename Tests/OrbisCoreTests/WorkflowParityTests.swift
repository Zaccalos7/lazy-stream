import XCTest
@testable import OrbisCore

final class WorkflowParityTests: XCTestCase {
    func testNonStreamingProfilesCanBeSavedButCannotReplaceLiveOutput() throws {
        var profile = EncoderProfile(); profile.codec = "hevc_videotoolbox"; profile.format = "mp4"
        XCTAssertNoThrow(try profile.validate())
        XCTAssertThrowsError(try profile.validateForStreaming())
        profile.format = "flv"
        XCTAssertThrowsError(try profile.validateForStreaming())
        profile.codec = "h264_videotoolbox"; profile.audioCodec = "libopus"
        XCTAssertThrowsError(try profile.validateForStreaming())
        profile.audioCodec = "aac"
        XCTAssertNoThrow(try profile.validateForStreaming())
    }
    func testPlatformPresetsMatchOutputSettingsAndUseAppleEncoder() throws {
        let expected: [(Platform, Bool, Int, Int, Int, Int, Double)] = [
            (.twitch, false, 6_000_000, 160_000, 0, 0, 0),
            (.twitch, true, 3_000_000, 96_000, 1280, 720, 30),
            (.youtube, false, 6_000_000, 128_000, 1920, 1080, 30),
            (.youtube, true, 3_000_000, 128_000, 1280, 720, 30),
            (.kick, false, 6_000_000, 160_000, 1920, 1080, 30),
            (.kick, true, 3_000_000, 128_000, 1280, 720, 30),
            (.facebook, false, 4_500_000, 128_000, 1920, 1080, 30),
            (.facebook, true, 3_000_000, 128_000, 1280, 720, 30),
            (.tiktok, false, 4_500_000, 128_000, 1080, 1920, 30),
            (.tiktok, true, 2_500_000, 128_000, 720, 1280, 30)
        ]
        for (platform, light, video, audio, width, height, fps) in expected {
            let profile = PlatformPresets.profile(platform, light: light)
            try profile.validate()
            XCTAssertEqual(profile.videoBitrate, video); XCTAssertEqual(profile.audioBitrate, audio)
            XCTAssertEqual(profile.width, width); XCTAssertEqual(profile.height, height); XCTAssertEqual(profile.frameRate, fps)
            XCTAssertEqual(profile.codec, "h264_videotoolbox"); XCTAssertEqual(profile.format, "flv")
            XCTAssertEqual(profile.defaultPlatform, platform); XCTAssertEqual(profile.isDefault, !light)
            XCTAssertEqual(profile.options.first(where: { $0.key == "profile:v" })?.value, light || platform == .facebook ? "main" : "high")
            XCTAssertEqual(Platform.detect(platform.ingest), platform)
        }
        let defaults = Workspace().profiles
        XCTAssertEqual(defaults.count, expected.count)
        XCTAssertEqual(Set(defaults.map(\.id)).count, defaults.count)
    }
    func testResumeUsesLastVideoAndRestartResetsEveryPositionWithoutChangingSnapshot() {
        var first = PlaylistItem(path: "/one.mp4"); first.resumeSeconds = 11
        var second = PlaylistItem(path: "/two.mp4"); second.resumeSeconds = 42
        var third = PlaylistItem(path: "/three.mp4"); third.resumeSeconds = 5
        var history = LiveHistory(channel: Channel(), profile: EncoderProfile(), scene: nil, playlist: [first, second, third])
        history.lastVideoID = second.id
        let resumed = LiveReplay.playlist(history, resume: true)
        XCTAssertEqual(resumed.map(\.id), [second.id, third.id]); XCTAssertEqual(resumed.first?.resumeSeconds, 42)
        let restarted = LiveReplay.playlist(history, resume: false)
        XCTAssertEqual(restarted.map(\.id), [first.id, second.id, third.id])
        XCTAssertTrue(restarted.allSatisfy { $0.resumeSeconds == 0 })
        XCTAssertEqual(history.playlist[1].resumeSeconds, 42)
        let checkpoint = PlaybackCheckpoint(historyID: history.id, channelID: history.channel.id, index: 2, position: 99)
        let recovered = LiveReplay.playlist(history, resume: true, checkpoint: checkpoint)
        XCTAssertEqual(recovered.map(\.id), [third.id]); XCTAssertEqual(recovered.first?.resumeSeconds, 99)
    }
    func testOldHistorySnapshotStillDecodesAndMissingVideoFallsBackToFirst() throws {
        var history = LiveHistory(channel: Channel(), profile: EncoderProfile(), scene: nil, playlist: [PlaylistItem(path: "/one.mp4")])
        let encoded = try JSONEncoder().encode(history)
        var object = try XCTUnwrap(JSONSerialization.jsonObject(with: encoded) as? [String: Any])
        object.removeValue(forKey: "lastVideoID")
        let decoded = try JSONDecoder().decode(LiveHistory.self, from: JSONSerialization.data(withJSONObject: object))
        XCTAssertNil(decoded.lastVideoID)
        history.lastVideoID = UUID()
        XCTAssertEqual(LiveReplay.playlist(history, resume: true).count, 1)
        let empty = LiveHistory(channel: Channel(), profile: EncoderProfile(), scene: nil, playlist: [])
        XCTAssertTrue(LiveReplay.playlist(empty, resume: true).isEmpty)
    }
    func testReportsHideFullUrlsDecodedKeysAndDetachedTokens() {
        let key = "private%2Fstream?token=abcdefgh12345678&other=longsecret123"
        let message = "rtmps://host/live/\(key) private/stream abcdefgh12345678 longsecret123 rtmp://other/key"
        let report = ErrorReport.redact(message, keys: [key])
        for secret in [key, "private/stream", "abcdefgh12345678", "longsecret123", "rtmps://", "rtmp://"] {
            XCTAssertFalse(report.contains(secret), secret)
        }
        XCTAssertTrue(report.contains("[stream URL hidden]"))
    }
    func testQrPartsPreserveUnicodeAndRespectByteLimit() {
        let text = String(repeating: "è😀漢字\n", count: 200)
        let parts = ErrorReport.parts(text)
        XCTAssertGreaterThan(parts.count, 1)
        XCTAssertTrue(parts.allSatisfy { !$0.isEmpty && $0.utf8.count <= 600 })
        XCTAssertEqual(parts.joined(), text)
        XCTAssertTrue(ErrorReport.parts("").isEmpty)
        XCTAssertEqual(ErrorReport.parts("😀é", maxBytes: 1).joined(), "😀é")
    }
}
