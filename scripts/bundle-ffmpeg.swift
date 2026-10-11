import Foundation

// Copy only arm64 slices and relocate every non-system dynamic dependency.
struct BundleError: Error, CustomStringConvertible { let description: String }
let files = FileManager.default
func run(_ tool: String, _ arguments: [String], allowFailure: Bool = false) throws -> String {
    let process = Process(), output = Pipe()
    process.executableURL = URL(fileURLWithPath: tool); process.arguments = arguments
    process.standardOutput = output; process.standardError = FileHandle.standardError
    try process.run()
    let data = output.fileHandleForReading.readDataToEndOfFile()
    process.waitUntilExit()
    if process.terminationStatus != 0 && !allowFailure { throw BundleError(description: "\(tool) fallito: \(arguments.joined(separator: " "))") }
    return String(decoding: data, as: UTF8.self)
}
func systemLibrary(_ path: String) -> Bool { path.hasPrefix("/System/Library/") || path.hasPrefix("/usr/lib/") }
func expanded(_ path: String, loader: URL, executable: URL) -> String {
    path.replacingOccurrences(of: "@loader_path", with: loader.deletingLastPathComponent().path)
        .replacingOccurrences(of: "@executable_path", with: executable.deletingLastPathComponent().path)
}
func rpaths(_ binary: URL, executable: URL) throws -> [String] {
    let lines = try run("/usr/bin/otool", ["-l", binary.path]).components(separatedBy: .newlines)
    var paths: [String] = [], expecting = false
    for line in lines {
        let text = line.trimmingCharacters(in: .whitespaces)
        if text == "cmd LC_RPATH" { expecting = true }
        else if expecting && text.hasPrefix("path "), let end = text.range(of: " (offset") {
            paths.append(expanded(String(text[text.index(text.startIndex, offsetBy: 5)..<end.lowerBound]), loader: binary, executable: executable)); expecting = false
        }
    }
    return paths
}
var copied: [String: URL] = [:]
func copyBinary(_ source: URL, to destination: URL) throws {
    let architectures = try run("/usr/bin/lipo", ["-archs", source.path]).split(whereSeparator: \.isWhitespace)
    guard architectures.contains("arm64") else { throw BundleError(description: "Manca arm64: \(source.path)") }
    try files.copyItem(at: source, to: destination)
    _ = try run("/usr/bin/codesign", ["--remove-signature", destination.path], allowFailure: true)
    if architectures.count > 1 {
        let thin = destination.appendingPathExtension("thin")
        _ = try run("/usr/bin/lipo", [destination.path, "-thin", "arm64", "-output", thin.path])
        try files.removeItem(at: destination); try files.moveItem(at: thin, to: destination)
    }
    try files.setAttributes([.posixPermissions: 0o755], ofItemAtPath: destination.path)
}
func relocate(_ source: URL, destination: URL, frameworks: URL, executable: URL, inherited: [String] = []) throws {
    let search = try rpaths(source, executable: executable) + inherited
    let dependencies = try run("/usr/bin/otool", ["-L", source.path]).components(separatedBy: .newlines).dropFirst()
    let identity = try run("/usr/bin/otool", ["-D", source.path], allowFailure: true).components(separatedBy: .newlines).dropFirst().first ?? ""
    for line in dependencies {
        guard let end = line.range(of: " (compatibility version") else { continue }
        let name = String(line[..<end.lowerBound]).trimmingCharacters(in: .whitespaces)
        if name == identity || systemLibrary(name) { continue }
        var candidates: [String]
        if name.hasPrefix("@rpath/") { candidates = search.map { $0 + "/" + name.dropFirst(7) } }
        else { candidates = [expanded(name, loader: source, executable: executable)] }
        guard let resolved = candidates.first(where: { files.fileExists(atPath: $0) }) else {
            throw BundleError(description: "Dipendenza non trovata: \(name) in \(source.path)")
        }
        if systemLibrary(resolved) {
            _ = try run("/usr/bin/install_name_tool", ["-change", name, resolved, destination.path]); continue
        }
        let library = URL(fileURLWithPath: resolved).resolvingSymlinksInPath(), basename = library.lastPathComponent
        let target = frameworks.appendingPathComponent(basename)
        if let previous = copied[basename] {
            guard previous == library else { throw BundleError(description: "Librerie con lo stesso nome: \(basename)") }
        } else {
            copied[basename] = library
            try copyBinary(library, to: target)
            _ = try run("/usr/bin/install_name_tool", ["-id", "@rpath/" + basename, target.path])
            try relocate(library, destination: target, frameworks: frameworks, executable: executable, inherited: search)
        }
        let replacement = destination.deletingLastPathComponent() == frameworks ? "@loader_path/" + basename : "@loader_path/../Frameworks/" + basename
        _ = try run("/usr/bin/install_name_tool", ["-change", name, replacement, destination.path])
    }
}
do {
    guard CommandLine.arguments.count == 3 else { throw BundleError(description: "Uso: bundle-ffmpeg.swift cartella-binari app") }
    let source = URL(fileURLWithPath: CommandLine.arguments[1]), app = URL(fileURLWithPath: CommandLine.arguments[2])
    let helpers = app.appendingPathComponent("Contents/Helpers"), frameworks = app.appendingPathComponent("Contents/Frameworks")
    try files.createDirectory(at: helpers, withIntermediateDirectories: true)
    try files.createDirectory(at: frameworks, withIntermediateDirectories: true)
    for name in ["ffmpeg", "ffprobe"] {
        let original = source.appendingPathComponent(name).resolvingSymlinksInPath(), target = helpers.appendingPathComponent(name)
        try copyBinary(original, to: target)
        try relocate(original, destination: target, frameworks: frameworks, executable: original)
    }
} catch { fputs("\(error)\n", stderr); exit(1) }
