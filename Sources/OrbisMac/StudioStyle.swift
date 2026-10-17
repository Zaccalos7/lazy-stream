import SwiftUI
import AppKit

enum StudioStyle {
    static let canvas = Color(nsColor: .windowBackgroundColor)
    static let surface = Color(nsColor: .controlBackgroundColor)
    static let stroke = Color.primary.opacity(0.07)
}

struct StudioGroupBoxStyle: GroupBoxStyle {
    func makeBody(configuration: Configuration) -> some View {
        VStack(alignment: .leading, spacing: 16) {
            configuration.label.font(.system(.headline)).foregroundStyle(.primary)
            configuration.content.frame(maxWidth: .infinity, alignment: .leading)
        }
        .padding(20)
        .background(StudioStyle.surface, in: RoundedRectangle(cornerRadius: 16))
        .overlay(RoundedRectangle(cornerRadius: 16).strokeBorder(StudioStyle.stroke))
    }
}

struct StudioShortcut: View {
    let title: LocalizedStringKey
    let subtitle: LocalizedStringKey
    let symbol: String
    let action: () -> Void
    var body: some View {
        Button(action: action) {
            HStack(spacing: 14) {
                Image(systemName: symbol).font(.system(size: 19, weight: .medium))
                    .foregroundStyle(Color.accentColor).frame(width: 40, height: 40)
                    .background(Color.accentColor.opacity(0.08), in: RoundedRectangle(cornerRadius: 10))
                VStack(alignment: .leading, spacing: 4) {
                    Text(title).font(.headline).foregroundStyle(.primary)
                    Text(subtitle).font(.caption).foregroundStyle(.secondary)
                }
                Spacer(minLength: 8)
                Image(systemName: "chevron.right").font(.caption.weight(.semibold)).foregroundStyle(.tertiary)
            }.padding(18).frame(maxWidth: .infinity, alignment: .leading)
                .background(StudioStyle.surface, in: RoundedRectangle(cornerRadius: 14))
                .overlay(RoundedRectangle(cornerRadius: 14).strokeBorder(StudioStyle.stroke))
                .contentShape(RoundedRectangle(cornerRadius: 14))
        }.buttonStyle(.plain)
    }
}

struct ActivationPicker: View {
    @Binding var selection: Int
    var body: some View {
        Picker("Stato", selection: $selection) {
            Text("Tutti").tag(-1); Text("Attivi").tag(1); Text("Disattivati").tag(0)
        }.frame(maxWidth: 260)
    }
}

struct PageControls: View {
    let total: Int
    @Binding var page: Int
    var pageSize = 10
    private var pages: Int { max(1, (total + pageSize - 1) / pageSize) }
    private var current: Int { min(page, pages - 1) }
    var body: some View {
        HStack {
            Text("\(total) risultati").font(.caption).foregroundStyle(.secondary)
            Spacer()
            Button { page = max(0, current - 1) } label: { Image(systemName: "chevron.left") }.disabled(current == 0).help("Pagina precedente")
            Text("\(current + 1) / \(pages)").monospacedDigit()
            Button { page = current + 1 } label: { Image(systemName: "chevron.right") }.disabled(current + 1 >= pages).help("Pagina successiva")
        }.padding(16)
    }
}
