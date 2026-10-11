import Foundation

/// Daily files survive application restarts; writes never block the SwiftUI actor.
public final class SessionLog: @unchecked Sendable {
    private let queue = DispatchQueue(label: "com.orbis.stream.log", qos: .utility)
    private let directory: URL
    private let lock = NSLock()
    private var pending = 0
    private var lastCleanup = ""
    public init(directory: URL) { self.directory = directory }
    public func append(_ message: String) {
        lock.lock(); guard pending < 10_000 else { lock.unlock(); return }; pending += 1; lock.unlock()
        let now = Date()
        queue.async { [self] in
            defer { lock.lock(); pending -= 1; lock.unlock() }
            do {
                try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
                let formatter = DateFormatter(); formatter.locale = Locale(identifier: "en_US_POSIX"); formatter.dateFormat = "yyyy-MM-dd"
                let day = formatter.string(from: now)
                if lastCleanup != day {
                    lastCleanup = day
                    let cutoff = Calendar.current.date(byAdding: .day, value: -2, to: Calendar.current.startOfDay(for: now)) ?? now
                    let logs = (try? FileManager.default.contentsOfDirectory(at: directory, includingPropertiesForKeys: nil)) ?? []
                    for file in logs where file.lastPathComponent.hasPrefix("OrbisStream-") && file.pathExtension == "log" {
                        let date = String(file.deletingPathExtension().lastPathComponent.dropFirst("OrbisStream-".count))
                        if let written = formatter.date(from: date), written < cutoff { try? FileManager.default.removeItem(at: file) }
                    }
                }
                let path = directory.appendingPathComponent("OrbisStream-\(formatter.string(from: now)).log")
                if !FileManager.default.fileExists(atPath: path.path) {
                    FileManager.default.createFile(atPath: path.path, contents: nil, attributes: [.posixPermissions: 0o600])
                }
                let handle = try FileHandle(forWritingTo: path); defer { try? handle.close() }
                try handle.seekToEnd()
                let line = "\(ISO8601DateFormatter().string(from: now)) \(message)\n"
                try handle.write(contentsOf: Data(line.utf8))
            } catch {
                // Report to the system console without writing a potentially secret message twice.
                NSLog("OrbisStream: registrazione su disco non disponibile: %@", error.localizedDescription)
            }
        }
    }
    public func flush() { queue.sync {} }
}
