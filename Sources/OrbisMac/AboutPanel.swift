import AppKit

@MainActor enum AboutPanel {
    static func show() {
        let credits = NSMutableAttributedString(string: "OrbisStream per macOS · Apple Silicon\nmarco.guarino\n\n")
        for (title, address) in [
            ("GitHub", "https://github.com/Zaccalos7/lazy-stream-mac"),
            ("LinkedIn", "https://www.linkedin.com/in/marco-amleto-guarino/"),
            ("Supporta il progetto", "https://buymeacoffee.com/zaccalos")
        ] {
            credits.append(NSAttributedString(string: title + "\n", attributes: [.link: URL(string: address)!]))
        }
        NSApp.orderFrontStandardAboutPanel(options: [.credits: credits])
    }
}
