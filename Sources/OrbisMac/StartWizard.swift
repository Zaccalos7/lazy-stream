import SwiftUI
import OrbisCore

@MainActor struct StartWizard: View {
    @EnvironmentObject private var model: AppModel
    @Environment(\.dismiss) private var dismiss
    let request: StartRequest
    @State private var channelID: UUID?
    @State private var profileID: UUID?
    @State private var sceneID: UUID?
    @State private var folder = ""
    @State private var playlist: [PlaylistItem]?
    @State private var step = 0
    @State private var countdown = 0
    @State private var counting = false
    @State private var countdownTask: Task<Void, Never>?
    @State private var newScene: OrbisCore.Scene?
    @State private var newSceneID: UUID?
    @State private var composedScene: OrbisCore.Scene?
    @State private var localError: String?
    init(request: StartRequest) {
        self.request = request
        _channelID = State(initialValue: request.channelID)
    }
    private var channel: Channel? { model.workspace.channels.first { $0.id == channelID && $0.enabled } }
    private var profile: EncoderProfile? { model.workspace.profiles.first { $0.id == profileID && $0.active } }
    private var scene: OrbisCore.Scene? {
        if let composedScene, composedScene.id == sceneID { return composedScene }
        return model.workspace.scenes.first { $0.id == sceneID && !$0.isLayout }
    }
    private var canContinue: Bool {
        if counting { return false }
        if step == 0 { return profile != nil }
        if step == 1 { return channel != nil }
        return profile != nil && channel != nil && (request.mode == .playlist ? !(playlist ?? []).filter { $0.enabled }.isEmpty : scene != nil)
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            HStack {
                Image(systemName: request.mode == .playlist ? "film.stack" : "dot.radiowaves.left.and.right").font(.largeTitle).foregroundStyle(Color.accentColor)
                VStack(alignment: .leading, spacing: 6) {
                    Text(request.mode == .playlist ? "Avvia playlist" : "Avvia Live").font(.largeTitle.bold())
                    Text("\(step + 1) di 3 · \(["Video e audio", "Configurazione streaming", request.mode == .playlist ? "Cartella video" : "Componi scena"][step])").foregroundStyle(.secondary)
                }
            }
            Form {
                if step == 0 {
                    Section("1. Seleziona le Impostazioni Video e Audio") {
                        Picker("Profilo", selection: $profileID) {
                            Text("Seleziona…").tag(nil as UUID?)
                            ForEach(model.workspace.profiles.filter { $0.active }) { Text($0.title).tag(Optional($0.id)) }
                        }
                        if let profile {
                            Text("\(profile.codec) · \(profile.width)×\(profile.height) · \(profile.frameRate.formatted()) fps")
                            Text("\(profile.videoBitrate / 1000) kbit/s · \(profile.audioCodec) \(profile.audioBitrate / 1000) kbit/s")
                        }
                        if !model.workspace.profiles.contains(where: { $0.active }) {
                            Text("Attiva o crea un profilo in Video e audio.")
                            Button("Apri Video e audio") { model.open(.profiles); dismiss() }
                        }
                    }
                } else if step == 1 {
                    Section("2. Seleziona la Configurazione Streaming") {
                        Picker("Canale", selection: $channelID) {
                            Text("Seleziona…").tag(nil as UUID?)
                            ForEach(model.workspace.channels.filter { $0.enabled }) { Text("\($0.name) · \($0.platform.title)").tag(Optional($0.id)) }
                        }
                        if let channel { Text(channel.description).foregroundStyle(.secondary) }
                        if !model.workspace.channels.contains(where: { $0.enabled }) {
                            Text("Aggiungi o abilita un canale prima di avviare la diretta.")
                            Button("Apri Canali") { model.open(.channels); dismiss() }
                        }
                    }
                } else {
                    Section(request.mode == .playlist ? "3. Scegli la cartella dei video da trasmettere" : "3. Componi la scena") {
                        if request.mode == .playlist {
                            HStack {
                                Text(folder.isEmpty ? "Playlist del canale" : folder).lineLimit(2)
                                Spacer()
                                Button("Scegli cartella…") { chooseFolder() }
                            }
                            Text("\((playlist ?? []).filter { $0.enabled }.count) video abilitati")
                            Toggle("Ripeti playlist", isOn: $model.workspace.loopPlaylist)
                        } else {
                            Picker("Scena", selection: $sceneID) {
                                Text("Seleziona…").tag(nil as UUID?)
                                ForEach(model.workspace.scenes.filter { !$0.isLayout }) { Text($0.name).tag(Optional($0.id)) }
                                if let composedScene, !model.workspace.scenes.contains(where: { $0.id == composedScene.id }) {
                                    Text(composedScene.name).tag(Optional(composedScene.id))
                                }
                            }
                            Button("Componi nuova scena…") {
                                var draft = OrbisCore.Scene(); draft.name = channel?.name ?? "Nuova scena"
                                newSceneID = draft.id; newScene = draft
                            }
                            if let scene { Button("Modifica scena…") { newScene = scene } }
                            Menu("Usa layout") {
                                ForEach(model.workspace.scenes.filter { $0.isLayout }) { layout in
                                    Button(layout.name) {
                                        var draft = OrbisCore.Scene(); draft.name = channel?.name ?? layout.name
                                        draft = SceneGeometry.applying(layout, to: draft); newScene = draft
                                    }
                                }
                            }
                            if let scene { Text("\(scene.width)×\(scene.height) · \(scene.items.count) sorgenti").foregroundStyle(.secondary) }
                        }
                        Stepper("Conto alla rovescia: \(countdown) s", value: $countdown, in: 0...60).disabled(counting)
                    }
                    Section("Riepilogo") {
                        Text("\(profile?.title ?? "") → \(channel?.name ?? "")")
                        Text("Controlla la pubblicazione anche sulla pagina della piattaforma.").font(.caption).foregroundStyle(.secondary)
                        if counting { Text("Avvio tra \(countdown)…").font(.title).monospacedDigit() }
                    }
                }
                if let localError { Text(localError).foregroundStyle(.red) }
            }.formStyle(.grouped).disabled(counting)
            HStack {
                Button("Annulla") { countdownTask?.cancel(); dismiss() }.keyboardShortcut(.cancelAction)
                Spacer()
                if step > 0 { Button("Indietro") { step -= 1 }.disabled(counting) }
                Button(step == 2 ? "Trasmetti" : "Continua") {
                    localError = nil
                    if step < 2 {
                        step += 1
                        if step == 2 { prepareSources() }
                    } else { transmit() }
                }.buttonStyle(.borderedProminent).disabled(!canContinue)
            }
        }.padding(28).frame(width: 720, height: 600)
        .interactiveDismissDisabled(counting)
        .sheet(item: $newScene) { draft in
            SceneEditor(scene: draft, persist: false, commitTitle: "Usa questa scena") { value in
                composedScene = value; sceneID = value.id
            }.environmentObject(model)
        }
        .onDisappear { countdownTask?.cancel() }
        .onChange(of: channelID) { folder = ""; playlist = nil; sceneID = nil; composedScene = nil }
    }
    private func prepareSources() {
        guard let channel else { return }
        sceneID = sceneID ?? channel.sceneID
        guard request.mode == .playlist, playlist == nil else { return }
        do {
            if let saved = model.workspace.playlists[channel.id.uuidString], !saved.isEmpty { playlist = saved }
            else if !channel.videoFolder.isEmpty { folder = channel.videoFolder; playlist = try MediaFiles.playlist(folder: folder) }
        } catch { localError = error.localizedDescription }
    }
    private func chooseFolder() {
        guard let url = FilePicker.files(folder: true).first else { return }
        do { let items = try MediaFiles.playlist(folder: url.path); folder = url.path; playlist = items; localError = nil }
        catch { localError = error.localizedDescription }
    }
    private func transmit() {
        guard let channel, let profile else { return }
        do {
            try profile.validateForStreaming()
            if request.mode == .scene, let scene { try scene.validate() }
        }
        catch { localError = error.localizedDescription; return }
        counting = true
        countdownTask = Task {
            while countdown > 0 {
                do { try await Task.sleep(for: .seconds(1)) } catch { return }
                countdown -= 1
            }
            guard !Task.isCancelled else { return }
            model.persist()
            if request.mode == .playlist { model.start(channel: channel, profile: profile, playlist: playlist) }
            else { model.start(channel: channel, profile: profile, scene: scene) }
            dismiss()
        }
    }
}
