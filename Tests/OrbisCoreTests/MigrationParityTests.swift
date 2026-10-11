import XCTest
@testable import OrbisCore

final class MigrationParityTests: XCTestCase {
    func testCanvasEndsAfterLongestVideoAndRepeatsShorterFrames() throws {
        var scene = Scene(); scene.width = 320; scene.height = 240
        var short = SceneItem(); short.target = "short.mp4"; short.width = 320; short.height = 240
        var long = short; long.id = UUID(); long.target = "long.mp4"
        scene.items = [short, long]
        var profile = EncoderProfile(); profile.codec = "libx264"; profile.width = 320; profile.height = 240
        let plan = try FFmpegCommand.build(scene: scene, profile: profile, platform: .custom,
            media: [short.id: MediaInfo(duration: 2), long.id: MediaInfo(duration: 7)], preview: nil, resume: 1)
        XCTAssertTrue(plan.arguments.contains("6.000000"))
        XCTAssertTrue(plan.arguments.contains(where: { $0.contains("eof_action=repeat") }))
        XCTAssertFalse(plan.arguments.contains("-shortest"))
        XCTAssertFalse(plan.arguments.contains("-re"))
    }
    func testPlatformPacingAndTimeouts() {
        let twitch = StreamingPolicy(platform: .twitch), youtube = StreamingPolicy(platform: .youtube)
        XCTAssertEqual(twitch.maxLead, 1); XCTAssertEqual(twitch.stallTimeout, 15)
        XCTAssertEqual(youtube.maxLead, 4); XCTAssertEqual(youtube.preroll, 2)
        let buffered = FFmpegCommand.sender(destination: "rtmps://example.test/live", policy: youtube)
        XCTAssertTrue(buffered.contains("-readrate_initial_burst"))
        XCTAssertFalse(FFmpegCommand.sender(destination: "rtmp://example.test/live", policy: twitch).contains("-readrate"))
        XCTAssertEqual(StreamingPolicy(platform: .tiktok).maxLead, 3)
    }
    func testAdaptiveBitrateMeasuresBacklogAndThroughput() {
        var controller = BitrateController(policy: StreamingPolicy(platform: .twitch))
        var decision: RateDecision?
        for step in 0...160 {
            let time = Double(step) * 0.5
            if let update = controller.observe(DeliverySample(time: time, media: time * 0.65, bytes: Int64(time * 500_000), lead: 1), nominal: 6_000_000, audioBitrate: 160_000) {
                decision = update; break
            }
        }
        XCTAssertEqual(decision?.from, 6_000_000)
        XCTAssertEqual(decision?.to, 3_240_000)
        XCTAssertEqual(decision?.throughput, 4_000_000)
        for (rate, lead) in [(0.65, 0.0), (0.0, 1.0), (1.0, 1.0)] {
            var healthy = BitrateController(policy: StreamingPolicy(platform: .twitch))
            for step in 0...180 {
                let time = Double(step) * 0.5
                XCTAssertNil(healthy.observe(DeliverySample(time: time, media: time * rate, bytes: Int64(time * 500_000), lead: lead), nominal: 6_000_000, audioBitrate: 160_000))
            }
        }
    }
    func testBoundedQueueAppliesBackpressureAndCancellationWakesWriter() throws {
        let buffer = DeliveryBuffer(maxLead: 1, maxBytes: 16)
        let token = buffer.beginSegment()
        try buffer.enqueue(DeliveryPacket(data: Data(repeating: 0, count: 8), timestamp: 0, segment: token))
        try buffer.enqueue(DeliveryPacket(data: Data(repeating: 0, count: 8), timestamp: 1000, segment: token))
        let cancelled = expectation(description: "Blocked enqueue is released on interruption")
        DispatchQueue.global().async {
            do { try buffer.enqueue(DeliveryPacket(data: Data(repeating: 0, count: 8), timestamp: 2000, segment: token)); XCTFail("Old segment accepted") }
            catch is CancellationError { cancelled.fulfill() }
            catch { XCTFail("Unexpected error: \(error)") }
        }
        buffer.interrupt()
        wait(for: [cancelled], timeout: 2)
        XCTAssertEqual(buffer.sentTimestamp, 0)
        buffer.stop(); XCTAssertNil(buffer.next())
    }
    func testCleanupSupportsEverythingYesterdayAndProtectsActiveHistory() {
        let now = Date()
        var old = LiveHistory(channel: Channel(), profile: EncoderProfile(), scene: nil, playlist: [])
        old.start = now.addingTimeInterval(-172800)
        var active = old; active.id = UUID()
        XCTAssertEqual(HistoryCleanup.eligible([old, active], months: 0, active: [active.id], now: now).map(\.id), [old.id])
        XCTAssertEqual(HistoryCleanup.eligible([old], months: -1, active: [], now: now).count, 1)
        XCTAssertNil(HistoryCleanup.cutoff(months: -2))
    }
    func testExistingDocumentsDecodeWithoutNewFields() throws {
        let old = Data(#"{"channels":[],"profiles":[],"scenes":[],"deck":[],"playlists":{},"history":[],"overlays":[],"ffmpegPath":"","ffprobePath":"","loopPlaylist":true}"#.utf8)
        let workspace = try JSONDecoder().decode(Workspace.self, from: old)
        XCTAssertTrue(workspace.interruptedRuns.isEmpty)
        let item = try JSONDecoder().decode(PlaylistItem.self, from: Data(#"{"path":"/video.mp4","enabled":true,"resumeSeconds":12,"duration":60}"#.utf8))
        XCTAssertNil(item.profileID); XCTAssertEqual(item.resumeSeconds, 12)
    }
    func testRelayCutRebasesFromSentPositionInsteadOfDiscardedQueue() throws {
        var rebaser = FLVRebaser()
        let header = Data([70,76,86,1,5,0,0,0,9,0,0,0,0])
        func tag(_ timestamp: UInt32) -> Data {
            Data([9,0,0,2,UInt8(truncatingIfNeeded: timestamp >> 16),UInt8(truncatingIfNeeded: timestamp >> 8),UInt8(truncatingIfNeeded: timestamp),UInt8(truncatingIfNeeded: timestamp >> 24),0,0,0,0x17,0,0,0,0,13])
        }
        rebaser.beginSegment(); _ = try rebaser.consume(header + tag(0) + tag(4000))
        rebaser.beginSegment(after: 1000, frameMilliseconds: 40)
        let result = try rebaser.consume(header + tag(0))
        XCTAssertEqual(result.first?.timestamp, 1040)
    }
}
