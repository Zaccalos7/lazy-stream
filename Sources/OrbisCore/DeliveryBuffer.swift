import Foundation

public struct DeliveryPacket: Sendable {
    public let data: Data
    public let timestamp: UInt32?
    public let segment: UInt64
    public init(data: Data, timestamp: UInt32?, segment: UInt64) {
        self.data = data; self.timestamp = timestamp; self.segment = segment
    }
}
/// A bounded queue: backpressure limits memory even when the publisher stops consuming its pipe.
public final class DeliveryBuffer: @unchecked Sendable {
    private let condition = NSCondition()
    private var queue: [DeliveryPacket] = []
    private var bytes = 0
    private var stopped = false
    private var segment: UInt64 = 0
    private var ended = false
    private var inFlight: UInt64?
    private var latest: UInt32 = 0
    private var sent: UInt32 = 0
    private let maxBytes: Int
    private let maxLead: Double
    public init(maxLead: Double, maxBytes: Int = 32 * 1024 * 1024) { self.maxLead = maxLead; self.maxBytes = maxBytes }
    public var lead: Double {
        condition.lock(); defer { condition.unlock() }; return Double(latest >= sent ? latest - sent : 0) / 1000
    }
    public var sentTimestamp: UInt32 { condition.lock(); defer { condition.unlock() }; return sent }
    public func beginSegment() -> UInt64 {
        condition.lock(); defer { condition.unlock() }
        segment &+= 1; ended = false; condition.broadcast(); return segment
    }
    public func isCurrent(_ token: UInt64) -> Bool {
        condition.lock(); defer { condition.unlock() }; return !stopped && token == segment
    }
    public func enqueue(_ packet: DeliveryPacket) throws {
        guard packet.data.count <= maxBytes else { throw OrbisError.process("Tag FLV oltre il limite del buffer.") }
        condition.lock(); defer { condition.unlock() }
        while !stopped && packet.segment == segment && !queue.isEmpty &&
            (bytes + packet.data.count > maxBytes || Double(latest >= sent ? latest - sent : 0) / 1000 >= maxLead) {
            condition.wait()
        }
        guard !stopped, packet.segment == segment else { throw CancellationError() }
        queue.append(packet); bytes += packet.data.count
        if let timestamp = packet.timestamp { latest = max(latest, timestamp) }
        condition.broadcast()
    }
    public func next() -> DeliveryPacket? {
        condition.lock(); defer { condition.unlock() }
        while queue.isEmpty && !stopped { condition.wait() }
        guard !stopped, !queue.isEmpty else { return nil }
        let packet = queue.removeFirst(); bytes -= packet.data.count; inFlight = packet.segment
        condition.broadcast(); return packet
    }
    public func complete(_ packet: DeliveryPacket, delivered: Bool = true) {
        condition.lock(); defer { condition.unlock() }
        if delivered, let timestamp = packet.timestamp { sent = max(sent, timestamp); latest = max(latest, sent) }
        inFlight = nil; condition.broadcast()
    }
    public func finishAndWait(segment token: UInt64) throws {
        condition.lock(); defer { condition.unlock() }
        ended = true; condition.broadcast()
        while !stopped && token == segment && (queue.contains { $0.segment == token } || inFlight == token) { condition.wait() }
        guard !stopped, token == segment else { throw CancellationError() }
    }
    public func waitForCushion(seconds: Double) {
        condition.lock(); defer { condition.unlock() }
        let deadline = Date().addingTimeInterval(seconds + 2)
        while !stopped && !ended && bytes < maxBytes && Double(latest >= sent ? latest - sent : 0) / 1000 < seconds {
            if !condition.wait(until: deadline) { break }
        }
    }
    /// Discard pending frames at a cut; the packet already being written is allowed to finish.
    public func interrupt() {
        condition.lock(); defer { condition.unlock() }
        segment &+= 1; queue.removeAll(keepingCapacity: true); bytes = 0; latest = sent; ended = true; condition.broadcast()
    }
    public func waitForWriter() {
        condition.lock(); defer { condition.unlock() }
        while !stopped && inFlight != nil { condition.wait() }
    }
    public func stop() {
        condition.lock(); defer { condition.unlock() }
        stopped = true; queue.removeAll(); bytes = 0; condition.broadcast()
    }
}
