#if os(macOS)
import Foundation
import Darwin
import IOKit
public struct SystemSnapshot: Sendable {
    public var cpuPercent: Double?
    public var memoryUsed: UInt64 = 0
    public var memoryTotal: UInt64 = 0
    public var compressed: UInt64 = 0
    public var swapUsed: UInt64 = 0
    public var swapTotal: UInt64 = 0
    public var appMemory: UInt64 = 0
    public var appCPU: Double?
    public var downloadBytesPerSecond: Double?
    public var uploadBytesPerSecond: Double?
    public var diskReadBytesPerSecond: Double?
    public var diskWriteBytesPerSecond: Double?
    public var diskAvailable: Int64 = 0
    public var diskTotal: Int64 = 0
    public var thermalState = "Non disponibile"
    public var lowPower = false
    public var uptime = ProcessInfo.processInfo.systemUptime
    public init() {}
}
public final class SystemSampler {
    private var previous: NativeCounters?
    private var previousTime: TimeInterval?
    public init() {}
    public func sample(volume: URL) -> SystemSnapshot {
        var result = SystemSnapshot()
        let current = NativeCounters.read(); let now = ProcessInfo.processInfo.systemUptime
        if let old = previous, let last = previousTime, now > last {
            // Counters wrap at 32 bits; wrapping subtraction prevents spikes.
            let user = UInt32(truncatingIfNeeded: current.cpu_user) &- UInt32(truncatingIfNeeded: old.cpu_user)
            let system = UInt32(truncatingIfNeeded: current.cpu_system) &- UInt32(truncatingIfNeeded: old.cpu_system)
            let nice = UInt32(truncatingIfNeeded: current.cpu_nice) &- UInt32(truncatingIfNeeded: old.cpu_nice)
            let idle = UInt32(truncatingIfNeeded: current.cpu_idle) &- UInt32(truncatingIfNeeded: old.cpu_idle)
            let busy = Double(user) + Double(system) + Double(nice); let total = busy + Double(idle)
            if current.cpu_valid != 0 && old.cpu_valid != 0 && total > 0 { result.cpuPercent = busy / total * 100 }
            if current.disk_valid != 0 && old.disk_valid != 0 && current.disk_devices == old.disk_devices && current.disk_read >= old.disk_read && current.disk_written >= old.disk_written {
                result.diskReadBytesPerSecond = Double(current.disk_read - old.disk_read) / (now - last)
                result.diskWriteBytesPerSecond = Double(current.disk_written - old.disk_written) / (now - last)
            }
            result.appCPU = max(0, (current.app_cpu_seconds - old.app_cpu_seconds) / (now - last) * 100)
            if current.network_valid != 0 && old.network_valid != 0 {
                result.downloadBytesPerSecond = Double(UInt32(truncatingIfNeeded: current.network_in) &- UInt32(truncatingIfNeeded: old.network_in)) / (now - last)
                result.uploadBytesPerSecond = Double(UInt32(truncatingIfNeeded: current.network_out) &- UInt32(truncatingIfNeeded: old.network_out)) / (now - last)
            }
        }
        result.memoryUsed = current.memory_used; result.memoryTotal = current.memory_total; result.compressed = current.memory_compressed
        result.swapUsed = current.swap_used; result.swapTotal = current.swap_total; result.appMemory = current.app_resident
        previous = current; previousTime = now
        let states = ["Nominale", "Moderato", "Elevato", "Critico"]
        result.thermalState = states[min(3, ProcessInfo.processInfo.thermalState.rawValue)]
        result.lowPower = ProcessInfo.processInfo.isLowPowerModeEnabled
        if let values = try? volume.resourceValues(forKeys: [.volumeAvailableCapacityKey, .volumeTotalCapacityKey]) {
            result.diskAvailable = Int64(values.volumeAvailableCapacity ?? 0); result.diskTotal = Int64(values.volumeTotalCapacity ?? 0)
        }
        return result
    }
}

private struct NativeCounters {
    var cpu_user: UInt32 = 0, cpu_system: UInt32 = 0, cpu_idle: UInt32 = 0, cpu_nice: UInt32 = 0
    var cpu_valid = 0, disk_valid = 0, disk_devices = 0, network_valid = 0
    var memory_used: UInt64 = 0, memory_total: UInt64 = 0, memory_compressed: UInt64 = 0
    var swap_used: UInt64 = 0, swap_total: UInt64 = 0, app_resident: UInt64 = 0
    var network_in: UInt64 = 0, network_out: UInt64 = 0, disk_read: UInt64 = 0, disk_written: UInt64 = 0
    var app_cpu_seconds = 0.0

    static func read() -> Self {
        var result = Self()
        let host = mach_host_self()
        defer { mach_port_deallocate(mach_task_self_, host) }
        var cpu = host_cpu_load_info_data_t()
        var count = mach_msg_type_number_t(MemoryLayout.size(ofValue: cpu) / MemoryLayout<integer_t>.size)
        let cpuStatus = withUnsafeMutablePointer(to: &cpu) {
            $0.withMemoryRebound(to: integer_t.self, capacity: Int(count)) { host_statistics(host, HOST_CPU_LOAD_INFO, $0, &count) }
        }
        if cpuStatus == KERN_SUCCESS {
            result.cpu_user = cpu.cpu_ticks.0; result.cpu_system = cpu.cpu_ticks.1
            result.cpu_idle = cpu.cpu_ticks.2; result.cpu_nice = cpu.cpu_ticks.3; result.cpu_valid = 1
        }
        var total: UInt64 = 0
        var length = MemoryLayout.size(ofValue: total)
        if sysctlbyname("hw.memsize", &total, &length, nil, 0) == 0 { result.memory_total = total }
        var vm = vm_statistics64_data_t()
        count = mach_msg_type_number_t(MemoryLayout.size(ofValue: vm) / MemoryLayout<integer_t>.size)
        var page: vm_size_t = 0
        host_page_size(host, &page)
        let vmStatus = withUnsafeMutablePointer(to: &vm) {
            $0.withMemoryRebound(to: integer_t.self, capacity: Int(count)) { host_statistics64(host, HOST_VM_INFO64, $0, &count) }
        }
        if vmStatus == KERN_SUCCESS {
            let active = UInt64(vm.active_count), purgeable = min(active, UInt64(vm.purgeable_count))
            result.memory_compressed = UInt64(vm.compressor_page_count) * UInt64(page)
            result.memory_used = min(total, (active + UInt64(vm.wire_count) + UInt64(vm.compressor_page_count) - purgeable) * UInt64(page))
        }
        var swap = xsw_usage()
        length = MemoryLayout.size(ofValue: swap)
        if sysctlbyname("vm.swapusage", &swap, &length, nil, 0) == 0 { result.swap_used = swap.xsu_used; result.swap_total = swap.xsu_total }
        var task = mach_task_basic_info_data_t()
        count = mach_msg_type_number_t(MemoryLayout.size(ofValue: task) / MemoryLayout<natural_t>.size)
        let taskStatus = withUnsafeMutablePointer(to: &task) {
            $0.withMemoryRebound(to: integer_t.self, capacity: Int(count)) { task_info(mach_task_self_, task_flavor_t(MACH_TASK_BASIC_INFO), $0, &count) }
        }
        if taskStatus == KERN_SUCCESS { result.app_resident = task.resident_size }
        var usage = rusage()
        if getrusage(RUSAGE_SELF, &usage) == 0 {
            result.app_cpu_seconds = Double(usage.ru_utime.tv_sec) + Double(usage.ru_utime.tv_usec) / 1e6 + Double(usage.ru_stime.tv_sec) + Double(usage.ru_stime.tv_usec) / 1e6
        }
        var interfaces: UnsafeMutablePointer<ifaddrs>?
        if getifaddrs(&interfaces) == 0 {
            defer { freeifaddrs(interfaces) }
            var cursor = interfaces
            while let entry = cursor {
                let item = entry.pointee
                if let address = item.ifa_addr, Int32(address.pointee.sa_family) == AF_LINK,
                   item.ifa_flags & UInt32(IFF_UP) != 0, item.ifa_flags & UInt32(IFF_LOOPBACK) == 0,
                   String(cString: item.ifa_name).hasPrefix("en"), let data = item.ifa_data {
                    let counters = data.assumingMemoryBound(to: if_data.self).pointee
                    result.network_in += UInt64(counters.ifi_ibytes); result.network_out += UInt64(counters.ifi_obytes)
                }
                cursor = item.ifa_next
            }
            result.network_valid = 1
        }
        var iterator: io_iterator_t = 0
        if IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching("IOBlockStorageDriver"), &iterator) == KERN_SUCCESS {
            defer { IOObjectRelease(iterator) }
            while true {
                let drive = IOIteratorNext(iterator)
                guard drive != 0 else { break }
                defer { IOObjectRelease(drive) }
                if let property = IORegistryEntryCreateCFProperty(drive, "Statistics" as CFString, kCFAllocatorDefault, 0),
                   let statistics = property.takeRetainedValue() as? [String: Any],
                   let read = statistics["Bytes (Read)"] as? NSNumber, let written = statistics["Bytes (Write)"] as? NSNumber {
                    result.disk_read += UInt64(max(0, read.int64Value)); result.disk_written += UInt64(max(0, written.int64Value))
                    result.disk_devices += 1; result.disk_valid = 1
                }
            }
        }
        return result
    }
}
#endif
