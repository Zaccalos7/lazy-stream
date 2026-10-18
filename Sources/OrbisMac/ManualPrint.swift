import AppKit

@MainActor enum ManualPrint {
    static func run(language: String) {
        let sections = NativeLocalization.manual(language: language)
        let native = """
        OrbisStream · macOS Apple Silicon

        Percorsi Mac: Live → Avvia Live o Avvia playlist → Video e audio → Canale → Scena o cartella. Gestione Live contiene dettagli, ripresa, profili, pulsanti scena e spot. Live / Anteprima contiene segnale, composizione e mixer. Impostazioni apre Video e audio, Canali e Preferenze.

        Catture: AVFoundation. Autorizza camera, microfono e registrazione schermo in Privacy e sicurezza di macOS. Per l’audio di sistema aggiungi un dispositivo audio virtuale come microfono. La codifica Apple usa VideoToolbox. I comandi del Mac usano Cmd, Option, Shift e Ctrl.

        Le chiavi sono nel Portachiavi. Configurazione, storico, registri e media importati sono in ~/Library/Application Support/OrbisStream. I video della playlist mantengono i riferimenti ai file originali.

        """
        let text = native + sections.map { section in
            ([section.title, section.body] + (section.steps ?? []).enumerated().map { "\($0.offset + 1). \($0.element)" } + [section.callout ?? ""]).joined(separator: "\n\n")
        }.joined(separator: "\n\n————————————————\n\n")
        let view = NSTextView(frame: NSRect(x: 0, y: 0, width: 520, height: 720))
        view.string = text; view.font = .systemFont(ofSize: 11); view.textColor = .black; view.backgroundColor = .white
        view.textContainerInset = NSSize(width: 12, height: 12)
        view.isHorizontallyResizable = false; view.isVerticallyResizable = true
        view.textContainer?.widthTracksTextView = true
        view.textContainer?.containerSize = NSSize(width: 520, height: CGFloat.greatestFiniteMagnitude)
        if let container = view.textContainer, let manager = view.layoutManager {
            manager.ensureLayout(for: container)
            view.setFrameSize(NSSize(width: 520, height: manager.usedRect(for: container).height + 24))
        }
        let info = NSPrintInfo(dictionary: NSPrintInfo.shared.dictionary())
        info.horizontalPagination = .fit; info.verticalPagination = .automatic
        let operation = NSPrintOperation(view: view, printInfo: info)
        operation.showsPrintPanel = true; operation.showsProgressPanel = true
        operation.run()
    }
}
