import Foundation

public struct DeckHotkey: Equatable, Sendable {
    public let key: String
    public let modifiers: Set<String>
    public init?(_ value: String) {
        let parts = value.uppercased().split(separator: "+", omittingEmptySubsequences: false).map(String.init)
        guard let last = parts.last, !last.isEmpty else { return nil }
        let aliases = ["COMMAND": "CMD", "⌘": "CMD", "ALT": "OPTION", "⌥": "OPTION", "CONTROL": "CTRL", "⌃": "CTRL", "⇧": "SHIFT"]
        let values = parts.dropLast().map { aliases[$0] ?? $0 }
        guard Set(values).count == values.count, values.allSatisfy({ ["CMD", "OPTION", "CTRL", "SHIFT"].contains($0) }) else { return nil }
        let key = last.hasPrefix("KEY") && last.count == 4 ? String(last.suffix(1)) : last.hasPrefix("DIGIT") && last.count == 6 ? String(last.suffix(1)) : last
        let function = key.hasPrefix("F") && Int(key.dropFirst()).map { (1...24).contains($0) } == true
        let character = key.count == 1 && key.unicodeScalars.allSatisfy { CharacterSet.alphanumerics.contains($0) }
        guard function || character || ["ARROWUP", "ARROWDOWN", "ARROWLEFT", "ARROWRIGHT", "PAGEUP", "PAGEDOWN", "HOME", "END"].contains(key) else { return nil }
        let modifiers = Set(values)
        if modifiers.contains("CMD") && ["Q", "W", "N", "R", "F", "P", ",", "H", "M", "TAB", "ARROWRIGHT"].contains(key) { return nil }
        self.key = key; self.modifiers = modifiers
    }
    public var canonical: String {
        (["CMD", "OPTION", "SHIFT", "CTRL"].filter { modifiers.contains($0) } + [key]).joined(separator: "+")
    }
}
