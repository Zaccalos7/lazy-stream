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
