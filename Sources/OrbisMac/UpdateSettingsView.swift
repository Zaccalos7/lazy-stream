import SwiftUI
import OrbisCore

@MainActor struct UpdateSettingsView: View {
    @EnvironmentObject private var model: AppModel
    @AppStorage("updateFeedURL") private var feed = ""
    @AppStorage("autoCheckUpdates") private var automatic = true
    var body: some View {
        Section("Aggiornamenti") {
            Toggle("Verifica aggiornamenti all’avvio", isOn: $automatic)
            TextField("URL del manifest macOS", text: $feed, prompt: Text("Release ufficiali GitHub"))
            HStack {
                Button("Verifica aggiornamenti") { Task { await model.checkUpdates() } }.disabled(model.checkingUpdates)
                if model.checkingUpdates { ProgressView().controlSize(.small) }
                if let release = model.availableUpdate { Link("Scarica installer Apple Silicon", destination: release.url) }
            }
            if !model.updateMessage.isEmpty { Text(model.updateMessage).font(.caption) }
        }
    }
}
