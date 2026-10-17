#if os(macOS)
import Foundation
import Darwin

public struct ProcessSnapshot: Identifiable, Sendable {
    public var id: Int32 { pid }
    public let pid: Int32
    public let cpuPercent: Double?
    public let memory: UInt64
    public let threads: Int32
}
/// Sample only the application's own child PIDs; permissions failures remain unavailable.
public final class ProcessSampler {
    private var previous: [Int32: (ticks: UInt64, time: Double)] = [:]
    private var timebase = mach_timebase_info_data_t()
    public init() { mach_timebase_info(&timebase) }
    public func sample(pids: [Int32]) -> [Int32: ProcessSnapshot] {
        let now = ProcessInfo.processInfo.systemUptime
        previous = previous.filter { pids.contains($0.key) }
        var output: [Int32: ProcessSnapshot] = [:]
        for pid in pids {
            var info = proc_taskinfo()
            let size = Int32(MemoryLayout.size(ofValue: info))
            guard proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &info, size) == size else { continue }
            let ticks = info.pti_total_user &+ info.pti_total_system
            var cpu: Double?
            // XNU fill_taskprocinfo exports recount_times_mach here, so these are Mach ticks.
            // https://github.com/apple-oss-distributions/xnu/blob/main/osfmk/kern/bsd_kern.c
            if let old = previous[pid], now > old.time, ticks >= old.ticks, timebase.denom > 0 {
                let seconds = Double(ticks - old.ticks) * Double(timebase.numer) / Double(timebase.denom) / 1e9
                cpu = seconds / (now - old.time) * 100
            }
            previous[pid] = (ticks, now)
            output[pid] = ProcessSnapshot(pid: pid, cpuPercent: cpu, memory: info.pti_resident_size, threads: info.pti_threadnum)
        }
        return output
    }
}
#endif
