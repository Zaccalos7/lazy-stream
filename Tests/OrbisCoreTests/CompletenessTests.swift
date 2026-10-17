import XCTest
@testable import OrbisCore

final class CompletenessTests: XCTestCase {
    func testOrphanSweepPreservesReferencedMediaAndUnmanagedFiles() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        defer { try? FileManager.default.removeItem(at: root) }
        let store = try WorkspaceStore(directory: root)
        let source = root.appendingPathComponent("one.png"); try Data([1]).write(to: source)
        let kept = try store.importMedia(source, collection: "Overlays")
        try Data([2]).write(to: source)
        let orphan = try store.importMedia(source, collection: "Overlays")
        let unmanaged = root.appendingPathComponent("Overlays/user-note.txt"); try Data().write(to: unmanaged)
        try store.sweepMedia(collection: "Overlays", keeping: [kept], olderThan: Date().addingTimeInterval(60))
        XCTAssertTrue(FileManager.default.fileExists(atPath: kept))
        XCTAssertTrue(FileManager.default.fileExists(atPath: unmanaged.path))
        XCTAssertFalse(FileManager.default.fileExists(atPath: orphan))
    }
    func testStartFromVideoKeepsFollowingVideosAndExplicitResume() {
        let first = PlaylistItem(path: "/first.mp4")
        var second = PlaylistItem(path: "/second.mp4"); second.resumeSeconds = 22; second.enabled = false
        var third = PlaylistItem(path: "/third.mp4"); third.resumeSeconds = 11
        let history = LiveHistory(channel: Channel(), profile: EncoderProfile(), scene: nil, playlist: [first, second, third])
        let restarted = LiveReplay.starting(at: second.id, in: history, resume: false)
        XCTAssertEqual(restarted.map(\.id), [second.id, third.id]); XCTAssertTrue(restarted[0].enabled)
        XCTAssertTrue(restarted.allSatisfy { $0.resumeSeconds == 0 })
        XCTAssertEqual(LiveReplay.starting(at: second.id, in: history, resume: true)[0].resumeSeconds, 22)
        XCTAssertFalse(history.playlist[1].enabled)
        XCTAssertTrue(LiveReplay.starting(at: UUID(), in: history, resume: true).isEmpty)
    }
    func testCaptureCompositionDoesNotEndWhenFileEnds() throws {
        var scene = Scene(); var file = SceneItem(); file.target = "/file.mp4"
        var capture = SceneItem(); capture.kind = .camera; capture.target = "0"; capture.audioEnabled = false
        scene.items = [file, capture]
        let plan = try FFmpegCommand.build(scene: scene, profile: EncoderProfile(), platform: .custom, media: [file.id: MediaInfo(duration: 3)], preview: nil)
        XCTAssertFalse(plan.arguments.contains("-t"))
        XCTAssertFalse(plan.arguments.contains(where: { $0.hasPrefix("color=") && $0.contains(":d=") }))
    }
    func testSceneValidationRejectsOverflowAndDuplicateDeviceAliases() {
        var scene = Scene(); var file = SceneItem(); file.target = "/a.mp4"; file.x = Int.max
        scene.items = [file]; XCTAssertThrowsError(try scene.validate())
        file.x = 0; file.width = Int.max; scene.items = [file]; XCTAssertThrowsError(try scene.validate())
        var a = SceneItem(); a.kind = .camera; a.target = "0"
        var b = a; b.id = UUID(); b.target = "00"; scene.items = [a, b]
        XCTAssertThrowsError(try scene.validate())
        scene.items = [a]; scene.items[0].target = "not-an-index"; XCTAssertThrowsError(try scene.validate())
        scene.items = [a]; scene.name = "  "; XCTAssertThrowsError(try scene.validate())
    }
    func testLibraryDeduplicatesAndRetainsOriginalFileNameAndPermissions() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        defer { try? FileManager.default.removeItem(at: root) }
        let store = try WorkspaceStore(directory: root)
        let source = root.appendingPathComponent("banner.png"); try Data([1,2,3]).write(to: source)
        let first = try store.importMedia(source, collection: "Overlays")
        XCTAssertEqual(try store.importMedia(source, collection: "Overlays"), first)
        XCTAssertEqual(MediaFiles.displayName(first), "banner.png")
        let permissions = try FileManager.default.attributesOfItem(atPath: first)[.posixPermissions] as? NSNumber
        XCTAssertEqual(permissions?.intValue, 0o600)
        XCTAssertThrowsError(try store.importMedia(source, collection: "../other"))
        XCTAssertThrowsError(try store.importMedia(root, collection: "Overlays"))
        let bad = root.appendingPathComponent("unsupported.exe"); try Data().write(to: bad)
        XCTAssertThrowsError(try store.importMedia(bad, collection: "Overlays"))
    }
    func testSessionLogsSurviveRestartAndTailIsBounded() throws {
        let folder = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        defer { try? FileManager.default.removeItem(at: folder) }
        let id = UUID(), log = SessionLog(directory: folder, sessionID: id)
        log.append("first"); log.append(String(repeating: "a", count: 1000)); log.append("last"); log.flush()
        let full = try SessionLog.read(directory: folder, sessionID: id)
        XCTAssertTrue(full.contains("first")); XCTAssertTrue(full.contains("last"))
        let tail = try SessionLog.read(directory: folder, sessionID: id, maxBytes: 100)
        XCTAssertLessThanOrEqual(tail.utf8.count, 100); XCTAssertTrue(tail.contains("last")); XCTAssertFalse(tail.contains("first"))
        XCTAssertEqual(try SessionLog.read(directory: folder, sessionID: UUID()), "")
    }
    func testPublicLinksAcceptNamesAndKnownFullUrlsAndRejectLookalikes() {
        var channel = Channel(); channel.streamURL = Platform.youtube.ingest; channel.streamName = "@studio"
        XCTAssertEqual(PlatformLinks.publicURL(channel)?.absoluteString, "https://www.youtube.com/@studio/live")
        channel.streamName = "https://www.youtube.com/@studio/live"
        XCTAssertEqual(PlatformLinks.publicURL(channel)?.absoluteString, channel.streamName)
        channel.streamName = "https://youtube.com.attacker.test/live"; XCTAssertNil(PlatformLinks.publicURL(channel))
        channel.streamName = "https://www.youtube.com/@studio/live"
        XCTAssertEqual(PlatformLinks.publicURL(channel, onAirVideo: "abcdefghijk")?.absoluteString, "https://www.youtube.com/watch?v=abcdefghijk")
        channel.streamURL = Platform.tiktok.ingest; channel.streamName = "@studio"
        XCTAssertEqual(PlatformLinks.publicURL(channel)?.absoluteString, "https://www.tiktok.com/@studio/live")
    }
    func testMacUpdateFeedSelectsArm64PackageAndNumericVersions() throws {
        XCTAssertTrue(MacUpdates.isNewer("v3.10.0", than: "3.9.9"))
        XCTAssertFalse(MacUpdates.isNewer("3.0.0.0", than: "3.0"))
        XCTAssertFalse(MacUpdates.isNewer("3.1-rc1", than: "3.0"))
        let release = try MacUpdates.manifest(Data(#"{"version":"3.1.0","architecture":"arm64","url":"https://example.test/Orbis-arm64.pkg"}"#.utf8))
        XCTAssertEqual(release.version, "3.1.0")
        for data in [#"{"version":"3.1","architecture":"x64","url":"https://example.test/app.pkg"}"#,
                     #"{"version":"3.1","architecture":"arm64","url":"http://example.test/app.pkg"}"#,
                     #"{"version":"3.1","architecture":"arm64","url":"https://example.test/setup.exe"}"#] {
            XCTAssertThrowsError(try MacUpdates.manifest(Data(data.utf8)))
        }
        let github = Data(#"{"tag_name":"v3.2.0","draft":false,"prerelease":false,"assets":[{"name":"setup-win.exe","browser_download_url":"https://example.test/setup.exe"},{"name":"Orbis-3.2.0-arm64.pkg","browser_download_url":"https://example.test/Orbis-3.2.0-arm64.pkg"}]}"#.utf8)
        XCTAssertEqual(try MacUpdates.github(github)?.version, "3.2.0")
    }
    func testPublisherFailureCannotBeReportedAsSuccess() async throws {
        let ffmpeg = try Executables.resolve("ffmpeg")
        let missing = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString).appendingPathComponent("missing/out.flv")
        let relay = RelayEngine(executable: ffmpeg, destination: missing.path, platform: .custom, progress: { _ in }, log: { _ in })
        defer { relay.stop() }; try relay.start()
        try await Task.sleep(for: .milliseconds(200))
        do { try await relay.finish(); XCTFail("A publisher without a valid output must fail") }
        catch { XCTAssertFalse(error.localizedDescription.isEmpty) }
    }
    func testProcessCancellationStopsAndDrainsBothPipes() async throws {
        let ffmpeg = try Executables.resolve("ffmpeg")
        let task = Task { try await ProcessRunner.run(ffmpeg, arguments: ["-hide_banner", "-f", "lavfi", "-i", "testsrc2=size=64x64", "-f", "null", "-"], timeout: 30) }
        try await Task.sleep(for: .milliseconds(150)); task.cancel()
        do { _ = try await task.value; XCTFail("Cancelled process must throw") }
        catch is CancellationError { }
    }
}
