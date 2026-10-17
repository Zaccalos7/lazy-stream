import SwiftUI
import OrbisCore

struct StartRequest: Identifiable {
    enum Mode { case scene, playlist }
    let id = UUID()
    var mode: Mode
    var channelID: UUID?
}

@MainActor struct LiveHubView: View {
    @EnvironmentObject private var model: AppModel
    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 24) {
                HStack {
                    VStack(alignment: .leading, spacing: 8) {
                        Text("Live").font(.largeTitle.bold())
                        Text("Prepara, trasmetti e gestisci le tue dirette.").foregroundStyle(.secondary)
                    }
                    Spacer()
                    Button("Avvia playlist") { model.startRequest = StartRequest(mode: .playlist) }.controlSize(.large)
                    Button("Avvia Live") { model.startRequest = StartRequest(mode: .scene) }.buttonStyle(.borderedProminent).controlSize(.large)
                }
                LazyVGrid(columns: [GridItem(.adaptive(minimum: 280))], spacing: 16) {
                    shortcut(.management, "Controlla tutte le trasmissioni")
                    shortcut(.preview, "Segnale, mixer e regia")
                    shortcut(.scenes, "Componi il tuo spazio")
                    shortcut(.history, "Consulta le dirette precedenti")
                }
                Text("Prepara il programma").font(.headline)
                LazyVGrid(columns: [GridItem(.adaptive(minimum: 240))], spacing: 16) {
                    shortcut(.playlist, "Ordina e abilita i video")
                    shortcut(.deck, "Configura intermezzi e scorciatoie")
                    shortcut(.overlays, "Gestisci i media in sovrapposizione")
                    shortcut(.recovery, "Riprendi le sessioni interrotte")
                }
            }.padding(32).frame(maxWidth: 1180).frame(maxWidth: .infinity)
        }
    }
    private func shortcut(_ page: Page, _ subtitle: LocalizedStringKey) -> some View {
        StudioShortcut(title: LocalizedStringKey(page.rawValue), subtitle: subtitle, symbol: page.symbol) { model.open(page) }
    }
}

@MainActor struct SettingsHubView: View {
    @EnvironmentObject private var model: AppModel
    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 24) {
                Text("Impostazioni").font(.largeTitle.bold())
                Text("Seleziona la sezione da configurare.").foregroundStyle(.secondary)
                LazyVGrid(columns: [GridItem(.adaptive(minimum: 280))], spacing: 16) {
                    StudioShortcut(title: "Video e audio", subtitle: "Profili, qualità e codifica", symbol: Page.profiles.symbol) { model.open(.profiles) }
                    StudioShortcut(title: "Canali", subtitle: "Destinazioni e chiavi di streaming", symbol: Page.channels.symbol) { model.open(.channels) }
                    StudioShortcut(title: "Preferenze", subtitle: "Aspetto, lingua, aggiornamenti e dati", symbol: Page.preferences.symbol) { model.open(.preferences) }
                }
            }.padding(32).frame(maxWidth: 1180).frame(maxWidth: .infinity)
        }
    }
}

@MainActor struct LiveManagementView: View {
    @EnvironmentObject private var model: AppModel
    @State private var channelID: UUID?
    @State private var platform: Platform?
    @State private var status = ""
    @State private var page = 0
    @State private var details: LiveHistory?
    private var entries: [LiveHistory] {
        model.workspace.history.filter {
            (channelID == nil || $0.channel.id == channelID) && (platform == nil || $0.channel.platform == platform) &&
            (status.isEmpty || model.state($0) == status) &&
            (model.search.isEmpty || $0.channel.name.localizedCaseInsensitiveContains(model.search) ||
             $0.scene?.name.localizedCaseInsensitiveContains(model.search) == true || $0.playlist.contains { $0.name.localizedCaseInsensitiveContains(model.search) })
        }.sorted { $0.start > $1.start }
    }
    private var pageCount: Int { max(1, (entries.count + 9) / 10) }
    private var currentPage: Int { min(page, pageCount - 1) }
    var body: some View {
        VStack(spacing: 0) {
            HStack {
                Picker("Stato", selection: $status) {
                    Text("Tutti gli stati").tag(""); Text("In corso").tag("live")
                    Text("Terminata").tag("ended"); Text("Errore").tag("error"); Text("Interrotta").tag("interrupted")
                }
                Picker("Canale", selection: $channelID) {
                    Text("Tutti i canali").tag(nil as UUID?)
                    ForEach(model.knownChannels) { Text($0.name).tag(Optional($0.id)) }
                }
                Picker("Piattaforma", selection: $platform) {
                    Text("Tutte le piattaforme").tag(nil as Platform?)
                    ForEach(Platform.allCases, id: \.self) { Text($0.title).tag(Optional($0)) }
                }
            }.padding(20).background(.bar)
            List {
                ForEach(Array(entries.dropFirst(currentPage * 10).prefix(10))) { history in
                    LiveManagementRow(history: history, details: { details = history })
                }
            }
            .overlay {
                if entries.isEmpty { ContentUnavailableView("Nessuna diretta", systemImage: "video", description: Text("Avvia una Live o una playlist per gestirla qui.")) }
            }
            HStack {
                Text("\(entries.count) dirette").foregroundStyle(.secondary)
                Spacer()
                Button { page = max(0, currentPage - 1) } label: { Image(systemName: "chevron.left") }.disabled(currentPage == 0).help("Pagina precedente")
                Text("\(currentPage + 1) / \(pageCount)").monospacedDigit()
                Button { page = currentPage + 1 } label: { Image(systemName: "chevron.right") }.disabled(currentPage + 1 >= pageCount).help("Pagina successiva")
            }.padding(20)
        }
        .onChange(of: channelID) { page = 0 }.onChange(of: platform) { page = 0 }
        .onChange(of: status) { page = 0 }.onChange(of: model.search) { page = 0 }
        .sheet(item: $details) { LiveDetailsView(historyID: $0.id).environmentObject(model) }
    }
}

@MainActor struct LiveManagementRow: View {
    @EnvironmentObject private var model: AppModel
    let history: LiveHistory
    let details: () -> Void
    @State private var deckSession: LiveSession?
    private var session: LiveSession? { model.sessions.first { $0.historyID == history.id && $0.isRunning } }
    private var title: String {
        if let session { return session.sourceName }
        if let scene = history.scene { return scene.name }
        return history.playlist.first(where: { $0.id == history.lastVideoID })?.name ?? history.playlist.first?.name ?? history.channel.name
    }
    var body: some View {
        HStack(spacing: 16) {
            Image(systemName: session == nil ? "stop.circle" : "record.circle.fill")
                .foregroundStyle(session == nil ? Color.secondary : Color.red).font(.title2)
            VStack(alignment: .leading, spacing: 6) {
                Text(title).font(.headline)
                HStack {
                    Text(history.scene == nil ? "Playlist" : "Scena").font(.caption.weight(.medium)).padding(.horizontal, 8).padding(.vertical, 3).background(.quaternary, in: Capsule())
                    Text("\(history.channel.name) · \(history.channel.platform.title)").foregroundStyle(.secondary)
                    Text(LocalizedStringKey(model.stateTitle(history))).foregroundStyle(.secondary)
                }
                Text(history.start.formatted()).font(.caption).foregroundStyle(.secondary)
            }
            Spacer()
            Button(action: details) { Image(systemName: "info.circle") }.help("Dettagli")
            if let url = PlatformLinks.publicURL(history.channel, onAirVideo: session?.onAirVideo) {
                Link(destination: url) { Image(systemName: "arrow.up.right.square") }.help("Apri pagina piattaforma")
            }
            if let session {
                Button { model.showPreview(session) } label: { Image(systemName: "play.rectangle") }.help("Live / Anteprima")
                Button { model.selectedSession = session.id; deckSession = session } label: { Image(systemName: "square.grid.3x3") }.help("Pulsanti scena")
                Button {
                    guard let url = FilePicker.files().first else { return }
                    model.perform { try session.enqueueSpot(url.path) }
                } label: { Image(systemName: "film.badge.plus") }.help("Inserisci spot…")
                Button(role: .destructive) { session.stop() } label: { Image(systemName: "stop.fill") }.help("Termina")
            } else {
                Button { model.replay(history, resume: true) } label: { Image(systemName: "play.fill") }.help("Riprendi dalla posizione")
                Button { model.replay(history, resume: false) } label: { Image(systemName: "arrow.counterclockwise") }.help("Ricomincia dall’inizio")
            }
            Menu {
                ForEach(model.workspace.profiles.filter { $0.active }) { profile in
                    Button(profile.title) { model.assignProfile(profile, historyID: history.id) }
                }
            } label: { Image(systemName: "slider.horizontal.3") }.help("Associa profilo video e audio")
            if history.error != nil {
                Button { model.report = history } label: { Image(systemName: "exclamationmark.bubble") }.help("Segnala errore")
            }
            Button(role: .destructive) { model.removeHistory(history.id) } label: { Image(systemName: "trash") }.disabled(session != nil).help("Elimina")
        }.padding(.vertical, 10)
        .sheet(item: $deckSession) { LiveDeckPanel(session: $0).environmentObject(model) }
    }
}

@MainActor struct LiveDeckPanel: View {
    @EnvironmentObject private var model: AppModel
    @Environment(\.dismiss) private var dismiss
    @ObservedObject var session: LiveSession
    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            HStack { Text("Pulsanti scena").font(.title2.bold()); Spacer(); Button("Chiudi") { dismiss() } }
            Text(session.channel.name).foregroundStyle(.secondary)
            ScrollView {
                LazyVGrid(columns: [GridItem(.adaptive(minimum: 160))], spacing: 14) {
                    ForEach(model.workspace.deck) { button in
                        Button { model.playDeck(button, session: session) } label: {
                            VStack(spacing: 10) {
                                Image(systemName: button.inScene ? "square.3.layers.3d" : "play.rectangle").font(.title2)
                                Text(button.label); Text(button.hotkey).font(.caption).foregroundStyle(.secondary)
                            }.frame(maxWidth: .infinity).padding(20)
                        }.disabled(!session.isRunning)
                    }
                }
            }
            if let takeover = session.takeover {
                HStack { Text(takeover.label); Spacer(); Button("Riprendi programma") { session.resume() }.buttonStyle(.borderedProminent).disabled(!session.isRunning) }
            }
            Button("Configura pulsanti scena") { model.open(.deck); dismiss() }
        }.padding(28).frame(width: 680, height: 480)
        .onAppear { model.deckPanelOpen = true }.onDisappear { model.deckPanelOpen = false }
    }
}

@MainActor struct LiveDetailsView: View {
    @EnvironmentObject private var model: AppModel
    @Environment(\.dismiss) private var dismiss
    let historyID: UUID
    @State private var preview: MediaSelection?
    @State private var archivedLog = ""
    private var history: LiveHistory? { model.workspace.history.first { $0.id == historyID } }
    private var session: LiveSession? { model.sessions.first { $0.historyID == historyID && $0.isRunning } }
    private var currentVideoID: UUID? {
        guard let session, session.playlist.indices.contains(session.index) else { return history?.lastVideoID }
        return session.playlist[session.index].id
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            HStack { Text("Dettagli diretta").font(.title2.bold()); Spacer(); Button("Chiudi") { dismiss() } }
            if let history {
                HStack {
                    VStack(alignment: .leading) {
                        Text("\(history.channel.name) · \(history.profile.title)").font(.headline)
                        Text(history.start.formatted()).foregroundStyle(.secondary)
                    }
                    Spacer()
                    if let session {
                        Button("Live / Anteprima") { model.showPreview(session); dismiss() }
                        Button("Termina", role: .destructive) { session.stop() }
                    }
                }
                ScrollView {
                    VStack(alignment: .leading, spacing: 14) {
                        if let scene = history.scene {
                            ForEach(scene.items) { item in
                                HStack {
                                    Label("\(item.label) · \(item.kind.title) · \(item.width)×\(item.height)", systemImage: item.kind == .microphone ? "mic" : "video")
                                    Spacer()
                                    if item.kind == .file || item.kind == .overlay {
                                        Button("Anteprima") { preview = MediaSelection(path: item.target) }
                                    }
                                }
                            }
                        } else {
                            ForEach(Array(history.playlist.enumerated()), id: \.element.id) { index, item in
                                VStack(alignment: .leading, spacing: 8) {
                                    HStack {
                                        Image(systemName: currentVideoID == item.id ? "play.fill" : "film")
                                        Text("\(index + 1). \(item.name)").font(.headline)
                                        Spacer(); Text(clock(item.resumeSeconds)).monospacedDigit()
                                        if item.duration > 0 { Text("/ \(clock(item.duration))").foregroundStyle(.secondary) }
                                    }
                                    Text(item.path).font(.caption).foregroundStyle(.secondary).textSelection(.enabled)
                                    HStack {
                                        Button("Anteprima") { preview = MediaSelection(path: item.path) }
                                        Button("Avvia da questo video") { model.replayFrom(history, videoID: item.id, resume: false) }.disabled(session != nil)
                                        Button("Dalla posizione") { model.replayFrom(history, videoID: item.id, resume: true) }.disabled(session != nil)
                                        if let session { Button("Metti in onda") { if let next = session.playlist.firstIndex(where: { $0.id == item.id }) { session.skip(to: next) } } }
                                        Spacer()
                                        Menu("Profilo video") {
                                            Button("Della diretta") { model.linkVideoProfile(historyID: historyID, videoID: item.id, profileID: nil) }
                                            ForEach(model.workspace.profiles.filter { $0.active }) { profile in
                                                Button(profile.title) { model.linkVideoProfile(historyID: historyID, videoID: item.id, profileID: profile.id) }
                                            }
                                        }
                                        Button(role: .destructive) { model.deleteHistoryVideo(historyID: history.id, videoID: item.id) } label: { Image(systemName: "trash") }.disabled(session != nil).help("Elimina video")
                                    }
                                }.padding(14).background(.quaternary, in: RoundedRectangle(cornerRadius: 10))
                            }
                        }
                        if let error = history.error { Text(error).foregroundStyle(.red).textSelection(.enabled) }
                        if !logText.isEmpty {
                            DisclosureGroup("Registro FFmpeg") {
                                Text(logText).font(.system(.caption, design: .monospaced)).textSelection(.enabled)
                                Button("Esporta registro…") {
                                    if let url = FilePicker.save(name: "OrbisStream-Registro.txt") { model.perform { try logText.write(to: url, atomically: true, encoding: .utf8) } }
                                }
                            }
                        }
                    }.frame(maxWidth: .infinity, alignment: .leading)
                }
            }
        }.padding(28).frame(width: 920, height: 650)
        .sheet(item: $preview) { MediaPreviewPanel(path: $0.path).environmentObject(model) }
        .task(id: historyID) { archivedLog = (try? SessionLog.read(directory: model.store.directory.appendingPathComponent("Logs"), sessionID: historyID)) ?? "" }
    }
    private var logText: String { model.sessions.first(where: { $0.historyID == historyID })?.logs.joined(separator: "\n") ?? archivedLog }
}

extension AppModel {
    func state(_ history: LiveHistory) -> String {
        if sessions.contains(where: { $0.historyID == history.id && $0.isRunning }) { return "live" }
        if workspace.interruptedRuns.contains(where: { $0.historyID == history.id }) { return "interrupted" }
        return history.error == nil ? "ended" : "error"
    }
    func stateTitle(_ history: LiveHistory) -> String {
        switch state(history) {
        case "live": return "In corso"
        case "interrupted": return "Interrotta"
        case "error": return "Errore"
        default: return "Terminata"
        }
    }
}
