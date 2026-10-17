import SwiftUI
import OrbisCore

@MainActor struct ChannelsView: View {
    @EnvironmentObject var model: AppModel
    @State private var draft: Channel?
    @State private var activeOnly = -1
    @State private var platform: Platform?
    var body: some View {
        List {
            ForEach(model.workspace.channels.filter { (model.search.isEmpty || $0.name.localizedCaseInsensitiveContains(model.search)) && (activeOnly == -1 || $0.enabled == (activeOnly == 1)) && (platform == nil || $0.platform == platform) }) { channel in
                HStack(spacing: 16) {
                    Image(systemName: "antenna.radiowaves.left.and.right")
                        .font(.system(size: 20, weight: .medium)).foregroundStyle(Color.accentColor)
                        .frame(width: 44, height: 44)
                        .background(Color.accentColor.opacity(0.08), in: RoundedRectangle(cornerRadius: 12))
                    VStack(alignment: .leading, spacing: 5) { Text(channel.name).font(.headline); Text("\(channel.platform.title) · \(channel.enabled ? "Abilitato" : "Disabilitato")").foregroundStyle(.secondary) }
                    Spacer(); Button("Avvia") { model.startRequest = StartRequest(mode: channel.sceneID == nil ? .playlist : .scene, channelID: channel.id) }.disabled(!channel.enabled)
                    Button("Modifica") { draft = channel }
                    Button(channel.enabled ? "Disattiva" : "Attiva") { model.setChannelEnabled(channel.id, !channel.enabled) }
                    Button("Elimina", role: .destructive) { model.removeChannel(channel.id) }
                }.padding(.vertical, 8)
            }
        }
        .overlay { if model.workspace.channels.isEmpty { ContentUnavailableView("Nessun canale", systemImage: "antenna.radiowaves.left.and.right", description: Text("Aggiungi il server e la chiave ricevuti dalla piattaforma.")) } }
        .safeAreaInset(edge: .top) {
            HStack {
                ActivationPicker(selection: $activeOnly)
                Picker("Piattaforma", selection: $platform) {
                    Text("Tutte le piattaforme").tag(nil as Platform?)
                    ForEach(Platform.allCases, id: \.self) { Text($0.title).tag(Optional($0)) }
                }
            }.padding().background(.bar)
        }
        .toolbar { Button { draft = Channel() } label: { Label("Aggiungi canale", systemImage: "plus") } }
        .sheet(item: $draft) { ChannelEditor(channel: $0).environmentObject(model) }
    }
}
@MainActor struct ChannelEditor: View {
    @EnvironmentObject var model: AppModel
    @Environment(\.dismiss) var dismiss
    @State var channel: Channel
    @State private var key = ""
    @State private var localError: String?
    var body: some View {
        VStack {
            Text("Canale").font(.title2.bold()).padding(.top)
            Form {
                Section("Destinazione") {
                    TextField("Nome canale", text: $channel.name)
                    Picker("Piattaforma", selection: Binding(get: { channel.platform }, set: { channel.streamURL = $0.ingest })) {
                        ForEach(Platform.allCases, id: \.self) { Text($0.title).tag($0) }
                    }
                    TextField("Server RTMP/RTMPS", text: $channel.streamURL)
                    if channel.platform == .kick || channel.platform == .facebook {
                        Text("Usa il server assegnato dal pannello della piattaforma.").font(.caption).foregroundStyle(.secondary)
                    }
                    if channel.platform == .tiktok { Text("La chiave TikTok viene richiesta a ogni avvio e non viene salvata.").font(.caption) }
                    else { SecureField("Chiave di streaming", text: $key) }
                    TextField("Nome pubblico / ID trasmissione", text: $channel.streamName)
                    TextField("Descrizione", text: $channel.description)
                    Toggle("Canale abilitato", isOn: $channel.enabled)
                }
                Section("Sorgente e codifica") {
                    HStack { TextField("Cartella video", text: $channel.videoFolder); Button("Scegli…") { if let url = FilePicker.files(folder: true).first { channel.videoFolder = url.path } } }
                    Picker("Scena", selection: $channel.sceneID) {
                        Text("Playlist").tag(nil as UUID?)
                        ForEach(model.workspace.scenes.filter { !$0.isLayout }) { Text($0.name).tag(Optional($0.id)) }
                    }
                    Picker("Profilo", selection: $channel.profileID) {
                        Text("Predefinito").tag(nil as UUID?)
                        ForEach(model.workspace.profiles) { Text($0.title).tag(Optional($0.id)) }
                    }
                }
                Section("Pulizia dello storico") {
                    Toggle("Pulizia automatica", isOn: $channel.cleanupEnabled)
                    Stepper("Intervallo: \(channel.cleanupIntervalMonths) mesi", value: $channel.cleanupIntervalMonths, in: 1...24)
                    Stepper("Elimina record oltre \(channel.cleanupOlderThanMonths) mesi", value: $channel.cleanupOlderThanMonths, in: 1...120)
                }
                if let localError { Text(localError).foregroundStyle(.red) }
            }.formStyle(.grouped)
            HStack { Button("Annulla") { dismiss() }; Spacer(); Button("Salva") {
                do {
                    guard !channel.name.trimmingCharacters(in: .whitespaces).isEmpty else { throw OrbisError.invalid("Inserisci il nome del canale.") }
                    _ = try channel.destination(key: channel.platform == .tiktok ? "temporary-key" : key)
                    if channel.platform == .tiktok { KeychainStore.delete(channel.id) } else { try KeychainStore.save(key, id: channel.id) }
                    var next = model.workspace
                    if let index = next.channels.firstIndex(where: { $0.id == channel.id }) { next.channels[index] = channel } else { next.channels.append(channel) }
                    try model.store.save(next); model.workspace = next; dismiss()
                } catch { localError = error.localizedDescription }
            }.buttonStyle(.borderedProminent) }.padding()
        }.frame(width: 680, height: 710).onAppear { do { key = try KeychainStore.read(channel.id) } catch { localError = error.localizedDescription } }
    }
}
@MainActor struct ProfilesView: View {
    @EnvironmentObject var model: AppModel
    @State private var draft: EncoderProfile?
    @State private var activeOnly = -1
    var body: some View {
        List {
            ForEach(model.workspace.profiles.filter { (model.search.isEmpty || $0.title.localizedCaseInsensitiveContains(model.search)) && (activeOnly == -1 || $0.active == (activeOnly == 1)) }) { profile in
                HStack(spacing: 16) {
                    Image(systemName: "slider.horizontal.3").font(.system(size: 20, weight: .medium))
                        .foregroundStyle(.secondary).frame(width: 44, height: 44)
                        .background(.quaternary, in: RoundedRectangle(cornerRadius: 12))
                    VStack(alignment: .leading, spacing: 5) {
                        Text(profile.title).font(.headline)
                        Text("\(profile.codec) · \(profile.width)×\(profile.height) · \(profile.frameRate.formatted()) fps · \(Double(profile.videoBitrate) / 1_000_000, specifier: "%.1f") Mbit/s").foregroundStyle(.secondary)
                    }
                    Spacer(); Button("Modifica") { draft = profile }
                    Button(profile.active ? "Disattiva" : "Attiva") { model.setProfileActive(profile.id, !profile.active) }
                    Button("Duplica") { var copy = profile; copy.id = UUID(); copy.title += " copia"; draft = copy }
                    Button("Elimina", role: .destructive) { model.removeProfile(profile.id) }
                }.padding(.vertical, 8)
            }
        }.safeAreaInset(edge: .top) { HStack { ActivationPicker(selection: $activeOnly); Spacer() }.padding().background(.bar) }
        .toolbar {
            Button { draft = EncoderProfile() } label: { Label("Nuovo profilo", systemImage: "plus") }
            Menu("Preset piattaforma") {
                ForEach(Platform.allCases, id: \.self) { platform in
                    Button(platform.title) {
                        draft = PlatformPresets.profile(platform)
                    }
                }
            }
            Menu("Preset leggero") {
                ForEach(Platform.destinations, id: \.self) { platform in
                    Button(platform.title) { draft = PlatformPresets.profile(platform, light: true) }
                }
            }
            Button("Aggiungi preset mancanti") {
                model.perform {
                    var next = model.workspace
                    for profile in PlatformPresets.defaults() where !next.profiles.contains(where: { $0.defaultPlatform == profile.defaultPlatform && $0.isDefault == profile.isDefault }) {
                        next.profiles.append(profile)
                    }
                    try model.store.save(next); model.workspace = next
                }
            }
        }
        .sheet(item: $draft) { profile in
            ProfileEditor(profile: profile) { value in
                try value.validate()
                var next = model.workspace
                if let index = next.profiles.firstIndex(where: { $0.id == value.id }) { next.profiles[index] = value } else { next.profiles.append(value) }
                try model.store.save(next); model.workspace = next
            }
        }
    }
}
@MainActor struct ProfileFields: View {
    @EnvironmentObject var model: AppModel
    @Binding var profile: EncoderProfile
    var live = false
    var body: some View {
        Section("Video") {
            TextField("Titolo", text: $profile.title)
            Picker("Encoder", selection: $profile.codec) {
                Text("Apple H.264 (VideoToolbox)").tag("h264_videotoolbox")
                Text("Apple HEVC (VideoToolbox)").tag("hevc_videotoolbox")
                ForEach(CodecCatalog.videoEncoders.filter { !$0.hasSuffix("_videotoolbox") && (model.capabilities.encoders.isEmpty || model.capabilities.encoders.contains($0) || profile.codec == $0) }, id: \.self) { Text($0).tag($0) }
            }
            Picker("Qualità", selection: $profile.quality) { ForEach(EncoderQuality.allCases, id: \.self) { Text($0.rawValue.capitalized).tag($0) } }
            Text("Auto misura l’encoder sul Mac e sceglie bilanciato o leggero. VideoToolbox usa la priorità di velocità Apple; non usa i preset x264.").font(.caption).foregroundStyle(.secondary)
            Picker("Formato pixel", selection: $profile.pixelFormat) { ForEach(CodecCatalog.pixelFormats, id: \.self) { Text($0).tag($0) } }
            Picker("Contenitore", selection: $profile.format) { ForEach(CodecCatalog.formats, id: \.self) { Text($0.uppercased()).tag($0) } }.disabled(live)
            TextField("Bitrate video (bit/s)", value: $profile.videoBitrate, format: .number)
            TextField("Intervallo keyframe (s)", value: $profile.keyframeSeconds, format: .number).disabled(live)
            HStack { TextField("Larghezza", value: $profile.width, format: .number); TextField("Altezza", value: $profile.height, format: .number) }
            TextField("Frame rate", value: $profile.frameRate, format: .number)
            Text("0×0 conserva la risoluzione; 0 fps conserva la frequenza. YouTube, Kick, Facebook e TikTok mantengono il formato della prima sorgente.").font(.caption).foregroundStyle(.secondary)
        }
        Section("Audio") {
            Picker("Codec audio", selection: $profile.audioCodec) { ForEach(CodecCatalog.audioEncoders.filter { model.capabilities.encoders.isEmpty || model.capabilities.encoders.contains($0) || profile.audioCodec == $0 }, id: \.self) { Text($0).tag($0) } }
            TextField("Bitrate audio (bit/s)", value: $profile.audioBitrate, format: .number)
        }
        Section("Ottimizzazione") {
            Toggle("Bassa latenza", isOn: $profile.lowLatency)
            Toggle("Bitrate adattivo", isOn: $profile.adaptiveBitrate)
            if profile.codec.hasPrefix("lib") { TextField("Preset (vuoto = qualità selezionata)", text: $profile.preset); TextField("Tune", text: $profile.tune) }
            Toggle("Profilo attivo", isOn: $profile.active)
            Toggle("Configurazione predefinita", isOn: $profile.isDefault)
            Picker("Piattaforma predefinita", selection: $profile.defaultPlatform) { ForEach(Platform.allCases, id: \.self) { Text($0.title).tag($0) } }
        }
        Section("Opzioni FFmpeg aggiuntive") {
            ForEach($profile.options) { $option in
                HStack {
                    TextField("Chiave senza -", text: $option.key); TextField("Valore", text: $option.value)
                    Button { profile.options.removeAll { $0.id == option.id } } label: { Image(systemName: "minus.circle") }
                }
            }
            Button("Aggiungi opzione") { profile.options.append(EncoderOption()) }
        }
    }
}
@MainActor struct ProfileEditor: View {
    @Environment(\.dismiss) var dismiss
    @State var profile: EncoderProfile
    var commit: (EncoderProfile) throws -> Void
    @State private var localError: String?
    var body: some View {
        VStack {
            Text("Profilo video e audio").font(.title2.bold()).padding(.top)
            Form { ProfileFields(profile: $profile); if let localError { Text(localError).foregroundStyle(.red) } }.formStyle(.grouped)
            HStack { Button("Annulla") { dismiss() }; Spacer(); Button("Salva") {
                do { try profile.validate(); try commit(profile); dismiss() } catch { localError = error.localizedDescription }
            }.buttonStyle(.borderedProminent) }.padding()
        }.frame(width: 700, height: 780)
    }
}
@MainActor struct PlaylistView: View {
    @EnvironmentObject var model: AppModel
    @State private var selected: UUID?
    @State private var preview: MediaSelection?
    var body: some View {
        VStack {
            HStack {
                Picker("Canale", selection: $selected) {
                    Text("Seleziona un canale").tag(nil as UUID?)
                    ForEach(model.workspace.channels) { Text($0.name).tag(Optional($0.id)) }
                }.frame(maxWidth: 380)
                Spacer()
                if let selected {
                    Button("Aggiungi video…") { model.importPlaylist(channel: selected) }
                    Button("Aggiungi cartella…") { model.importPlaylist(channel: selected, folder: true) }
                }
            }.padding()
            if let selected {
                let key = selected.uuidString
                let items = model.workspace.playlists[key] ?? []
                List {
                    ForEach(Array(items.enumerated()), id: \.element.id) { index, item in
                        if model.search.isEmpty || item.name.localizedCaseInsensitiveContains(model.search) {
                            HStack {
                                Toggle("", isOn: Binding(get: { model.workspace.playlists[key]?.first { $0.id == item.id }?.enabled ?? true }, set: { value in update(key, id: item.id) { $0.enabled = value } })).labelsHidden()
                                VStack(alignment: .leading) { Text(item.name); Text(item.path).font(.caption).foregroundStyle(.secondary) }
                                if item.duration > 0 { Text(clock(item.duration)).font(.caption).monospacedDigit() }
                                Spacer()
                                Button { preview = MediaSelection(path: item.path) } label: { Image(systemName: "play.rectangle") }.help("Anteprima")
                                Picker("Profilo video", selection: Binding(get: { model.workspace.playlists[key]?.first { $0.id == item.id }?.profileID }, set: { value in update(key, id: item.id) { $0.profileID = value } })) {
                                    Text("Del canale").tag(nil as UUID?)
                                    ForEach(model.workspace.profiles.filter { $0.active }) { Text($0.title).tag(Optional($0.id)) }
                                }.frame(width: 180)
                                TextField("Riprendi da (s)", value: Binding(get: { model.workspace.playlists[key]?.first { $0.id == item.id }?.resumeSeconds ?? 0 }, set: { value in update(key, id: item.id) { $0.resumeSeconds = max(0, value) } }), format: .number).frame(width: 100).help("Posizione di ripresa in secondi")
                                Button { move(key, from: index, delta: -1) } label: { Image(systemName: "arrow.up") }.disabled(index == 0)
                                Button { move(key, from: index, delta: 1) } label: { Image(systemName: "arrow.down") }.disabled(index == items.count - 1)
                                Button("Rimuovi", role: .destructive) { model.workspace.playlists[key]?.removeAll { $0.id == item.id }; model.persist() }
                            }
                        }
                    }
                }
                HStack {
                    Toggle("Ripeti playlist", isOn: $model.workspace.loopPlaylist).onChange(of: model.workspace.loopPlaylist) { model.persist() }
                    Spacer()
                    Button("Trasmetti playlist") { model.startRequest = StartRequest(mode: .playlist, channelID: selected) }.disabled(!items.contains { $0.enabled })
                }.padding()
            } else { ContentUnavailableView("Playlist", systemImage: "list.bullet.rectangle", description: Text("Seleziona un canale per ordinare e abilitare i video.")) }
        }.onAppear { if selected == nil { selected = model.workspace.channels.first?.id } }
        .sheet(item: $preview) { MediaPreviewPanel(path: $0.path).environmentObject(model) }
    }
    private func update(_ key: String, id: UUID, action: (inout PlaylistItem) -> Void) {
        if let index = model.workspace.playlists[key]?.firstIndex(where: { $0.id == id }) { action(&model.workspace.playlists[key]![index]); model.persist() }
    }
    private func move(_ key: String, from: Int, delta: Int) {
        guard let count = model.workspace.playlists[key]?.count, (0..<count).contains(from + delta) else { return }
        model.workspace.playlists[key]?.swapAt(from, from + delta); model.persist()
    }
}
@MainActor struct HistoryView: View {
    @State private var channelFilter: UUID?
    @State private var statusFilter = ""
    @State private var newestFirst = true
    private func state(_ entry: LiveHistory) -> String {
        model.state(entry)
    }
    private var entries: [LiveHistory] {
        model.workspace.history.filter { entry in
            (channelFilter == nil || entry.channel.id == channelFilter) && (statusFilter.isEmpty || state(entry) == statusFilter) &&
            (model.search.isEmpty || entry.channel.name.localizedCaseInsensitiveContains(model.search) || entry.playlist.contains(where: { $0.name.localizedCaseInsensitiveContains(model.search) }) || entry.scene?.name.localizedCaseInsensitiveContains(model.search) == true)
        }.sorted { newestFirst ? $0.start > $1.start : $0.start < $1.start }
    }
    @EnvironmentObject var model: AppModel
    @State private var months = 6
    var body: some View {
        VStack {
            HStack {
                Picker("Periodo", selection: $months) {
                    Text("Tutto lo storico").tag(-1); Text("Prima di ieri").tag(0)
                    ForEach([1, 3, 6, 12, 24, 36, 60, 120], id: \.self) { Text("Oltre \($0) mesi").tag($0) }
                }.frame(width: 240)
                Button("Pulisci storico", role: .destructive) { model.cleanup(months: months) }.disabled(model.cleanupProgress.running)
                Spacer()
                Button("Esporta…") {
                    model.perform { let encoder = JSONEncoder(); encoder.outputFormatting = .prettyPrinted; try FilePicker.exportJSON(encoder.encode(model.workspace.history), name: "Storico-OrbisStream.json") }
                }
            }.padding()
            HStack {
                Picker("Canale", selection: $channelFilter) {
                    Text("Tutti i canali").tag(nil as UUID?)
                    ForEach(model.workspace.channels) { Text($0.name).tag(Optional($0.id)) }
                }
                Picker("Stato", selection: $statusFilter) {
                    Text("Tutti gli stati").tag(""); Text("In corso").tag("live"); Text("Terminata").tag("ended"); Text("Errore").tag("error"); Text("Interrotta").tag("interrupted")
                }
                Toggle("Più recenti prima", isOn: $newestFirst)
            }.padding(.horizontal)
            if model.cleanupProgress.running || model.cleanupProgress.deletedLives > 0 {
                ProgressView(value: Double(model.cleanupProgress.deletedVideos), total: Double(max(1, model.cleanupProgress.totalVideos)))
                Text("\(model.cleanupProgress.deletedLives) dirette eliminate · \(model.cleanupProgress.currentVideo)").font(.caption)
            }
            List {
                ForEach(entries) { history in
                    HStack {
                        VStack(alignment: .leading, spacing: 5) {
                            Text(history.channel.name).font(.headline)
                            Text(history.start.formatted()).foregroundStyle(.secondary)
                            Text(history.scene?.name ?? "\(history.playlist.count) video · \(history.channel.videoFolder)").font(.caption)
                            if !history.playlist.isEmpty {
                                DisclosureGroup("Video della diretta") {
                                    ForEach(history.playlist) { item in
                                        HStack {
                                            Text(item.name); Text(clock(item.resumeSeconds)).monospacedDigit()
                                            Spacer()
                                            Button("Dall’inizio") { replay(history, item: item, resume: false) }
                                            Button("Dalla posizione") { replay(history, item: item, resume: true) }
                                            Button("Elimina video", role: .destructive) { model.deleteHistoryVideo(historyID: history.id, videoID: item.id) }
                                        }.font(.caption)
                                    }
                                }
                            }
                            if let error = history.error { Text(error).foregroundStyle(.red).font(.caption) }
                        }
                        Spacer(); Text(LocalizedStringKey(model.stateTitle(history)))
                        if history.error != nil { Button("Segnala errore") { model.report = history } }
                        Button("Riprendi") { model.replay(history, resume: true) }.disabled(state(history) == "live")
                        Button("Ricomincia") { model.replay(history, resume: false) }.disabled(state(history) == "live")
                        Button("Elimina", role: .destructive) { model.removeHistory(history.id) }.disabled(state(history) == "live")
                    }.padding(.vertical, 8)
                }
            }
        }
    }
    private func replay(_ history: LiveHistory, item: PlaylistItem, resume: Bool) {
        var value = item; value.enabled = true; if !resume { value.resumeSeconds = 0 }
        var channel = history.channel; channel.sceneID = nil
        model.start(channel: channel, profile: history.profile, playlist: [value])
    }
}

@MainActor struct RecoveryView: View {
    @EnvironmentObject var model: AppModel
    var interrupted: [PlaybackCheckpoint] {
        model.workspace.interruptedRuns.filter { checkpoint in !model.sessions.contains { $0.historyID == checkpoint.historyID && $0.isRunning } }
    }
    var body: some View {
        List(interrupted) { checkpoint in
            HStack {
                VStack(alignment: .leading) {
                    Text(model.workspace.channels.first { $0.id == checkpoint.channelID }?.name ?? "Canale rimosso").font(.headline)
                    Text("Video \(checkpoint.index + 1) · \(clock(checkpoint.position)) · \(checkpoint.updated.formatted())").foregroundStyle(.secondary)
                }
                Spacer()
                Button("Sblocca") { model.unlock(checkpoint, restart: false) }
                Button("Riprendi diretta") { model.unlock(checkpoint, restart: true) }
            }
        }.overlay { if interrupted.isEmpty { ContentUnavailableView("Nessuna sessione da recuperare", systemImage: "checkmark.shield") } }
    }
}
