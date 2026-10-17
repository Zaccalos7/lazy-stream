import Foundation

/// Rebase FLV tags so multiple encoders share one publish connection.
/// Each producer starts with its own header and DTS zero. Only the first header
/// reaches the sender; codec sequence headers remain intact on every transition.
public struct FLVRebaser {
    private var buffer = Data()
    private var headerRead = false
    private var sentHeader = false
    private var offset: UInt32 = 0
    private var firstTimestamp: UInt32?
    private var lastTimestamp: UInt32 = 0
    public init() {}
    public mutating func beginSegment(after timestamp: UInt32? = nil, frameMilliseconds: UInt32 = 34) {
        if let timestamp { lastTimestamp = timestamp }
        buffer.removeAll(keepingCapacity: true); headerRead = false; firstTimestamp = nil
        offset = sentHeader ? lastTimestamp &+ max(1, frameMilliseconds) : 0
    }
    public mutating func consume(_ bytes: Data) throws -> [(data: Data, timestamp: UInt32?)] {
        buffer.append(bytes)
        var output: [(Data, UInt32?)] = []
        if !headerRead {
            guard buffer.count >= 9 else { return [] }
            guard Array(buffer.prefix(3)) == [70, 76, 86] else { throw OrbisError.process("Il produttore non ha restituito un flusso FLV.") }
            let size = Int(read32(buffer, 5))
            guard size >= 9 && size <= 1024 else { throw OrbisError.process("Header FLV non valido.") }
            guard buffer.count >= size + 4 else { return [] }
            if !sentHeader { output.append((Data(buffer.prefix(size + 4)), nil)); sentHeader = true }
            buffer.removeFirst(size + 4); buffer = Data(buffer); headerRead = true
        }
        while buffer.count >= 11 {
            let size = (Int(buffer[1]) << 16) | (Int(buffer[2]) << 8) | Int(buffer[3])
            guard size <= 32 * 1024 * 1024 else { throw OrbisError.process("Tag FLV troppo grande.") }
            let total = 11 + size + 4
            guard buffer.count >= total else { break }
            var tag = Data(buffer.prefix(total))
            guard read32(tag, total - 4) == UInt32(11 + size) else { throw OrbisError.process("Tag FLV corrotto.") }
            let type = tag[0]
            let source = (UInt32(tag[7]) << 24) | (UInt32(tag[4]) << 16) | (UInt32(tag[5]) << 8) | UInt32(tag[6])
            if type == 8 || type == 9 {
                if firstTimestamp == nil { firstTimestamp = source }
                let rebased = offset &+ (source >= firstTimestamp! ? source - firstTimestamp! : 0)
                tag[4] = UInt8(truncatingIfNeeded: rebased >> 16); tag[5] = UInt8(truncatingIfNeeded: rebased >> 8)
                tag[6] = UInt8(truncatingIfNeeded: rebased); tag[7] = UInt8(truncatingIfNeeded: rebased >> 24)
                lastTimestamp = max(lastTimestamp, rebased)
                output.append((tag, rebased))
            } else if offset == 0 { output.append((tag, nil)) }
            buffer.removeFirst(total); buffer = Data(buffer)
        }
        return output
    }
    private func read32(_ data: Data, _ at: Int) -> UInt32 {
        (UInt32(data[at]) << 24) | (UInt32(data[at + 1]) << 16) | (UInt32(data[at + 2]) << 8) | UInt32(data[at + 3])
    }
}
public final class WorkspaceStore: @unchecked Sendable {
    public let directory: URL
    public var document: URL { directory.appendingPathComponent("workspace.json") }
    public init(directory: URL? = nil) throws {
        self.directory = try directory ?? FileManager.default.url(for: .applicationSupportDirectory, in: .userDomainMask, appropriateFor: nil, create: true).appendingPathComponent("OrbisStream", isDirectory: true)
        try FileManager.default.createDirectory(at: self.directory, withIntermediateDirectories: true, attributes: [.posixPermissions: 0o700])
    }
    public func load() throws -> Workspace {
        guard FileManager.default.fileExists(atPath: document.path) else { return Workspace() }
        do { let workspace = try JSONDecoder().decode(Workspace.self, from: Data(contentsOf: document)); try workspace.validate(); return workspace }
        catch {
            let backup = directory.appendingPathComponent("workspace-recovery-\(UUID().uuidString).json")
            try FileManager.default.copyItem(at: document, to: backup)
            throw OrbisError.invalid("Configurazione non leggibile. Una copia è conservata in \(backup.path): \(error.localizedDescription)")
        }
    }
    public func save(_ workspace: Workspace) throws {
        let encoder = JSONEncoder(); encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        try encoder.encode(workspace).write(to: document, options: .atomic)
        try FileManager.default.setAttributes([.posixPermissions: 0o600], ofItemAtPath: document.path)
    }
    public func importMedia(_ source: URL, collection: String) throws -> String {
        guard !collection.isEmpty, collection.allSatisfy({ $0.isLetter || $0.isNumber || "_-".contains($0) }) else { throw OrbisError.invalid("Libreria non valida.") }
        let attributes = try source.resourceValues(forKeys: [.isRegularFileKey, .fileSizeKey])
        guard attributes.isRegularFile == true else { throw OrbisError.invalid("Seleziona un file.") }
        let ext = source.pathExtension.lowercased()
        if collection == "Overlays" {
            guard MediaFiles.overlays.contains(ext), (attributes.fileSize ?? 0) <= 100 * 1024 * 1024 else { throw OrbisError.invalid("Overlay non valido o superiore a 100 MiB.") }
        } else if collection == "Deck" {
            guard MediaFiles.videos.contains(ext) || MediaFiles.isPicture(source.path), (attributes.fileSize ?? 0) <= 2 * 1024 * 1024 * 1024 else { throw OrbisError.invalid("Media non valido o superiore a 2 GiB.") }
        }
        let folder = directory.appendingPathComponent(collection, isDirectory: true)
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true, attributes: [.posixPermissions: 0o700])
        for file in try FileManager.default.contentsOfDirectory(at: folder, includingPropertiesForKeys: [.isRegularFileKey]) where file.pathExtension.lowercased() == ext {
            if FileManager.default.contentsEqual(atPath: source.path, andPath: file.path) { return file.path }
        }
        let destination = folder.appendingPathComponent(UUID().uuidString + "-" + source.lastPathComponent)
        try FileManager.default.copyItem(at: source, to: destination)
        try FileManager.default.setAttributes([.posixPermissions: 0o600], ofItemAtPath: destination.path)
        return destination.path
    }
    public func sweepMedia(collection: String, keeping: Set<String>, olderThan: Date) throws {
        guard ["Overlays", "Deck"].contains(collection) else { return }
        let folder = directory.appendingPathComponent(collection, isDirectory: true)
        guard FileManager.default.fileExists(atPath: folder.path) else { return }
        for file in try FileManager.default.contentsOfDirectory(at: folder, includingPropertiesForKeys: [.creationDateKey, .contentModificationDateKey, .isRegularFileKey]) {
            guard !keeping.contains(file.path), UUID(uuidString: String(file.lastPathComponent.prefix(36))) != nil else { continue }
            let values = try file.resourceValues(forKeys: [.creationDateKey, .contentModificationDateKey, .isRegularFileKey])
            let date = values.creationDate ?? values.contentModificationDate ?? Date()
            if values.isRegularFile == true && date < olderThan { try FileManager.default.removeItem(at: file) }
        }
    }
}
