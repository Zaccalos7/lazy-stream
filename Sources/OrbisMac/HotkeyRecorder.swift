import SwiftUI
import AppKit
import OrbisCore

@MainActor enum MacHotkeys {
    static func value(_ event: NSEvent) -> String {
        let flags = event.modifierFlags.intersection(.deviceIndependentFlagsMask)
        let prefix = (flags.contains(.command) ? "Cmd+" : "") + (flags.contains(.option) ? "Option+" : "") + (flags.contains(.shift) ? "Shift+" : "") + (flags.contains(.control) ? "Ctrl+" : "")
        var character = (event.charactersIgnoringModifiers ?? "").uppercased()
        if let scalar = character.unicodeScalars.first {
            if (0xf704...0xf71b).contains(scalar.value) { character = "F\(scalar.value - 0xf704 + 1)" }
            else if let named = [UInt32(0xf700): "ARROWUP", 0xf701: "ARROWDOWN", 0xf702: "ARROWLEFT", 0xf703: "ARROWRIGHT", 0xf72c: "PAGEUP", 0xf72d: "PAGEDOWN", 0xf729: "HOME", 0xf72b: "END"][scalar.value] { character = named }
        }
        return prefix + character
    }
}

@MainActor struct HotkeyRecorder: View {
    @EnvironmentObject private var model: AppModel
    @Binding var value: String
    let buttonID: UUID
    @State private var monitor: Any?
    @State private var recording = false
    @State private var error: String?
    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                TextField("Scorciatoia", text: $value, prompt: Text("Option+1, Cmd+Shift+B"))
                Button(recording ? "Premi una combinazione…" : "Registra tasti") { recording ? stop() : start() }
                Button("Rimuovi") { stop(); value = "" }
            }
            if let error { Text(error).font(.caption).foregroundStyle(.red) }
        }.onDisappear { stop() }
    }
    private func start() {
        stop(); error = nil; recording = true; model.recordingHotkey = true
        monitor = NSEvent.addLocalMonitorForEvents(matching: .keyDown) { event in
            if event.keyCode == 53 { stop(); return nil }
            guard let key = DeckHotkey(MacHotkeys.value(event)) else { error = "Scorciatoia non valida o riservata."; return nil }
            if let other = model.workspace.deck.first(where: { $0.id != buttonID && DeckHotkey($0.hotkey) == key }) {
                error = "Scorciatoia già usata da \(other.label)."; return nil
            }
            value = key.canonical; stop(); return nil
        }
    }
    private func stop() {
        if let monitor { NSEvent.removeMonitor(monitor) }; monitor = nil
        recording = false; model.recordingHotkey = false
    }
}
