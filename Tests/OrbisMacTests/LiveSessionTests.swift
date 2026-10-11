import XCTest
import OrbisCore
@testable import OrbisMac

@MainActor final class LiveSessionTests: XCTestCase {
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
        let finished = expectation(description: "Program and queued spots completed")
        let session = LiveSession(channel: channel, profile: profile, scene: nil, playlist: [PlaylistItem(path: program.path)], historyID: UUID(),
            ffmpeg: ffmpeg, ffprobe: ffprobe, destination: output.path, directory: folder, loop: false) { _ in finished.fulfill() }
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
        await fulfillment(of: [finished], timeout: 25)
        XCTAssertEqual(session.status, .stopped, session.logs.joined(separator: "\n"))
        XCTAssertNil(session.takeover); XCTAssertTrue(session.queuedSpots.isEmpty)
        let media = try await Probe.media(output.path, executable: ffprobe)
        XCTAssertEqual(media.width, 320); XCTAssertEqual(media.height, 180)
        XCTAssertTrue(media.hasAudio); XCTAssertGreaterThan(media.duration, 7)
    }
}
