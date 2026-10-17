import SwiftUI
import OrbisCore

@MainActor struct HistoryCleanupSettings: View {
    @EnvironmentObject private var model: AppModel
    var channelID: UUID?
    private var channel: Channel? {
        if let channelID { return model.workspace.channels.first { $0.id == channelID } }
        return model.workspace.channels.first
    }
    var body: some View {
        if let channel {
            VStack(alignment: .leading, spacing: 12) {
                Text(channel.name).font(.headline)
                Toggle("Pulizia automatica", isOn: Binding(get: { self.channel?.cleanupEnabled ?? false }, set: { value in update(channel.id) { $0.cleanupEnabled = value } }))
                Stepper("Intervallo: \(channel.cleanupIntervalMonths) mesi", value: Binding(get: { self.channel?.cleanupIntervalMonths ?? 1 }, set: { value in update(channel.id) { $0.cleanupIntervalMonths = value } }), in: 1...24)
                Stepper("Elimina record oltre \(channel.cleanupOlderThanMonths) mesi", value: Binding(get: { self.channel?.cleanupOlderThanMonths ?? 6 }, set: { value in update(channel.id) { $0.cleanupOlderThanMonths = value } }), in: 1...120)
                if let last = channel.lastCleanup { Text("Ultima pulizia: \(last.formatted())").font(.caption).foregroundStyle(.secondary) }
            }.padding(12)
        } else { Text("Aggiungi un canale per impostare la pulizia automatica.").foregroundStyle(.secondary) }
    }
    private func update(_ id: UUID, _ action: (inout Channel) -> Void) {
        model.editWorkspace { next in
            if let index = next.channels.firstIndex(where: { $0.id == id }) { action(&next.channels[index]) }
        }
    }
}
