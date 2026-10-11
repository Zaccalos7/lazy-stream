#if !os(macOS) || !arch(arm64)
#error("OrbisStream richiede macOS su Apple Silicon.")
#endif
import SwiftUI
import AppKit
import OrbisCore

@main @MainActor struct OrbisApp: App {
    @AppStorage("language") private var language = ""
    @AppStorage("appearance") private var appearance = "system"
    private var colorScheme: ColorScheme? { appearance == "light" ? .light : appearance == "dark" ? .dark : nil }
    @StateObject private var model = AppModel()
    @NSApplicationDelegateAdaptor(AppDelegate.self) private var delegate
    var body: some SwiftUI.Scene {
        WindowGroup {
            RootView().environmentObject(model)
                .preferredColorScheme(colorScheme)
                .environment(\.locale, language.isEmpty ? Locale.current : Locale(identifier: language))
                .frame(minWidth: 1100, minHeight: 720)
                .onAppear { delegate.model = model; NSApp.setActivationPolicy(.regular); NSApp.activate(ignoringOtherApps: true) }
        }
        .defaultSize(width: 1320, height: 860)
        .windowStyle(.titleBar)
        .windowToolbarStyle(.unified)
        .commands {
            CommandGroup(replacing: .appInfo) {
                Button("Informazioni su OrbisStream") { NSApp.orderFrontStandardAboutPanel(nil) }
            }
            CommandGroup(replacing: .newItem) {
                Button("Importa configurazione…") { model.importWorkspace() }
                Button("Esporta configurazione…") { model.exportWorkspace() }
            }
            CommandMenu("Diretta") {
                Button("Termina tutte le dirette") { model.stopAll() }
                Button("Riprendi programma") { model.watched?.resume() }.keyboardShortcut("r", modifiers: [.command, .shift])
                Button("Video successivo") { model.watched?.skip() }.keyboardShortcut(.rightArrow, modifiers: .command)
            }
        }
        Settings { PreferencesView().environmentObject(model).preferredColorScheme(colorScheme).frame(width: 680, height: 560) }
    }
}
@MainActor final class AppDelegate: NSObject, NSApplicationDelegate {
    weak var model: AppModel?
    func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply {
        guard model?.sessions.contains(where: { $0.isRunning }) == true else { return .terminateNow }
        let alert = NSAlert(); alert.messageText = "Terminare le dirette e chiudere OrbisStream?"
        alert.addButton(withTitle: "Termina e chiudi"); alert.addButton(withTitle: "Annulla")
        guard alert.runModal() == .alertFirstButtonReturn else { return .terminateCancel }
        model?.stopAll()
        Task {
            while model?.sessions.contains(where: { $0.isRunning }) == true { try? await Task.sleep(for: .milliseconds(100)) }
            sender.reply(toApplicationShouldTerminate: true)
        }
        return .terminateLater
    }
}
enum Page: String, CaseIterable, Identifiable {
    case dashboard = "Panoramica", live = "Regia", playlist = "Playlist", scenes = "Scene e layout", deck = "Pulsanti scena", overlays = "Overlay", channels = "Canali", profiles = "Video e audio", history = "Storico", tasks = "Attività", recovery = "Recupero", manual = "Guida"
    var id: String { rawValue }
    var symbol: String {
        switch self {
        case .dashboard: return "square.grid.2x2"; case .live: return "dot.radiowaves.left.and.right"; case .playlist: return "list.bullet.rectangle"; case .scenes: return "rectangle.3.group"; case .deck: return "square.grid.3x3"; case .overlays: return "square.3.layers.3d"; case .channels: return "antenna.radiowaves.left.and.right"; case .profiles: return "slider.horizontal.3"; case .history: return "clock.arrow.circlepath"; case .tasks: return "waveform.path.ecg"; case .recovery: return "lock.open"; case .manual: return "book"
        }
    }
}
@MainActor struct RootView: View {
    @EnvironmentObject private var model: AppModel
    @State private var page: Page? = .dashboard
    @State private var wizard = false
    var body: some View {
        NavigationSplitView {
            List(selection: $page) {
                Section("Studio") {
                    sidebarRow(.dashboard); sidebarRow(.live); sidebarRow(.playlist)
                }
                Section("Libreria") {
                    sidebarRow(.scenes); sidebarRow(.deck); sidebarRow(.overlays)
                }
                Section("Configurazione") {
                    sidebarRow(.channels); sidebarRow(.profiles)
                }
                Section("Monitoraggio") {
                    sidebarRow(.history); sidebarRow(.tasks); sidebarRow(.recovery)
                }
                Section { sidebarRow(.manual) }
            }
            .listStyle(.sidebar)
            .navigationTitle("OrbisStream")
            .navigationSplitViewColumnWidth(min: 210, ideal: 228, max: 270)
            .safeAreaInset(edge: .bottom) {
                HStack(spacing: 10) {
                    Circle().fill(model.sessions.contains(where: { $0.isRunning }) ? Color.red : Color.secondary.opacity(0.4)).frame(width: 7, height: 7)
                    Text("\(model.sessions.filter { $0.isRunning }.count) dirette attive").font(.caption)
                    Spacer()
                    Image(systemName: "apple.logo").foregroundStyle(.tertiary).accessibilityHidden(true)
                }.padding(18).background(.bar)
            }
        } detail: {
            Group {
                switch page ?? .dashboard {
                case .dashboard: DashboardView(open: { page = $0 }, start: { wizard = true })
                case .live: LiveView()
                case .playlist: PlaylistView()
                case .scenes: ScenesView()
                case .deck: DeckView()
                case .overlays: OverlaysView()
                case .channels: ChannelsView()
                case .profiles: ProfilesView()
                case .history: HistoryView()
                case .tasks: TasksView()
                case .recovery: RecoveryView()
                case .manual: ManualView()
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(StudioStyle.canvas)
            .navigationTitle((page ?? .dashboard).rawValue)
            .toolbar {
                ToolbarItem(placement: .primaryAction) { Button { wizard = true } label: { Label("Avvia diretta", systemImage: "dot.radiowaves.left.and.right") }.buttonStyle(.borderedProminent).controlSize(.regular).help("Avvia una scena o una playlist") }
            }
            .searchable(text: $model.search, prompt: "Cerca")
        }
        .tint(.accentColor)
        .groupBoxStyle(StudioGroupBoxStyle())
        .sheet(isPresented: $wizard) { StartWizard().environmentObject(model) }
        .alert("OrbisStream", isPresented: Binding(get: { model.error != nil }, set: { if !$0 { model.error = nil } })) {
            Button("OK") { model.error = nil }
        } message: { Text(model.error ?? "") }
    }
    private func sidebarRow(_ entry: Page) -> some View {
        Label { Text(LocalizedStringKey(entry.rawValue)) } icon: {
            Image(systemName: entry.symbol).font(.system(size: 14, weight: .medium))
        }.padding(.vertical, 3).tag(entry)
    }

}
func bytes(_ value: UInt64) -> String { ByteCountFormatter.string(fromByteCount: Int64(clamping: value), countStyle: .memory) }
func rate(_ value: Double?) -> String { value.map { ByteCountFormatter.string(fromByteCount: Int64($0), countStyle: .decimal) + "/s" } ?? "—" }
func percent(_ value: Double?) -> String { value.map { String(format: "%.1f%%", $0) } ?? "—" }
func clock(_ seconds: Double) -> String { let value = Int(max(0, seconds)); return String(format: "%02d:%02d:%02d", value / 3600, value / 60 % 60, value % 60) }
@MainActor struct MetricCard: View {
    let title: String
    let value: String
    let symbol: String
    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            HStack {
                Text(LocalizedStringKey(title)).font(.subheadline).foregroundStyle(.secondary)
                Spacer()
                Image(systemName: symbol).foregroundStyle(.tertiary)
            }
            Text(value).font(.system(size: 25, weight: .medium, design: .rounded))
                .monospacedDigit().lineLimit(1).minimumScaleFactor(0.6)
        }.padding(20).frame(maxWidth: .infinity, alignment: .leading)
            .background(StudioStyle.surface, in: RoundedRectangle(cornerRadius: 16))
            .overlay(RoundedRectangle(cornerRadius: 16).strokeBorder(StudioStyle.stroke))
    }
}
@MainActor struct DashboardView: View {
    let open: (Page) -> Void
    let start: () -> Void
    @State private var showResources = false
    @AppStorage("homeBackground") private var background = ""
    @EnvironmentObject private var model: AppModel
    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 24) {
                HStack(alignment: .center) {
                    VStack(alignment: .leading, spacing: 8) {
                        Text("Il tuo studio.").font(.system(size: 34, weight: .semibold))
                        Text("Tutto pronto per la prossima diretta.").font(.title3).foregroundStyle(.secondary)
                    }
                    Spacer()
                    Button(action: start) { Label("Avvia diretta", systemImage: "plus") }
                        .buttonStyle(.borderedProminent).controlSize(.large)
                }.padding(.vertical, 12)
                LazyVGrid(columns: [GridItem(.adaptive(minimum: 220), spacing: 14)], spacing: 14) {
                    StudioShortcut(title: "Canali", subtitle: "Le tue destinazioni", symbol: "antenna.radiowaves.left.and.right") { open(.channels) }
                    StudioShortcut(title: "Scene", subtitle: "Componi il tuo spazio", symbol: "rectangle.3.group") { open(.scenes) }
                    StudioShortcut(title: "Playlist", subtitle: "Prepara il programma", symbol: "list.bullet.rectangle") { open(.playlist) }
                }
                Text("Il Mac, in tempo reale").font(.headline).padding(.top, 8)
                LazyVGrid(columns: [GridItem(.adaptive(minimum: 205), spacing: 14)], spacing: 14) {
                    MetricCard(title: "CPU del Mac", value: percent(model.metrics.cpuPercent), symbol: "cpu")
                    MetricCard(title: "Memoria", value: bytes(model.metrics.memoryUsed) + " / " + bytes(model.metrics.memoryTotal), symbol: "memorychip")
                    MetricCard(title: "Upload del Mac", value: rate(model.metrics.uploadBytesPerSecond), symbol: "arrow.up.circle")
                    MetricCard(title: "Stato termico", value: model.metrics.thermalState, symbol: "thermometer.medium")
                }
                GroupBox("Dirette") {
                    if model.sessions.isEmpty { ContentUnavailableView("Pronto per trasmettere", systemImage: "video", description: Text("Configura un canale e scegli una playlist o una scena.")) }
                    else { ForEach(model.sessions.filter { model.search.isEmpty || $0.channel.name.localizedCaseInsensitiveContains(model.search) }) { SessionRow(session: $0) } }
                }
                DisclosureGroup("Dettagli del Mac", isExpanded: $showResources) {
                    Grid(alignment: .leading, horizontalSpacing: 40, verticalSpacing: 14) {
                        GridRow { Text("Sistema"); Text(ProcessInfo.processInfo.operatingSystemVersionString) }
                        GridRow { Text("CPU"); Text("\(ProcessInfo.processInfo.processorCount) core") }
                        GridRow { Text("Memoria compressa"); Text(bytes(model.metrics.compressed)) }
                        GridRow { Text("Swap"); Text(bytes(model.metrics.swapUsed) + " / " + bytes(model.metrics.swapTotal)) }
                        GridRow { Text("Spazio disponibile"); Text(bytes(UInt64(max(0, model.metrics.diskAvailable)))) }
                        GridRow { Text("Lettura dischi del Mac"); Text(rate(model.metrics.diskReadBytesPerSecond)) }
                        GridRow { Text("Scrittura dischi del Mac"); Text(rate(model.metrics.diskWriteBytesPerSecond)) }
                        GridRow { Text("Download del Mac"); Text(rate(model.metrics.downloadBytesPerSecond)) }
                        GridRow { Text("Risparmio energetico"); Text(model.metrics.lowPower ? "Attivo" : "Disattivato") }
                        GridRow { Text("Temperatura CPU/GPU e carico GPU"); Text("Non disponibili tramite API pubbliche uniformi").foregroundStyle(.secondary) }
                    }.frame(maxWidth: .infinity, alignment: .leading).padding()
                }
            }.frame(maxWidth: 1180).padding(32).frame(maxWidth: .infinity)
        }.background {
            if let image = NSImage(contentsOfFile: background) { Image(nsImage: image).resizable().scaledToFill().opacity(0.12).clipped() }
        }
    }
}
@MainActor struct SessionRow: View {
    @ObservedObject var session: LiveSession
    var body: some View {
        HStack(spacing: 14) {
            Image(systemName: session.isRunning ? "record.circle.fill" : "stop.circle").foregroundStyle(session.isRunning ? .red : .secondary)
            VStack(alignment: .leading) { Text(session.channel.name).font(.headline); Text(session.sourceName).foregroundStyle(.secondary) }
            Spacer(); Text(session.status.rawValue); Text(clock(session.progress.seconds)).monospacedDigit()
            if session.isRunning { Button("Termina", role: .destructive) { session.stop() } }
        }.padding(.vertical, 12).padding(.horizontal, 4)
    }
}
@MainActor struct TasksView: View {
    @EnvironmentObject var model: AppModel
    var body: some View {
        VStack(spacing: 20) {
            HStack {
                MetricCard(title: "CPU dell’app (100% = un core)", value: percent(model.metrics.appCPU), symbol: "cpu")
                MetricCard(title: "Memoria dell’app", value: bytes(model.metrics.appMemory), symbol: "memorychip")
                MetricCard(title: "Uptime del Mac", value: clock(model.metrics.uptime), symbol: "clock")
            }
            Text("Le risorse dell’app escludono i processi FFmpeg. I contatori di rete riguardano le interfacce fisiche del Mac.").font(.caption).foregroundStyle(.secondary)
            List(model.sessions) { session in
                VStack(alignment: .leading) {
                    SessionRow(session: session)
                    ForEach(session.processDetails) { process in
                        HStack {
                            Text("\(process.role) · PID \(process.pid) · \(session.profile.codec)")
                            if let metrics = model.processMetrics[process.pid] { Text("CPU \(percent(metrics.cpuPercent)) · \(bytes(metrics.memory)) · \(metrics.threads) thread") }
                            else { Text("Statistiche non disponibili").foregroundStyle(.secondary) }
                        }.font(.system(.caption, design: .monospaced))
                    }
                    Text("\(session.progress.fps.formatted()) fps · \(session.progress.bitrate.formatted()) kbit/s · \(session.progress.bytes) byte inviati").font(.caption)
                }
            }
            HStack { Spacer(); Button("Termina tutte", role: .destructive) { model.stopAll() } }
        }.padding(24)
    }
}
@MainActor struct PreferencesView: View {
    @AppStorage("appearance") private var appearance = "system"
    @AppStorage("homeBackground") private var background = ""
    @AppStorage("language") private var language = ""
    @EnvironmentObject var model: AppModel
    var body: some View {
        Form {
            Section("FFmpeg") {
                executable("FFmpeg", path: $model.workspace.ffmpegPath)
                executable("FFprobe", path: $model.workspace.ffprobePath)
                Text("Ricerca automatica nel bundle, in /opt/homebrew/bin. Usa binari macOS della stessa distribuzione.").font(.caption)
                Toggle("Ripeti la playlist", isOn: $model.workspace.loopPlaylist)
            }
            Section("Aspetto e lingua") {
                Picker("Tema", selection: $appearance) {
                    Text("Sistema").tag("system"); Text("Chiaro").tag("light"); Text("Scuro").tag("dark")
                }
                Picker("Lingua", selection: $language) {
                    Text("Sistema").tag("")
                    ForEach(NativeLocalization.languages, id: \.self) { code in Text(Locale.current.localizedString(forLanguageCode: code) ?? code).tag(code) }
                }.onChange(of: language) {
                    UserDefaults.standard.set(language.isEmpty ? [] : [language], forKey: "AppleLanguages")
                }
                Text("Riavvia l’app per applicare la lingua a tutti i menu. I nuovi controlli Apple non tradotti usano l’italiano.").font(.caption)
                HStack {
                    Text("Sfondo della panoramica")
                    Button("Scegli immagine…") {
                        model.perform {
                            guard let url = FilePicker.files().first else { return }
                            guard NSImage(contentsOf: url) != nil else { throw OrbisError.invalid("Seleziona un’immagine leggibile dal Mac.") }
                            background = try model.store.importMedia(url, collection: "Appearance")
                        }
                    }
                    Button("Rimuovi sfondo") { background = "" }
                }
                Button("Apri registri") { NSWorkspace.shared.open(model.store.directory.appendingPathComponent("Logs")) }
            }
            UpdateSettingsView()
            Section("Dati") {
                Text(model.store.directory.path).textSelection(.enabled)
                HStack { Button("Importa…") { model.importWorkspace() }; Button("Esporta…") { model.exportWorkspace() } }
                Text("Le chiavi di streaming restano nel Portachiavi e non vengono esportate.").font(.caption)
            }
            Section("Dispositivi") {
                Button("Aggiorna dispositivi AVFoundation") { model.refreshDevices() }
                ForEach(model.devices, id: \.self) { Text("\($0.kind.title) · \($0.name) · indice \($0.id)") }
            }
            Button("Salva impostazioni") { model.persist(); model.refreshCapabilities() }.buttonStyle(.borderedProminent)
        }.formStyle(.grouped)
    }
    func executable(_ title: String, path: Binding<String>) -> some View {
        HStack { TextField(title, text: path, prompt: Text("Automatico")); Button("Scegli…") { if let url = FilePicker.files().first { path.wrappedValue = url.path } } }
    }
}
@MainActor struct StartWizard: View {
    @EnvironmentObject var model: AppModel
    @Environment(\.dismiss) var dismiss
    @State private var channelID: UUID?
    @State private var profileID: UUID?
    @State private var sceneID: UUID?
    @State private var step = 0
    @State private var countdown = 0
    @State private var counting = false
    @State private var countdownTask: Task<Void, Never>?
    @State private var newScene: OrbisCore.Scene?
    @State private var newSceneID: UUID?
    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            Text("Nuova diretta").font(.largeTitle.bold())
            Text("\(step + 1) di 3 · \(["Destinazione", "Sorgenti", "Riepilogo"][step])").foregroundStyle(.secondary)
            Form {
                if step == 0 {
                    Picker("Canale", selection: $channelID) {
                        Text("Seleziona").tag(nil as UUID?)
                        ForEach(model.workspace.channels.filter { $0.enabled }) { Text($0.name).tag(Optional($0.id)) }
                    }
                    Picker("Profilo", selection: $profileID) {
                        Text("Predefinito del canale").tag(nil as UUID?)
                        ForEach(model.workspace.profiles.filter { $0.active }) { Text($0.title).tag(Optional($0.id)) }
                    }
                    if model.workspace.channels.isEmpty { Text("Aggiungi un canale dalla barra laterale prima di avviare la diretta.") }
                } else if step == 1 {
                    Picker("Sorgente", selection: $sceneID) {
                        Text("Playlist / scena del canale").tag(nil as UUID?)
                        ForEach(model.workspace.scenes.filter { !$0.isLayout }) { Text($0.name).tag(Optional($0.id)) }
                    }
                    Button("Componi nuova scena…") { let draft = OrbisCore.Scene(); newSceneID = draft.id; newScene = draft }
                    Stepper("Conto alla rovescia: \(countdown) s", value: $countdown, in: 0...60)
                    Text("Gli slot di un layout devono essere riempiti e salvati come scena prima di trasmettere.").font(.caption)
                } else {
                    Text(model.workspace.channels.first(where: { $0.id == channelID })?.name ?? "")
                    Text("Il server riceverà H.264 e audio stereo. La regia mostrerà l’anteprima del segnale codificato.")
                    Text("L’avanzamento dell’invio non conferma la pubblicazione sulla piattaforma: controlla anche la pagina del canale.").font(.caption).foregroundStyle(.secondary)
                    if counting { Text("Avvio tra \(countdown)…").font(.title) }
                }
            }.formStyle(.grouped)
            HStack {
                Button("Annulla") { countdownTask?.cancel(); dismiss() }.keyboardShortcut(.cancelAction)
                Spacer()
                if step > 0 { Button("Indietro") { step -= 1 }.disabled(counting) }
                Button(step == 2 ? "Trasmetti" : "Continua") {
                    if step < 2 { step += 1 }
                    else {
                        counting = true
                        countdownTask = Task {
                            while countdown > 0 {
                                do { try await Task.sleep(for: .seconds(1)) } catch { return }
                                countdown -= 1
                            }
                            guard !Task.isCancelled, let channel = model.workspace.channels.first(where: { $0.id == channelID }) else { return }
                            model.start(channel: channel, profile: model.workspace.profiles.first { $0.id == profileID }, scene: model.workspace.scenes.first { $0.id == sceneID })
                            dismiss()
                        }
                    }
                }.buttonStyle(.borderedProminent).disabled(channelID == nil || counting)
            }
        }.padding(28).frame(width: 660, height: 520)
        .sheet(item: $newScene, onDismiss: {
            if let id = newSceneID, model.workspace.scenes.contains(where: { $0.id == id }) { sceneID = id }
        }) { SceneEditor(scene: $0).environmentObject(model) }
        .onDisappear { countdownTask?.cancel() }
    }
}
