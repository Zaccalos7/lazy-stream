import Foundation
import SwiftUI

enum NativeLocalization {
    static let languages = ["ar", "da", "de", "el", "en", "es", "fil", "fr", "hi", "hod", "it", "ja", "ko", "la", "nl", "pt", "ro", "ru", "tlh", "tr", "zh"]
    struct Manual: Decodable { let sections: [Section] }
    struct Section: Decodable, Identifiable {
        let id: String
        let title: String
        let body: String
        let steps: [String]?
        let callout: String?
    }
    private static var resources: Bundle {
        if let root = Bundle.main.resourceURL, let bundle = Bundle(url: root.appendingPathComponent("OrbisStream_OrbisMac.bundle")) { return bundle }
        return Bundle.module
    }
    static func manual(language: String) -> [Section] {
        let code = language.isEmpty ? (Locale.current.language.languageCode?.identifier ?? "it") : language
        let url = resources.url(forResource: "manual.\(code)", withExtension: "json") ?? resources.url(forResource: "manual.it", withExtension: "json")
        guard let url, let data = try? Data(contentsOf: url), let manual = try? JSONDecoder().decode(Manual.self, from: data) else { return [] }
        return manual.sections
    }
}
@MainActor struct FeatureManualView: View {
    @AppStorage("language") private var language = ""
    var body: some View {
        ForEach(NativeLocalization.manual(language: language)) { section in
            GroupBox(section.title) {
                VStack(alignment: .leading, spacing: 8) {
                    Text(section.body)
                    ForEach(Array((section.steps ?? []).enumerated()), id: \.offset) { index, step in Text("\(index + 1). \(step)") }
                    if let callout = section.callout { Text(callout).font(.caption).foregroundStyle(.secondary) }
                }.padding().frame(maxWidth: .infinity, alignment: .leading)
            }
        }
    }
}
