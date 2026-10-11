import AppKit
import AVFoundation
import CoreGraphics
import Security
import OrbisCore

struct KeychainStore {
    static func read(_ id: UUID) throws -> String {
        var query = base(id); query[kSecReturnData as String] = true; query[kSecMatchLimit as String] = kSecMatchLimitOne
        var result: CFTypeRef?
        let status = SecItemCopyMatching(query as CFDictionary, &result)
        if status == errSecItemNotFound { return "" }
        guard status == errSecSuccess, let data = result as? Data else { throw OrbisError.invalid("Portachiavi non accessibile (\(status)).") }
        return String(decoding: data, as: UTF8.self)
    }
    static func save(_ key: String, id: UUID) throws {
        let query = base(id)
        let attributes = [kSecValueData as String: Data(key.utf8)]
        var status = SecItemUpdate(query as CFDictionary, attributes as CFDictionary)
        if status == errSecItemNotFound {
            var entry = query; entry[kSecValueData as String] = Data(key.utf8)
            entry[kSecAttrAccessible as String] = kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly
            status = SecItemAdd(entry as CFDictionary, nil)
        }
        guard status == errSecSuccess else { throw OrbisError.invalid("Impossibile salvare la chiave nel Portachiavi (\(status)).") }
    }
    static func delete(_ id: UUID) { SecItemDelete(base(id) as CFDictionary) }
    private static func base(_ id: UUID) -> [String: Any] {
        [kSecClass as String: kSecClassGenericPassword, kSecAttrService as String: "com.orbis.stream", kSecAttrAccount as String: id.uuidString]
    }
}
struct CaptureDevice: Identifiable, Hashable {
    var id: String
    var name: String
    var kind: SourceKind
}
enum AppleCapture {
    static func authorize(_ items: [SceneItem]) async throws {
        if items.contains(where: { $0.kind == .screen }) && !CGPreflightScreenCaptureAccess() {
            guard CGRequestScreenCaptureAccess() else { throw OrbisError.invalid("Abilita OrbisStream in Impostazioni di Sistema → Privacy e sicurezza → Registrazione schermo, poi riapri l’app.") }
        }
        for (kind, type) in [(SourceKind.camera, AVMediaType.video), (.microphone, .audio)] where items.contains(where: { $0.kind == kind }) {
            let status = AVCaptureDevice.authorizationStatus(for: type)
            if status == .notDetermined {
                guard await AVCaptureDevice.requestAccess(for: type) else { throw OrbisError.invalid("Accesso a \(kind.title.lowercased()) negato.") }
            } else if status != .authorized { throw OrbisError.invalid("Abilita \(kind.title.lowercased()) nelle impostazioni Privacy e sicurezza.") }
        }
    }
    /// FFmpeg's own list is authoritative: AVCapture discovery and FFmpeg may order devices differently.
    static func devices(ffmpeg: URL) async throws -> [CaptureDevice] {
        let result = try await ProcessRunner.run(ffmpeg, arguments: ["-hide_banner", "-f", "avfoundation", "-list_devices", "true", "-i", ""], timeout: 10)
        let text = String(decoding: result.error, as: UTF8.self)
        var audio = false; var devices: [CaptureDevice] = []
        let pattern = try NSRegularExpression(pattern: #"\[(\d+)\] (.+)$"#)
        for line in text.components(separatedBy: .newlines) {
            if line.contains("AVFoundation audio devices") { audio = true; continue }
            if line.contains("AVFoundation video devices") { audio = false; continue }
            let range = NSRange(line.startIndex..., in: line)
            guard let match = pattern.firstMatch(in: line, range: range), let idRange = Range(match.range(at: 1), in: line), let nameRange = Range(match.range(at: 2), in: line) else { continue }
            let name = String(line[nameRange]); let index = String(line[idRange])
            let kind: SourceKind = audio ? .microphone : name.hasPrefix("Capture screen") ? .screen : .camera
            devices.append(CaptureDevice(id: index, name: name, kind: kind))
        }
        guard !devices.isEmpty else { throw OrbisError.invalid("Nessun dispositivo AVFoundation trovato. Controlla FFmpeg e i permessi di macOS.") }
        return devices
    }
}
@MainActor enum FilePicker {
    @MainActor static func save(name: String) -> URL? {
        let panel = NSSavePanel(); panel.nameFieldStringValue = name
        return panel.runModal() == .OK ? panel.url : nil
    }
    static func files(multiple: Bool = false, folder: Bool = false) -> [URL] {
        let panel = NSOpenPanel(); panel.canChooseDirectories = folder; panel.canChooseFiles = !folder
        panel.allowsMultipleSelection = multiple
        return panel.runModal() == .OK ? panel.urls : []
    }
    static func liveKey(platform: String) -> String? {
        let alert = NSAlert(); alert.messageText = "Chiave per questa diretta \(platform)"
        alert.informativeText = "Inserisci la chiave ricevuta dalla piattaforma. Viene usata soltanto per questa sessione."
        let field = NSSecureTextField(frame: NSRect(x: 0, y: 0, width: 360, height: 24)); alert.accessoryView = field
        alert.addButton(withTitle: "Trasmetti"); alert.addButton(withTitle: "Annulla")
        guard alert.runModal() == .alertFirstButtonReturn, !field.stringValue.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty else { return nil }
        return field.stringValue
    }
    static func exportJSON(_ data: Data, name: String) throws {
        let panel = NSSavePanel(); panel.nameFieldStringValue = name
        if panel.runModal() == .OK, let url = panel.url { try data.write(to: url, options: .atomic) }
    }
}
