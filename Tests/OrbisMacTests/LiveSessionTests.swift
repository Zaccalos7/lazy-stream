import XCTest
import OrbisCore
@testable import OrbisMac

@MainActor final class LiveSessionTests: XCTestCase {
    func testSceneChangesReturnToOriginalPlaylistPosition() throws {
        let item = PlaylistItem(path: "/tmp/program.mp4")
        let session = LiveSession(channel: Channel(), profile: EncoderProfile(), scene: nil, playlist: [item], historyID: UUID(),
            ffmpeg: URL(fileURLWithPath: "/unused/ffmpeg"), ffprobe: URL(fileURLWithPath: "/unused/ffprobe"),
            destination: "/unused/output.flv", directory: FileManager.default.temporaryDirectory, loop: false) { _ in }
        session.currentPosition = 34
        var scene = Scene(); var source = SceneItem(); source.target = "/tmp/other.mp4"; scene.items = [source]
        try session.switchScene(scene)
        XCTAssertEqual(session.currentPosition, 0); XCTAssertNotNil(session.scene)
        session.returnToPlaylist()
        XCTAssertNil(session.scene); XCTAssertEqual(session.currentPosition, 34)
        XCTAssertEqual(session.sources.first?.id, item.id)
    }
    func testPlaylistMixerKeepsOneSourceAndClampsVolume() {
        let item = PlaylistItem(path: "/tmp/program.mp4")
        let session = LiveSession(channel: Channel(), profile: EncoderProfile(), scene: nil, playlist: [item], historyID: UUID(),
            ffmpeg: URL(fileURLWithPath: "/unused/ffmpeg"), ffprobe: URL(fileURLWithPath: "/unused/ffprobe"),
            destination: "/unused/output.flv", directory: FileManager.default.temporaryDirectory, loop: false) { _ in }
        XCTAssertEqual(session.sources.map(\.id), [item.id])
        session.setVolume(id: item.id, value: 250)
        XCTAssertEqual(session.sources.first?.volume, 200)
        session.setVolume(id: item.id, value: -1)
        XCTAssertEqual(session.sources.first?.volume, 0)
        session.setVolume(id: UUID(), value: 100)
        XCTAssertEqual(session.programVolume, 0)
        session.setVolume(id: item.id, value: .nan)
        XCTAssertEqual(session.programVolume, 0)
    }
    func testQueuedSpotsReturnToProgramWithoutOpeningAnotherPublisher() async throws {
        let ffmpeg = try Executables.resolve("ffmpeg"), ffprobe = try Executables.resolve("ffprobe")
        let folder = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: folder) }
        let program = folder.appendingPathComponent("program.mp4"), spot = folder.appendingPathComponent("spot.mp4")
        for (url, length) in [(program, 6), (spot, 1)] {
            let result = try await ProcessRunner.run(ffmpeg, arguments: ["-hide_banner", "-loglevel", "error", "-f", "lavfi", "-i", "testsrc2=size=320x180:rate=25", "-t", String(length), "-c:v", "mpeg4", "-y", url.path])
            XCTAssertEqual(result.status, 0, String(decoding: result.error, as: UTF8.self))
        }
        var channel = Channel(); channel.streamURL = "rtmp://live.twitch.tv/app"
        var profile = EncoderProfile(); profile.codec = "libx264"; profile.quality = .balanced
        profile.width = 320; profile.height = 180; profile.frameRate = 25; profile.adaptiveBitrate = false
        let output = folder.appendingPathComponent("captured.flv")
        var completed = false
        let session = LiveSession(channel: channel, profile: profile, scene: nil, playlist: [PlaylistItem(path: program.path)], historyID: UUID(),
            ffmpeg: ffmpeg, ffprobe: ffprobe, destination: output.path, directory: folder, loop: false) { _ in completed = true }
        defer { session.stop() }
        session.start()
        let deadline = Date().addingTimeInterval(12)
        while session.status != .live && session.isRunning && Date() < deadline { try await Task.sleep(for: .milliseconds(100)) }
        XCTAssertEqual(session.status, .live, session.logs.joined(separator: "\n"))
        let publisher = session.processDetails.first { $0.role == "Publisher" }?.pid
        XCTAssertNotNil(publisher)
        try session.enqueueSpot(spot.path); try session.enqueueSpot(spot.path)
        XCTAssertEqual(session.queuedSpots.count, 1)
        try await Task.sleep(for: .milliseconds(500))
        XCTAssertEqual(session.processDetails.first { $0.role == "Publisher" }?.pid, publisher)
        let endDeadline = Date().addingTimeInterval(25)
        while !completed && Date() < endDeadline { try await Task.sleep(for: .milliseconds(100)) }
        XCTAssertTrue(completed, session.logs.joined(separator: "\n"))
        XCTAssertEqual(session.status, .stopped, session.logs.joined(separator: "\n"))
        XCTAssertNil(session.takeover); XCTAssertTrue(session.queuedSpots.isEmpty)
        let media = try await Probe.media(output.path, executable: ffprobe)
        XCTAssertEqual(media.width, 320); XCTAssertEqual(media.height, 180)
        XCTAssertTrue(media.hasAudio); XCTAssertGreaterThan(media.duration, 7)
    }
}
