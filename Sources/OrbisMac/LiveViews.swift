import SwiftUI
import AppKit
import WebKit
import OrbisCore

@MainActor struct LiveView: View {
    @State private var channelFilter: UUID?
    @State private var platformFilter: Platform?
    @State private var liveOnly = false
    private var visible: [LiveSession] {
        model.sessions.filter { session in
            (channelFilter == nil || session.channel.id == channelFilter) && (platformFilter == nil || session.channel.platform == platformFilter) &&
            (!liveOnly || session.isRunning) && (model.search.isEmpty || session.channel.name.localizedCaseInsensitiveContains(model.search) || session.sourceName.localizedCaseInsensitiveContains(model.search))
        }
    }
    private func selectVisible() { model.selectedSession = visible.first?.id }
    @EnvironmentObject var model: AppModel
    var body: some View {
        VStack(spacing: 0) {
            HStack(spacing: 18) {
                Picker("Canale", selection: $channelFilter) {
                    Text("Tutti i canali").tag(nil as UUID?)
                    ForEach(model.workspace.channels) { Text($0.name).tag(Optional($0.id)) }
                }
                Picker("Piattaforma", selection: $platformFilter) {
                    Text("Tutte le piattaforme").tag(nil as Platform?)
                    ForEach(Platform.allCases, id: \.self) { Text($0.title).tag(Optional($0)) }
                }
                Toggle("Solo dirette attive", isOn: $liveOnly)
            }.controlSize(.small).padding(.horizontal, 24).padding(.vertical, 14).background(.bar)
            Divider()
            if !visible.isEmpty {
                Picker("Diretta", selection: $model.selectedSession) {
                    ForEach(visible) { session in Text("\(session.channel.name) · \(session.status.rawValue)").tag(Optional(session.id)) }
                }.labelsHidden().pickerStyle(.menu).padding(.vertical, 12).frame(maxWidth: 600)
            }
            if let session = visible.first(where: { $0.id == model.selectedSession }) ?? visible.first { LiveControlView(session: session).environmentObject(model).id(session.id) }
            else { ContentUnavailableView("Nessuna diretta", systemImage: "video", description: Text("Avvia una diretta per aprire anteprima, mixer e controlli.")) }
        }.onChange(of: channelFilter) { selectVisible() }.onChange(of: platformFilter) { selectVisible() }
        .onChange(of: liveOnly) { selectVisible() }.onChange(of: model.search) { selectVisible() }
    }
}
@MainActor struct LiveControlView: View {
    @EnvironmentObject var model: AppModel
    @ObservedObject var session: LiveSession
    @State private var draft: EncoderProfile
    @State private var showPlatform = false
    @State private var sceneID: UUID?
    @AppStorage("liveInspectorVisible") private var inspectorVisible = true
    init(session: LiveSession) { self.session = session; _draft = State(initialValue: session.profile) }
    var body: some View {
        HSplitView {
            ScrollView {
                VStack(alignment: .leading, spacing: 18) {
                    if session.channel.platform == .youtube && !session.airWarningDismissed {
                        HStack {
                            Label(session.airState == .onAir ? "YouTube: diretta pubblica rilevata" : session.airState == .offAir || session.airState == .ended ? "YouTube: diretta pubblica non in onda" : "YouTube: pubblicazione non verificata", systemImage: session.airState == .onAir ? "checkmark.circle" : "exclamationmark.triangle")
                            Spacer(); Button("Chiudi") { session.airWarningDismissed = true }
                        }.font(.caption).padding(10).background(.quaternary, in: RoundedRectangle(cornerRadius: 8))
                    }
                    ZStack {
                        RoundedRectangle(cornerRadius: 16).fill(.black)
                        if showPlatform, let url = publicPage(session.channel) { PlatformPlayer(url: url).clipShape(RoundedRectangle(cornerRadius: 16)) }
                        else if let preview = session.preview { Image(nsImage: preview).resizable().scaledToFit().clipShape(RoundedRectangle(cornerRadius: 16)) }
                        else { VStack { ProgressView(); Text("Attesa del segnale").foregroundStyle(.white) } }
                    }.aspectRatio(CGFloat(max(2, session.outputWidth)) / CGFloat(max(2, session.outputHeight)), contentMode: .fit)
                    HStack {
                        VStack(alignment: .leading) { Text(session.sourceName).font(.headline); Text("\(session.channel.name) · \(session.status.rawValue)").foregroundStyle(.secondary) }
                        Spacer(); Text(clock(session.currentPosition)).monospacedDigit()
                        if session.lastError != nil, let history = model.workspace.history.first(where: { $0.id == session.historyID }) {
                            Button("Segnala errore") { model.report = history }
                        }
                        Button("Termina", role: .destructive) { session.stop() }.disabled(!session.isRunning)
                    }
                    HStack {
                        Label("\(session.progress.fps, specifier: "%.1f") fps", systemImage: "film")
                        Text("\(session.progress.bitrate, specifier: "%.0f") kbit/s")
                        Text("\(session.progress.speed, specifier: "%.2f")×"); Spacer()
                        if let url = publicPage(session.channel) {
                            Toggle("Pagina piattaforma", isOn: $showPlatform).toggleStyle(.switch)
                            Link("Apri nel browser", destination: url)
                        }
                    }.font(.caption)
                    Text("L’anteprima locale mostra la composizione inviata a FFmpeg. L’avanzamento dell’invio non conferma che la piattaforma sia in onda.").font(.caption).foregroundStyle(.secondary)
                    HStack {
                        Button("Inserisci spot…") {
                            if let file = FilePicker.files().first { model.perform { try session.enqueueSpot(file.path) } }
                        }.disabled(!session.isRunning)
                        if !session.queuedSpots.isEmpty {
                            Text("\(session.queuedSpots.count) spot in coda").foregroundStyle(.secondary)
                            Button("Svuota coda") { session.queuedSpots.removeAll() }
                        }
                        Spacer()
                        Button("Esporta registro…") {
                            if let url = FilePicker.save(name: "OrbisStream-Registro.txt") {
                                model.perform { try session.logs.joined(separator: "\n").write(to: url, atomically: true, encoding: .utf8) }
                            }
                        }
                    }
                    GroupBox("Pulsanti scena") {
                        VStack(alignment: .leading) {
                            LazyVGrid(columns: [GridItem(.adaptive(minimum: 130))]) {
                                ForEach(model.workspace.deck) { button in
                                    Button { session.play(button) } label: {
                                        VStack(spacing: 8) { Image(systemName: button.inScene ? "square.3.layers.3d" : "play.rectangle"); Text(button.label); Text(button.hotkey).font(.caption).foregroundStyle(.secondary) }.frame(maxWidth: .infinity).padding(10)
                                    }.disabled(!session.isRunning)
                                }
                            }
                            HStack {
                                Button("Intermezzo…") { if let url = FilePicker.files().first { var button = DeckButton(); button.label = url.lastPathComponent; button.mediaPath = url.path; session.play(button) } }
                                if let button = session.takeover { Text("In scena: \(button.label)"); Button("Riprendi programma") { session.resume() }.buttonStyle(.borderedProminent) }
                            }.disabled(!session.isRunning)
                        }.padding(8)
                    }
                    if !session.sources.isEmpty {
                        GroupBox("Mixer audio") {
                            VStack {
                                ForEach(session.sources) { source in
                                    MixerRow(source: source) { session.setVolume(id: source.id, value: $0) }
                                }
                            }.padding(8)
                        }
                    }
                    GroupBox("Scene salvate") {
                        HStack {
                            Picker("Scena", selection: $sceneID) {
                                Text("Seleziona…").tag(nil as UUID?)
                                ForEach(model.workspace.scenes.filter { !$0.isLayout }) { Text($0.name).tag(Optional($0.id)) }
                            }
                            Button("Metti in onda") {
                                if let scene = model.workspace.scenes.first(where: { $0.id == sceneID }) {
                                    Task { do { try await AppleCapture.authorize(scene.items); try session.switchScene(scene) } catch { model.error = error.localizedDescription } }
                                }
                            }.disabled(sceneID == nil || !session.isRunning)
                        }.padding(8)
                    }
                    if !session.playlist.isEmpty {
                        GroupBox("Playlist in trasmissione") {
                            VStack {
                                ForEach(Array(session.playlist.enumerated()), id: \.element.id) { index, item in
                                    HStack {
                                        Image(systemName: index == session.index ? "play.fill" : "film")
                                        Text(item.name); Spacer()
                                        Button("Riproduci") { session.skip(to: index) }.disabled(!session.isRunning)
                                    }
                                }
                                Button("Video successivo") { session.skip() }.disabled(!session.isRunning)
                            }.padding(8)
                        }
                    }
                    DisclosureGroup("Registro FFmpeg") {
                        Text(session.logs.joined(separator: "\n")).font(.system(.caption, design: .monospaced)).textSelection(.enabled).frame(maxWidth: .infinity, alignment: .leading)
                    }
                }.frame(maxWidth: 1120).padding(28).frame(maxWidth: .infinity)
            }.frame(minWidth: 620)
            if inspectorVisible {
            VStack(spacing: 0) {
                HStack {
                    Text("Video e audio").font(.headline)
                    Spacer()
                    Button { inspectorVisible = false } label: { Image(systemName: "sidebar.right") }
                        .buttonStyle(.borderless).accessibilityLabel("Nascondi controlli video e audio")
                        .help("Nascondi controlli video e audio")
                }.padding(20)
                Divider()
                Form { ProfileFields(profile: $draft, live: true) }.formStyle(.grouped)
                Button("Applica alla diretta") { model.perform { try session.apply(draft) } }.buttonStyle(.borderedProminent).disabled(!session.isRunning).padding()
                Text("L’applicazione riavvia il produttore FFmpeg mantenendo aperta la connessione RTMP. Può esserci una breve pausa del segnale.").font(.caption).foregroundStyle(.secondary).padding(.horizontal)
            }.frame(width: 330).background(StudioStyle.surface)
            }
        }
        .toolbar {
            ToolbarItem(placement: .automatic) {
                Button { inspectorVisible.toggle() } label: { Label("Controlli video e audio", systemImage: "sidebar.right") }
                    .help(inspectorVisible ? "Nascondi controlli video e audio" : "Mostra controlli video e audio")
            }
        }
    }
    private func publicPage(_ channel: Channel) -> URL? {
        if let video = session.onAirVideo, channel.platform == .youtube { return URL(string: "https://www.youtube.com/watch?v=\(video)") }
        guard !channel.streamName.isEmpty else { return nil }
        let escaped = channel.streamName.addingPercentEncoding(withAllowedCharacters: .urlPathAllowed) ?? ""
        switch channel.platform {
        case .twitch: return URL(string: "https://www.twitch.tv/\(escaped)")
        case .youtube: return URL(string: "https://www.youtube.com/\(escaped.hasPrefix("UC") ? "channel/\(escaped)/live" : escaped.count == 11 && !escaped.hasPrefix("@") ? "watch?v=\(escaped)" : "\(escaped.hasPrefix("@") ? escaped : "@" + escaped)/live")")
        case .kick: return URL(string: "https://kick.com/\(escaped)")
        case .facebook: return URL(string: "https://www.facebook.com/\(escaped)/live")
        case .tiktok: return URL(string: "https://www.tiktok.com/@\(escaped)/live")
        case .custom: return nil
        }
    }
}
@MainActor struct MixerRow: View {
    let source: SceneItem
    let apply: (Double) -> Void
    @State private var value: Double
    @State private var previous = 100.0
    init(source: SceneItem, apply: @escaping (Double) -> Void) { self.source = source; self.apply = apply; _value = State(initialValue: source.volume) }
    var body: some View {
        HStack {
            Button { if value > 0 { previous = value; value = 0 } else { value = previous }; apply(value) } label: { Image(systemName: value == 0 ? "speaker.slash.fill" : "speaker.wave.2.fill") }.help("Mute / riattiva")
            Text(source.label).frame(width: 130, alignment: .leading)
            Slider(value: $value, in: 0...200, onEditingChanged: { editing in if !editing { apply(value) } })
            Text("\(Int(value))%").monospacedDigit().frame(width: 50)
        }.onChange(of: source.volume) { value = source.volume }
    }
}
@MainActor struct PlatformPlayer: NSViewRepresentable {
    let url: URL
    func makeNSView(context: Context) -> WKWebView { let view = WKWebView(); view.load(URLRequest(url: url)); return view }
    func updateNSView(_ view: WKWebView, context: Context) {
        if context.coordinator.requestedURL != url { context.coordinator.requestedURL = url; view.load(URLRequest(url: url)) }
    }
    final class Coordinator { var requestedURL: URL? }
    func makeCoordinator() -> Coordinator { let coordinator = Coordinator(); coordinator.requestedURL = url; return coordinator }
}
@MainActor struct ManualView: View {
    @AppStorage("language") private var language = ""
    var body: some View {
        ScrollViewReader { proxy in
        ScrollView {
            VStack(alignment: .leading, spacing: 24) {
                Text("OrbisStream per macOS").font(.largeTitle.bold())
                HStack {
                    Menu("Indice") {
                        ForEach(NativeLocalization.manual(language: language)) { section in
                            Button(section.title) { withAnimation { proxy.scrollTo(section.id, anchor: .top) } }
                        }
                    }
                    Button("Stampa / Salva PDF") { ManualPrint.run(language: language) }
                }
                GroupBox("1 · Configura") { Text("In Canali inserisci server RTMP/RTMPS, chiave, nome pubblico e sorgente. La chiave viene salvata nel Portachiavi. In Video e audio scegli Apple H.264 per sfruttare VideoToolbox, oppure libx264 per codifica software. Per RTMP usa FLV con H.264 e AAC.").padding().frame(maxWidth: .infinity, alignment: .leading) }
                GroupBox("2 · Prepara") { Text("Aggiungi e ordina i video in Playlist. Per una composizione usa Scene e layout: disponi video, schermo, camera e microfoni, poi aggiungi overlay trasparenti o animati. Un layout conserva slot riutilizzabili: Crea scena ne produce una copia da riempire. Il primo riquadro è sotto gli altri. Il primo video che termina conclude la composizione; le sorgenti di cattura continuano fino allo stop.").padding().frame(maxWidth: .infinity, alignment: .leading) }
                GroupBox("3 · Trasmetti") { Text("In Live scegli Avvia Live o Avvia playlist: seleziona prima il profilo video e audio, poi il canale e infine la scena o la cartella video. Il conto alla rovescia è annullabile. Gestione Live offre filtri, dettagli, cambio profilo, pulsanti scena, spot, ripresa e riavvio. In Live / Anteprima scegli la sessione per controllare segnale, mixer, scene e playlist. Impostazioni apre Video e audio, Canali e Preferenze.").padding().frame(maxWidth: .infinity, alignment: .leading) }
                GroupBox("4 · Intermezzi") { Text("Configura i Pulsanti scena con un’immagine o un video. A schermo intero un video viene riprodotto una volta; un’immagine resta finché riprendi il programma. In sovrapposizione il media si ripete, con posizione e durata configurabili. La scorciatoia usa Cmd, Option, Shift e Ctrl (esempio Option+1). Il mixer modifica il volume tra 0 e 200%; 0 equivale a mute.").padding().frame(maxWidth: .infinity, alignment: .leading) }
                GroupBox("macOS e statistiche") { Text("Autorizza camera, microfono e registrazione schermo nelle impostazioni Privacy e sicurezza. L’elenco dispositivi usa gli indici di AVFoundation rilevati da FFmpeg. Per audio di sistema seleziona un dispositivo audio virtuale come microfono. Panoramica campiona CPU e memoria tramite Mach, swap tramite sysctl, rete tramite getifaddrs e spazio tramite Foundation. Stato termico e risparmio energetico provengono da ProcessInfo. CPU dell’app: 100% corrisponde a un core; non include FFmpeg. Le temperature e il carico GPU non sono inventati.").padding().frame(maxWidth: .infinity, alignment: .leading) }
                FeatureManualView()
                GroupBox("Dati e recupero") { Text("Le configurazioni e lo storico sono in ~/Library/Application Support/OrbisStream/workspace.json. Gli overlay e i media dei pulsanti vengono copiati in quella cartella; i video della playlist sono riferimenti ai file originali. Esporta/importa la configurazione dal menu File. Le chiavi non sono esportate: sul nuovo Mac reinseriscile nei canali. Lo storico consente riavvio, esportazione e pulizia manuale o periodica. Il registro maschera l’indirizzo di pubblicazione. La pagina della piattaforma resta il controllo dell’effettiva messa in onda: il solo ingest non è una conferma.").padding().frame(maxWidth: .infinity, alignment: .leading) }
            }.padding(28)
        }
        }
    }
}
