import Foundation

public struct StreamingPolicy: Sendable {
    public let preroll: Double
    public let maxLead: Double
    public let connectTimeout: Double
    public let stallTimeout: Double
    public let senderPacing: Bool
    public let uniformFormat: Bool
    public let adaptation: AdaptationRules?
    public init(platform: Platform) {
        switch platform {
        case .twitch, .kick:
            preroll = 0; maxLead = 1; connectTimeout = 20; stallTimeout = 15
            senderPacing = false; uniformFormat = platform == .kick; adaptation = .lowLatency
        case .youtube, .facebook, .tiktok:
            preroll = 2; maxLead = platform == .tiktok ? 3 : 4; connectTimeout = 30; stallTimeout = 20
            senderPacing = true; uniformFormat = true; adaptation = .buffered
        case .custom:
            preroll = 0; maxLead = 1; connectTimeout = 30; stallTimeout = 30
            senderPacing = false; uniformFormat = false; adaptation = nil
        }
    }
}
public struct AdaptationRules: Sendable {
    public var window: Double
    public var maxDrift: Double
    public var probeAfter: Double
    public static let lowLatency = Self(window: 6, maxDrift: 2, probeAfter: 120)
    public static let buffered = Self(window: 10, maxDrift: 6, probeAfter: 180)
}
public struct DeliverySample: Sendable {
    public var time: Double
    public var media: Double
    public var bytes: Int64
    public var lead: Double
    public init(time: Double, media: Double, bytes: Int64, lead: Double) {
        self.time = time; self.media = media; self.bytes = bytes; self.lead = lead
    }
}
public struct RateDecision: Sendable {
    public let from: Int
    public let to: Int
    public let throughput: Int
}
/// Drift and encoder backlog distinguish a slow network from an encoder that is starving the queue.
public struct BitrateController: Sendable {
    public private(set) var cap: Int?
    private let rules: AdaptationRules?
    private let backlog: Double
    private var samples: [DeliverySample] = []
    private var origin: DeliverySample?
    private var bestAhead = 0.0
    private var settleUntil: Double?
    private var cleanSince = 0.0
    private var lastChange = 0.0
    private var probeAfter: Double
    private var probing = false
    public init(policy: StreamingPolicy) {
        rules = policy.adaptation; backlog = policy.maxLead * 0.5; probeAfter = policy.adaptation?.probeAfter ?? 0
    }
    public mutating func observe(_ sample: DeliverySample, nominal: Int, audioBitrate: Int) -> RateDecision? {
        guard let rules, nominal > 0 else { return nil }
        if let cap, cap >= nominal { self.cap = nil }
        if settleUntil == nil { lastChange = sample.time; settle(sample.time); return nil }
        guard sample.time >= settleUntil! else { return nil }
        if let last = samples.last {
            if sample.media < last.media || sample.bytes < last.bytes { settle(sample.time); return nil }
            if sample.time - last.time < 0.2 { return nil }
        }
        if origin == nil { origin = sample; bestAhead = 0 }
        samples.append(sample)
        samples.removeAll { sample.time - $0.time > rules.window }
        if samples.count > 128 { samples.removeFirst(samples.count - 128) }
        let ahead = sample.media - origin!.media - (sample.time - origin!.time)
        bestAhead = max(bestAhead, ahead)
        let drift = bestAhead - ahead
        guard let oldest = samples.first, sample.time - oldest.time >= rules.window * 0.8,
              let recent = samples.first(where: { sample.time - $0.time <= rules.window / 3 }), sample.time > recent.time else { return nil }
        let rate = (sample.media - oldest.media) / (sample.time - oldest.time)
        let recentRate = (sample.media - recent.media) / (sample.time - recent.time)
        let throughput = Int(max(0, Double(sample.bytes - recent.bytes) * 8 / (sample.time - recent.time)))
        let lead = samples.reduce(0) { $0 + $1.lead } / Double(samples.count)
        if lead >= backlog && recentRate >= 0.1 && recentRate <= 1 && drift >= rules.maxDrift {
            cleanSince = sample.time
            let current = cap ?? nominal
            let floor = min(nominal, max(500_000, Int(Double(nominal) * 0.3)))
            let carried = Int(Double(throughput) * 0.85) - audioBitrate
            var target = min(Int(Double(current) * 0.85), max(Int(Double(current) * 0.5), carried))
            target = max(floor, target - target % 10_000)
            guard target < current else { return nil }
            if probing {
                probeAfter = sample.time - lastChange < probeAfter ? min(probeAfter * 2, 960) : rules.probeAfter
                probing = false
            }
            cap = target; lastChange = sample.time; settle(sample.time)
            return RateDecision(from: current, to: target, throughput: throughput)
        }
        if drift >= 0.5 || rate < 0.97 { cleanSince = sample.time }
        guard let cap, sample.time - cleanSince >= probeAfter, sample.time - lastChange >= probeAfter else { return nil }
        if probing { probeAfter = rules.probeAfter }
        var target = cap + Int(Double(nominal) * 0.2); target -= target % 10_000
        self.cap = target < nominal ? target : nil
        probing = true; lastChange = sample.time; settle(sample.time)
        return RateDecision(from: cap, to: self.cap ?? nominal, throughput: throughput)
    }
    private mutating func settle(_ time: Double) {
        samples.removeAll(keepingCapacity: true); origin = nil; bestAhead = 0
        settleUntil = time + 5; cleanSince = time
    }
}

public struct StreamingProcess: Identifiable, Sendable {
    public var id: Int32 { pid }
    public let role: String
    public let pid: Int32
    public init(role: String, pid: Int32) { self.role = role; self.pid = pid }
}
