import Foundation

public struct CleanupProgress: Sendable {
    public var running = false
    public var totalVideos = 0
    public var deletedVideos = 0
    public var deletedLives = 0
    public var currentVideo = ""
    public var error: String?
    public init() {}
}
public enum HistoryCleanup {
    /// -1: everything, 0: before yesterday, positive: age in calendar months.
    public static func cutoff(months: Int, now: Date = Date(), calendar: Calendar = .current) -> Date? {
        if months == -1 { return now }
        if months == 0 { return calendar.date(byAdding: .day, value: -1, to: now) }
        guard months > 0 else { return nil }
        return calendar.date(byAdding: .month, value: -months, to: now)
    }
    public static func eligible(_ history: [LiveHistory], months: Int, active: Set<UUID>, now: Date = Date()) -> [LiveHistory] {
        guard let limit = cutoff(months: months, now: now) else { return [] }
        return history.filter { $0.start < limit && !active.contains($0.id) }
    }
}
