import SwiftUI
import AppKit
import CoreImage.CIFilterBuiltins
import OrbisCore

@MainActor struct ErrorReportView: View {
    @EnvironmentObject private var model: AppModel
    @Environment(\.dismiss) private var dismiss
    let history: LiveHistory
    @State private var text = ""
    @State private var parts: [String] = []
    @State private var index = 0
    @State private var localError: String?
    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            HStack { Text("Segnala errore").font(.title2.bold()); Spacer(); Button("Chiudi") { dismiss() } }
            if let image = qrImage {
                Image(nsImage: image).interpolation(.none).resizable().scaledToFit().frame(maxWidth: .infinity, maxHeight: 280)
                    .padding(20).background(.white).accessibilityLabel("QR code contenente la segnalazione dell’errore")
            }
            if parts.count > 1 {
                HStack {
                    Button("Precedente") { index -= 1 }.disabled(index == 0)
                    Text("\(index + 1) / \(parts.count)").monospacedDigit()
                    Button("Successivo") { index += 1 }.disabled(index + 1 >= parts.count)
                }.frame(maxWidth: .infinity)
            }
            Text("Scansiona i codici QR o copia il testo da allegare alla segnalazione.").font(.callout)
            ScrollView { Text(text).font(.system(.caption, design: .monospaced)).textSelection(.enabled).frame(maxWidth: .infinity, alignment: .leading) }
                .padding(12).background(.quaternary, in: RoundedRectangle(cornerRadius: 12))
            if let localError { Text(localError).font(.caption).foregroundStyle(.red) }
            HStack {
                Button("Copia testo") { NSPasteboard.general.clearContents(); NSPasteboard.general.setString(text, forType: .string) }
                Button("Esporta…") {
                    guard let url = FilePicker.save(name: "OrbisStream-Segnalazione.txt") else { return }
                    do { try text.write(to: url, atomically: true, encoding: .utf8) }
                    catch { localError = error.localizedDescription }
                }
                Spacer()
                Link("Apri segnalazione", destination: URL(string: "https://github.com/Zaccalos7/lazy-stream-mac/issues/new")!)
            }
        }.padding(28).frame(width: 700, height: 720).onAppear { buildReport() }
    }
    private var qrImage: NSImage? {
        guard parts.indices.contains(index) else { return nil }
        let filter = CIFilter.qrCodeGenerator()
        filter.message = Data("Orbis report \(index + 1)/\(parts.count)\n\(parts[index])".utf8)
        filter.correctionLevel = "M"
        guard let output = filter.outputImage?.transformed(by: CGAffineTransform(scaleX: 5, y: 5)),
              let cgImage = CIContext().createCGImage(output, from: output.extent) else { return nil }
        return NSImage(cgImage: cgImage, size: NSSize(width: CGFloat(cgImage.width), height: CGFloat(cgImage.height)))
    }
    private func buildReport() {
        let version = Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "3.0.0"
        let logs = model.sessions.first(where: { $0.historyID == history.id })?.logs.suffix(80).joined(separator: "\n") ??
            (try? SessionLog.read(directory: model.store.directory.appendingPathComponent("Logs"), sessionID: history.id, maxBytes: 12_000)) ?? ""
        let keys: [String]
        do { keys = try model.workspace.channels.map { try KeychainStore.read($0.id) } + [try KeychainStore.read(history.channel.id)] }
        catch { localError = error.localizedDescription; return }
        text = ErrorReport.redact("""
        OrbisStream \(version) · macOS Apple Silicon
        \(ProcessInfo.processInfo.operatingSystemVersionString)
        Diretta \(history.id) · \(model.stateTitle(history))
        \(history.start.ISO8601Format())
        \(history.channel.platform.title) · \(history.profile.codec) · \(history.profile.width)×\(history.profile.height)

        \(history.error ?? "")
        \(logs)
        """, keys: keys)
        parts = ErrorReport.parts(text)
    }
}
