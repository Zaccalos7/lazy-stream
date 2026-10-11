import Foundation
import Darwin
import OrbisCore

/// Blocking reads and paced writes live on a worker queue, never on the UI thread.
final class RelayEngine: @unchecked Sendable {
    private let lock = NSLock()
    private var producer: Process?
    private var sender: Process?
    private var input: FileHandle?
    private var cancelled = false
    private var rebaser = FLVRebaser()
    private var clockOrigin: TimeInterval?
    private let buffer: DeliveryBuffer
    private let policy: StreamingPolicy
    private var latestReadTimestamp: UInt32 = 0
    private var deliveryFailure: String?
    private var headerDelivered = false
    private var positionSegment: UInt64 = 0
    private var segmentFirstTimestamp: UInt32?
    private var segmentPosition = 0.0
    var currentSegmentPosition: Double { lock.lock(); defer { lock.unlock() }; return segmentPosition }
    var processDetails: [StreamingProcess] {
        lock.lock(); defer { lock.unlock() }
        return [("Encoder", producer), ("Publisher", sender)].compactMap { role, process in
            guard let process, process.isRunning else { return nil }; return StreamingProcess(role: role, pid: process.processIdentifier)
        }
    }
    func deliverySample(media: Double, bytes: Int64) -> DeliverySample {
        lock.lock(); let latest = latestReadTimestamp; lock.unlock()
        return DeliverySample(time: ProcessInfo.processInfo.systemUptime, media: media, bytes: bytes, lead: max(0, Double(latest) / 1000 - media))
    }
    private let executable: URL
    private let destination: String
    private let progress: @Sendable (StreamProgress) -> Void
    private let log: @Sendable (String) -> Void
    init(executable: URL, destination: String, platform: Platform, progress: @escaping @Sendable (StreamProgress) -> Void, log: @escaping @Sendable (String) -> Void) {
        policy = StreamingPolicy(platform: platform); buffer = DeliveryBuffer(maxLead: policy.maxLead)
        self.executable = executable; self.destination = destination; self.progress = progress; self.log = log
    }
    func start() throws {
        let process = Process(); let stdin = Pipe(); let stdout = Pipe(); let stderr = Pipe()
        process.executableURL = executable; process.arguments = FFmpegCommand.sender(destination: destination, policy: policy)
        process.standardInput = stdin; process.standardOutput = stdout; process.standardError = stderr
        try process.run()
        lock.lock(); sender = process; input = stdin.fileHandleForWriting; lock.unlock()
        drainLines(stdout.fileHandleForReading) { [progress] lines in
            var state = StreamProgress()
            for line in lines { if state.consume(line) { progress(state) } }
        }
        drainErrors(stderr.fileHandleForReading)
        DispatchQueue.global(qos: .userInitiated).async { [self] in deliver() }
    }
    private func deliver() {
        var primed = false
        var lastTarget: Double?
        do {
            while let packet = buffer.next() {
                if !buffer.isCurrent(packet.segment) { buffer.complete(packet, delivered: false); continue }
                if let timestamp = packet.timestamp, !policy.senderPacing {
                    if !primed { buffer.waitForCushion(seconds: policy.maxLead); primed = true }
                    let now = ProcessInfo.processInfo.systemUptime
                    if clockOrigin == nil { clockOrigin = now - policy.preroll - Double(timestamp) / 1000 }
                    var target = clockOrigin! + Double(timestamp) / 1000
                    if target - now > 5 { clockOrigin! -= target - now; target = now }
                    let due = max(0, target - (lastTarget ?? target)); lastTarget = target
                    // Recover lateness over paced frames instead of dumping the accumulated queue.
                    let deadline = target < now ? now + due / 2 : target
                    while ProcessInfo.processInfo.systemUptime < deadline && buffer.isCurrent(packet.segment) {
                        Thread.sleep(forTimeInterval: max(0, min(0.01, deadline - ProcessInfo.processInfo.systemUptime)))
                    }
                }
                if !buffer.isCurrent(packet.segment) { buffer.complete(packet, delivered: false); continue }
                lock.lock(); let handle = input; let alive = sender?.isRunning == true; lock.unlock()
                guard alive, let handle else { throw OrbisError.process("Il publisher ha chiuso la connessione.") }
                try handle.write(contentsOf: packet.data)
                if packet.timestamp == nil && packet.data.starts(with: [70, 76, 86]) { lock.lock(); headerDelivered = true; lock.unlock() }
                if let timestamp = packet.timestamp {
                    lock.lock()
                    if positionSegment == packet.segment {
                        if segmentFirstTimestamp == nil { segmentFirstTimestamp = timestamp }
                        segmentPosition = max(segmentPosition, Double(timestamp >= segmentFirstTimestamp! ? timestamp - segmentFirstTimestamp! : 0) / 1000)
                    }
                    lock.unlock()
                }
                buffer.complete(packet)
            }
        } catch {
            lock.lock(); deliveryFailure = error.localizedDescription; lock.unlock()
            log(error.localizedDescription); stop()
        }
    }
    func encode(arguments: [String], frameRate: Double = 30) async throws -> Int32 {
        try await withCheckedThrowingContinuation { continuation in
            DispatchQueue.global(qos: .userInitiated).async { [self] in
                let process = Process(); let stdout = Pipe(); let stderr = Pipe()
                process.executableURL = executable; process.arguments = arguments
                process.standardInput = FileHandle.nullDevice; process.standardOutput = stdout; process.standardError = stderr
                do {
                    lock.lock()
                    if cancelled { lock.unlock(); throw CancellationError() }
                    do { try process.run() } catch { lock.unlock(); throw error }
                    producer = process; lock.unlock()
                    let errorDone = DispatchGroup(); errorDone.enter()
                    DispatchQueue.global().async { [self] in
                        let data = stderr.fileHandleForReading.readDataToEndOfFile()
                        log(redact(String(decoding: data.suffix(16000), as: UTF8.self))); errorDone.leave()
                    }
                    buffer.waitForWriter()
                    let token = buffer.beginSegment()
                    lock.lock(); positionSegment = token; segmentFirstTimestamp = nil; segmentPosition = 0; lock.unlock()
                    lock.lock(); let hasHeader = headerDelivered; lock.unlock()
                    if !hasHeader { rebaser = FLVRebaser() }
                    rebaser.beginSegment(after: buffer.sentTimestamp, frameMilliseconds: UInt32(max(1, (1000 / max(1, frameRate)).rounded())))
                    while true {
                        let chunk = stdout.fileHandleForReading.availableData
                        if chunk.isEmpty { break }
                        for packet in try rebaser.consume(chunk) {
                            if let timestamp = packet.timestamp { lock.lock(); latestReadTimestamp = timestamp; lock.unlock() }
                            try buffer.enqueue(DeliveryPacket(data: packet.data, timestamp: packet.timestamp, segment: token))
                        }
                    }
                    try buffer.finishAndWait(segment: token)
                    process.waitUntilExit(); errorDone.wait()
                    lock.lock(); if producer === process { producer = nil }; lock.unlock()
                    continuation.resume(returning: process.terminationStatus)
                } catch {
                    terminate(process)
                    // Drain stdout before waiting: FFmpeg can be blocked on a full pipe.
                    _ = stdout.fileHandleForReading.readDataToEndOfFile()
                    if process.processIdentifier != 0 { process.waitUntilExit() }
                    lock.lock(); if producer === process { producer = nil }; lock.unlock()
                    continuation.resume(throwing: error)
                }
            }
        }
    }
    func interruptProducer() {
        buffer.interrupt()
        lock.lock(); let process = producer; producer = nil; lock.unlock()
        if let process { terminate(process) }
    }
    func finish() async {
        buffer.stop()
        let (publisher, handle) = detachPublisher()
        try? handle?.close()
        guard let publisher else { return }
        await withCheckedContinuation { continuation in
            DispatchQueue.global().async {
                let timeout = DispatchWorkItem { if publisher.isRunning { publisher.terminate() } }
                DispatchQueue.global().asyncAfter(deadline: .now() + 2, execute: timeout)
                DispatchQueue.global().asyncAfter(deadline: .now() + 4) { if publisher.isRunning { _ = kill(publisher.processIdentifier, SIGKILL) } }
                publisher.waitUntilExit(); timeout.cancel(); continuation.resume()
            }
        }
    }
    private func detachPublisher() -> (Process?, FileHandle?) {
        lock.lock(); let publisher = sender; let handle = input
        cancelled = true; sender = nil; input = nil; lock.unlock()
        return (publisher, handle)
    }
    func stop() {
        buffer.stop()
        lock.lock(); cancelled = true; let encoder = producer; let publisher = sender; let handle = input
        producer = nil; sender = nil; input = nil; lock.unlock()
        if let encoder { terminate(encoder) }
        // Closing the input lets the publisher flush and exit. Terminate also releases blocked writes.
        if let publisher { terminate(publisher) }
        try? handle?.close()
    }
    private func terminate(_ process: Process) {
        if process.isRunning {
            process.terminate()
            DispatchQueue.global().asyncAfter(deadline: .now() + 2) {
                if process.isRunning { _ = kill(process.processIdentifier, SIGKILL) }
            }
        }
    }
    private func redact(_ value: String) -> String {
        var safe = value.replacingOccurrences(of: destination, with: "[server protetto]")
        if let address = URLComponents(string: destination) {
            let key = address.path.split(separator: "/").last.map(String.init) ?? ""
            if key.count > 3 { safe = safe.replacingOccurrences(of: key, with: "[chiave protetta]") }
            for item in address.queryItems ?? [] {
                if let token = item.value, token.count > 3 { safe = safe.replacingOccurrences(of: token, with: "[token protetto]") }
            }
        }
        return safe.replacingOccurrences(of: #"rtmps?://[^\s'\"]+"#, with: "[server protetto]", options: .regularExpression)
    }
    private func drainErrors(_ handle: FileHandle) {
        DispatchQueue.global().async { [self] in
            var buffer = Data()
            while true {
                let bytes = handle.availableData
                if bytes.isEmpty { break }
                buffer.append(bytes)
                while let newline = buffer.firstIndex(of: 10) {
                    log(redact(String(decoding: buffer.prefix(upTo: newline), as: UTF8.self)))
                    buffer.removeSubrange(...newline)
                }
                if buffer.count > 16000 { buffer.removeAll() }
            }
        }
    }
    private func drainLines(_ handle: FileHandle, body: @escaping @Sendable ([String]) -> Void) {
        DispatchQueue.global().async {
            var buffer = Data(); var batch: [String] = []
            while true {
                let bytes = handle.availableData
                if bytes.isEmpty { break }
                buffer.append(bytes)
                while let newline = buffer.firstIndex(of: 10) {
                    let line = String(decoding: buffer.prefix(upTo: newline), as: UTF8.self)
                    buffer.removeSubrange(...newline); batch.append(line)
                    if line.hasPrefix("progress=") { body(batch); batch.removeAll() }
                }
            }
        }
    }
    deinit { stop() }
}
