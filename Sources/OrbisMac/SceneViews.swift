import SwiftUI
import AppKit
import AVKit
import OrbisCore

@MainActor struct ScenesView: View {
    @EnvironmentObject var model: AppModel
    @State private var draft: OrbisCore.Scene?
    var body: some View {
        List {
            ForEach(model.workspace.scenes.filter { model.search.isEmpty || $0.name.localizedCaseInsensitiveContains(model.search) }) { scene in
                HStack {
                    Label { VStack(alignment: .leading) { Text(scene.name).font(.headline); Text("\(scene.isLayout ? "Layout" : "Scena") · \(scene.width)×\(scene.height) · \(scene.items.count) sorgenti").foregroundStyle(.secondary) } } icon: { Image(systemName: scene.isLayout ? "rectangle.3.group" : "video") }
                    Spacer(); Button("Modifica") { draft = scene }
                    Button(scene.isLayout ? "Crea scena" : "Duplica") { var copy = scene; copy.id = UUID(); copy.isLayout = false; copy.name += " copia"; copy.items = copy.items.map { var item = $0; item.id = UUID(); return item }; draft = copy }
                    Button("Elimina", role: .destructive) { model.removeScene(scene.id) }
                }.padding(.vertical, 8)
            }
        }
        .toolbar {
            Button("Nuova scena") { draft = OrbisCore.Scene() }
            Button("Nuovo layout") { var scene = OrbisCore.Scene(); scene.isLayout = true; draft = scene }
        }
        .sheet(item: $draft) { SceneEditor(scene: $0).environmentObject(model) }
    }
}
@MainActor struct SceneEditor: View {
    @EnvironmentObject var model: AppModel
    @Environment(\.dismiss) var dismiss
    @State var scene: OrbisCore.Scene
    private let original: OrbisCore.Scene
    private let persist: Bool
    private let commitTitle: String
    private let commit: ((OrbisCore.Scene) async throws -> Void)?
    init(scene: OrbisCore.Scene, persist: Bool = true, commitTitle: String = "Salva", commit: ((OrbisCore.Scene) async throws -> Void)? = nil) {
        _scene = State(initialValue: scene); original = scene
        self.persist = persist; self.commitTitle = commitTitle; self.commit = commit
    }
    @State private var selected: UUID?
    @State private var localError: String?
    @State private var previewing = false
    @State private var saving = false
    var body: some View {
        VStack(spacing: 0) {
            HStack { Text(scene.isLayout ? "Editor layout" : "Editor scena").font(.title2.bold()); Spacer(); TextField("Nome", text: $scene.name).frame(width: 280) }.padding()
            HSplitView {
                VStack(alignment: .leading) {
                    HStack {
                        Picker("Canvas", selection: $previewing) { Text("Editor").tag(false); Text("Anteprima composizione").tag(true) }
                            .pickerStyle(.segmented).frame(maxWidth: 360).disabled(scene.isLayout)
                        Spacer()
                        Button("Rileva dispositivi") { model.refreshDevices() }
                    }.padding(.horizontal)
                    Group {
                        if previewing { CompositionPreview(scene: scene) }
                        else { CanvasView(scene: $scene, selected: $selected) }
                    }.frame(minHeight: 280).padding()
                    HStack {
                        Menu("Applica layout") {
                            ForEach(model.workspace.scenes.filter { $0.isLayout }) { layout in
                                Button(layout.name) { scene = SceneGeometry.applying(layout, to: scene) }
                            }
                        }
                        TextField("Larghezza canvas", value: Binding(get: { scene.width }, set: { scene = SceneGeometry.resized(scene, width: $0, height: scene.height) }), format: .number); TextField("Altezza canvas", value: Binding(get: { scene.height }, set: { scene = SceneGeometry.resized(scene, width: scene.width, height: $0) }), format: .number) }.padding(.horizontal)
                    HStack {
                        Menu("Risoluzione canvas") {
                            ForEach([720, 1080, 1440, 2160], id: \.self) { height in
                                Button("\(height * 16 / 9)×\(height)") { scene = SceneGeometry.resized(scene, width: height * 16 / 9, height: height) }
                            }
                        }
                        Button("Verticale / orizzontale") { scene = SceneGeometry.resized(scene, width: scene.height, height: scene.width) }
                        Button("Dalla sorgente") { sourceResolution() }
                        Button("Svuota canvas", role: .destructive) { scene.items.removeAll(); selected = nil }
                    }.padding(.horizontal)
                    TextField("Descrizione", text: $scene.description).padding(.horizontal)
                    List(selection: $selected) {
                        ForEach(scene.items) { item in
                            HStack { Image(systemName: item.kind == .microphone ? "mic" : item.kind == .overlay ? "square.3.layers.3d" : "video"); Text(item.label); Spacer(); Text(item.kind.title).foregroundStyle(.secondary) }.tag(item.id)
                        }
                        .onMove { scene.items.move(fromOffsets: $0, toOffset: $1) }
                    }
                    HStack {
                        Menu("Aggiungi sorgente") {
                            ForEach(SourceKind.allCases, id: \.self) { kind in Button(kind.title) { add(kind) } }
                            Divider()
                            ForEach(model.devices.filter { device in !scene.items.contains { $0.kind == device.kind && $0.target == device.id } }, id: \.self) { device in
                                Button(device.name) { addDevice(device) }
                            }
                            Divider()
                            ForEach(model.workspace.overlays, id: \.self) { path in
                                Button(MediaFiles.displayName(path)) { addOverlay(path) }
                            }
                            Button("Importa overlay…") { model.importOverlay() }
                        }
                        if let selected, let index = scene.items.firstIndex(where: { $0.id == selected }) {
                            Button { if index > 0 { scene.items.swapAt(index, index - 1) } } label: { Image(systemName: "arrow.down.square") }.help("Sposta sotto")
                            Button { if index + 1 < scene.items.count { scene.items.swapAt(index, index + 1) } } label: { Image(systemName: "arrow.up.square") }.help("Sposta sopra")
                            Button("Rimuovi", role: .destructive) { scene.items.removeAll { $0.id == selected }; self.selected = nil }
                        }
                    }.padding()
                }.frame(minWidth: 550)
                ScrollView {
                    if let selected, let index = scene.items.firstIndex(where: { $0.id == selected }) {
                        SourceInspector(item: $scene.items[index], canvasWidth: scene.width, canvasHeight: scene.height).environmentObject(model)
                    } else { ContentUnavailableView("Seleziona una sorgente", systemImage: "cursorarrow", description: Text("Trascina i riquadri sul canvas per spostarli.")) }
                }.frame(width: 340)
            }
            if let localError { Text(localError).foregroundStyle(.red).padding(8) }
            HStack {
                if persist { Toggle("Salva come layout", isOn: $scene.isLayout) }
                if saving { ProgressView().controlSize(.small) }
                Spacer(); Button("Annulla") { dismiss() }
                Button(LocalizedStringKey(commitTitle)) { save() }.buttonStyle(.borderedProminent).disabled(saving)
            }.padding()
        }.frame(width: 1120, height: 780).disabled(saving).interactiveDismissDisabled(scene != original || saving)
        .onAppear { if model.devices.isEmpty { model.refreshDevices() } }
    }
    private func save() {
        saving = true; localError = nil
        Task {
            defer { saving = false }
            do {
                try scene.validate(requireSources: !scene.isLayout)
                var result = scene
                if result.isLayout {
                    result.items = result.items.map { source in
                        var slot = source
                        if slot.kind != .overlay { slot.kind = .file; slot.target = "" }
                        slot.audioEnabled = false; return slot
                    }
                }
                if let commit { try await commit(result) }
                if persist {
                    var next = model.workspace
                    if let index = next.scenes.firstIndex(where: { $0.id == result.id }) { next.scenes[index] = result } else { next.scenes.append(result) }
                    try model.store.save(next); model.workspace = next
                }
                dismiss()
            } catch { localError = error.localizedDescription }
        }
    }
    private func addDevice(_ device: CaptureDevice) {
        guard !scene.items.contains(where: { $0.kind == device.kind && $0.target == device.id }) else { return }
        add(device.kind)
        if let index = scene.items.firstIndex(where: { $0.id == selected }) { scene.items[index].target = device.id; scene.items[index].label = device.name }
    }
    private func addOverlay(_ path: String) {
        add(.overlay)
        if let index = scene.items.firstIndex(where: { $0.id == selected }) { scene.items[index].target = path; scene.items[index].label = MediaFiles.displayName(path) }
    }
    private func sourceResolution() {
        guard let source = scene.items.first(where: { $0.id == selected && $0.kind == .file }) ?? scene.items.first(where: { $0.kind == .file && !$0.target.isEmpty }) else { return }
        Task {
            do {
                let info = try await Probe.media(source.target, executable: Executables.resolve("ffprobe", configured: model.workspace.ffprobePath))
                scene = SceneGeometry.resized(scene, width: info.width, height: info.height)
            } catch { localError = error.localizedDescription }
        }
    }
    func add(_ kind: SourceKind) {
        var item = SceneItem(); item.kind = kind; item.label = kind.title
        item.width = scene.width; item.height = scene.height; item.audioEnabled = kind != .overlay && kind != .camera && kind != .screen
        if kind == .microphone { item.width = 2; item.height = 2 }
        scene.items.append(item); selected = item.id
    }
}
@MainActor struct CanvasView: View {
    @EnvironmentObject var model: AppModel
    @FocusState private var keyboardFocus: Bool
    @Binding var scene: OrbisCore.Scene
    @Binding var selected: UUID?
    @State private var dragOrigins: [UUID: CGPoint] = [:]
    @State private var resizeOrigins: [UUID: CGSize] = [:]
    @State private var snapping = true
    var body: some View {
        GeometryReader { proxy in
            let scale = min(proxy.size.width / CGFloat(max(2, scene.width)), proxy.size.height / CGFloat(max(2, scene.height)))
            ZStack(alignment: .topLeading) {
                Color.black
                ForEach($scene.items) { $item in
                    if item.kind != .microphone {
                        ZStack {
                            if (item.kind == .overlay || item.kind == .file) && !item.target.isEmpty { CanvasMediaPreview(path: item.target) }
                            else { Rectangle().fill(item.kind == .overlay ? .purple.opacity(0.35) : .blue.opacity(0.25)) }
                            VStack { Text(item.label).font(.headline); Text("\(item.width)×\(item.height)").font(.caption) }.foregroundStyle(.white)
                        }
                        .frame(width: CGFloat(item.width) * scale, height: CGFloat(item.height) * scale)
                        .overlay(Rectangle().stroke(selected == item.id ? Color.accentColor : .white.opacity(0.4), lineWidth: selected == item.id ? 3 : 1))
                        .overlay(alignment: .bottomTrailing) {
                            if selected == item.id {
                                Circle().fill(Color.accentColor).frame(width: 14, height: 14)
                                    .highPriorityGesture(DragGesture().onChanged { value in
                                        if resizeOrigins[item.id] == nil { resizeOrigins[item.id] = CGSize(width: CGFloat(item.width), height: CGFloat(item.height)) }
                                        let original = resizeOrigins[item.id]!
                                        item.width = max(2, min(scene.width - item.x, Int(original.width + value.translation.width / max(0.001, scale))))
                                        item.height = max(2, min(scene.height - item.y, Int(original.height + value.translation.height / max(0.001, scale))))
                                        item.width -= item.width % 2; item.height -= item.height % 2
                                    }.onEnded { _ in resizeOrigins.removeValue(forKey: item.id) })
                            }
                        }
                        .offset(x: CGFloat(item.x) * scale, y: CGFloat(item.y) * scale)
                        .onTapGesture { selected = item.id; keyboardFocus = true }
                        .gesture(DragGesture().onChanged { value in
                            selected = item.id
                            if dragOrigins[item.id] == nil { dragOrigins[item.id] = CGPoint(x: CGFloat(item.x), y: CGFloat(item.y)) }
                            let origin = dragOrigins[item.id]!
                            item.x = max(0, min(scene.width - item.width, Int(origin.x + value.translation.width / scale)))
                            item.y = max(0, min(scene.height - item.height, Int(origin.y + value.translation.height / scale)))
                            if snapping { item = SceneGeometry.snapped(item, in: scene, tolerance: Int(8 / max(0.001, scale))) }
                        }.onEnded { _ in dragOrigins.removeValue(forKey: item.id) })
                    }
                }
            }
            .frame(width: CGFloat(scene.width) * scale, height: CGFloat(scene.height) * scale)
            .clipped().clipShape(RoundedRectangle(cornerRadius: 12))
            .contextMenu { Toggle("Aggancia a bordi e centri", isOn: $snapping) }
            .focusable().focused($keyboardFocus)
            .onMoveCommand { direction in
                guard let selected, let index = scene.items.firstIndex(where: { $0.id == selected }) else { return }
                let step = NSEvent.modifierFlags.contains(.shift) ? 10 : 1
                switch direction {
                case .left: scene.items[index].x -= step
                case .right: scene.items[index].x += step
                case .up: scene.items[index].y -= step
                case .down: scene.items[index].y += step
                @unknown default: break
                }
                scene.items[index] = SceneGeometry.constrain(scene.items[index], width: scene.width, height: scene.height)
            }
            .onDeleteCommand { if let selected { scene.items.removeAll { $0.id == selected }; self.selected = nil } }
            .dropDestination(for: URL.self) { urls, location in
                model.perform {
                    for (offset, url) in urls.enumerated() {
                        let picture = MediaFiles.isPicture(url.path)
                        guard picture || MediaFiles.videos.contains(url.pathExtension.lowercased()) else { throw OrbisError.invalid("Trascina un video o un’immagine supportata.") }
                        var item = SceneItem(); item.kind = picture ? .overlay : .file; item.label = url.lastPathComponent; item.audioEnabled = !picture
                        if picture { item.target = try model.store.importMedia(url, collection: "Overlays") } else { item.target = url.path }
                        if picture && !model.workspace.overlays.contains(item.target) { model.workspace.overlays.append(item.target); model.persist() }
                        item.width = min(640, scene.width); item.height = min(360, scene.height)
                        let x = Int(location.x / max(0.001, scale)) + offset * 20
                        let y = Int(location.y / max(0.001, scale)) + offset * 20
                        if offset == 0 {
                            let placement = SceneGeometry.placing(item, in: scene, x: x, y: y)
                            scene = placement.scene; selected = placement.selection
                        } else {
                            item.x = x; item.y = y; item = SceneGeometry.constrain(item, width: scene.width, height: scene.height)
                            scene.items.append(item); selected = item.id
                        }
                        model.prepareMedia(item.target)
                    }
                }
                return !urls.isEmpty
            }
        }
    }
}
@MainActor struct SourceInspector: View {
    @EnvironmentObject var model: AppModel
    @Binding var item: SceneItem
    let canvasWidth: Int
    let canvasHeight: Int
    @State private var player: AVPlayer?
    @State private var snapshot: NSImage?
    @State private var previewBusy = false
    @State private var previewError: String?
    @State private var previewTask: Task<Void, Never>?
    var body: some View {
        Form {
            Section("Sorgente") {
                TextField("Etichetta", text: $item.label)
                Picker("Tipo", selection: $item.kind) { ForEach(SourceKind.allCases, id: \.self) { Text($0.title).tag($0) } }
                if item.kind == .file || item.kind == .overlay {
                    TextField("File", text: $item.target)
                    Button("Scegli file…") { if let url = FilePicker.files().first { item.target = url.path; loadPreview() } }
                    if item.kind == .overlay {
                        Picker("Dalla libreria", selection: $item.target) {
                            Text("Seleziona…").tag("")
                            ForEach(model.workspace.overlays, id: \.self) { Text(MediaFiles.displayName($0)).tag($0) }
                        }
                    }
                    if let player { VideoPlayer(player: player).frame(height: 150) }
                    else if let image = NSImage(contentsOfFile: item.target) { Image(nsImage: image).resizable().scaledToFit().frame(height: 150) }
                } else {
                    Picker("Dispositivo", selection: $item.target) {
                        Text("Seleziona…").tag("")
                        ForEach(model.devices.filter { $0.kind == item.kind }, id: \.self) { Text($0.name).tag($0.id) }
                    }
                    TextField("Indice AVFoundation", text: $item.target)
                    Button("Rileva dispositivi") { model.refreshDevices() }
                }
            }
            if item.kind != .microphone {
                Section("Anteprima sorgente") {
                    if let snapshot { Image(nsImage: snapshot).resizable().scaledToFit().frame(height: 150) }
                    Button("Acquisisci fotogramma") { captureSnapshot() }.disabled(previewBusy || item.target.isEmpty)
                    if previewBusy { ProgressView() }
                    if let previewError { Text(previewError).font(.caption).foregroundStyle(.red) }
                }
                Section("Riquadro (pixel)") {
                    TextField("X", value: $item.x, format: .number); TextField("Y", value: $item.y, format: .number)
                    TextField("Larghezza", value: $item.width, format: .number); TextField("Altezza", value: $item.height, format: .number)
                    Button("Riempi canvas") { item.x = 0; item.y = 0; item.width = canvasWidth; item.height = canvasHeight }
                }
            }
            Section("Audio") {
                Toggle("Includi nel mix", isOn: $item.audioEnabled).disabled(item.kind == .overlay || item.kind == .screen || item.kind == .camera)
                Slider(value: $item.volume, in: 0...200) { Text("Volume") }
                Text("\(Int(item.volume))%")
                Text("Per audio di sistema usa un dispositivo audio virtuale e aggiungilo come microfono. La cattura schermo AVFoundation fornisce il video.").font(.caption).foregroundStyle(.secondary)
            }
        }.formStyle(.grouped).onAppear { loadPreview() }.onChange(of: item.target) { loadPreview() }.onDisappear { player?.pause(); previewTask?.cancel() }
    }
    private func loadPreview() {
        previewTask?.cancel(); player?.pause(); player = nil; snapshot = nil; previewError = nil
        guard item.kind == .file, !item.target.isEmpty, !MediaFiles.isStill(item.target) else { return }
        let source = item.target
        model.prepareMedia(source)
        previewTask = Task {
            previewBusy = true; defer { previewBusy = false }
            do {
                let url = try await MediaPreviewService.shared.playable(URL(fileURLWithPath: source), ffmpeg: Executables.resolve("ffmpeg", configured: model.workspace.ffmpegPath), ffprobe: Executables.resolve("ffprobe", configured: model.workspace.ffprobePath))
                guard !Task.isCancelled, item.target == source else { return }
                player = AVPlayer(url: url)
            } catch { if !Task.isCancelled { previewError = error.localizedDescription } }
        }
    }
    private func captureSnapshot() {
        previewTask?.cancel(); let source = item
        previewTask = Task {
            previewBusy = true; defer { previewBusy = false }
            do {
                try await AppleCapture.authorize([source])
                let data = try await MediaPreviewService.shared.snapshot(source, ffmpeg: Executables.resolve("ffmpeg", configured: model.workspace.ffmpegPath), ffprobe: Executables.resolve("ffprobe", configured: model.workspace.ffprobePath))
                guard !Task.isCancelled, item.id == source.id, item.target == source.target else { return }
                snapshot = NSImage(data: data)
            } catch { if !Task.isCancelled { previewError = error.localizedDescription } }
        }
    }
}
@MainActor struct OverlaysView: View {
    @EnvironmentObject var model: AppModel
    @State private var preview: MediaSelection?
    var body: some View {
        ScrollView {
            LazyVGrid(columns: [GridItem(.adaptive(minimum: 230))], spacing: 20) {
                ForEach(model.workspace.overlays.filter { model.search.isEmpty || MediaFiles.displayName($0).localizedCaseInsensitiveContains(model.search) }, id: \.self) { path in
                    GroupBox {
                        VStack {
                            CanvasMediaPreview(path: path).frame(height: 150)
                            Text(MediaFiles.displayName(path)).lineLimit(2)
                            Button("Anteprima") { preview = MediaSelection(path: path) }
                            Button("Rimuovi dalla libreria", role: .destructive) { model.removeOverlay(path) }
                        }.padding(8)
                    }
                }
            }.padding(24)
        }.toolbar { Button("Importa overlay…") { model.importOverlay() } }
        .sheet(item: $preview) { MediaPreviewPanel(path: $0.path).environmentObject(model) }
    }
}
@MainActor struct DeckView: View {
    @EnvironmentObject var model: AppModel
    @State private var draft: DeckButton?
    @State private var preview: MediaSelection?
    var body: some View {
        List {
            ForEach(model.workspace.deck.filter { model.search.isEmpty || $0.label.localizedCaseInsensitiveContains(model.search) }) { button in
                HStack {
                    Label(button.label, systemImage: button.inScene ? "square.3.layers.3d" : "play.rectangle")
                    Text(button.hotkey).foregroundStyle(.secondary); Spacer()
                    Button("Anteprima") { preview = MediaSelection(path: button.mediaPath) }
                    Button("Duplica") { var copy = button; copy.id = UUID(); copy.hotkey = ""; copy.label += " copia"; draft = copy }
                    Button("Modifica") { draft = button }
                    Button("Elimina", role: .destructive) { model.removeDeckButton(button.id) }
                }
            }.onMove { model.workspace.deck.move(fromOffsets: $0, toOffset: $1); model.persist() }
        }.toolbar { Button("Nuovo pulsante") { draft = DeckButton() } }
        .sheet(item: $draft) { DeckEditor(button: $0).environmentObject(model) }
        .sheet(item: $preview) { MediaPreviewPanel(path: $0.path).environmentObject(model) }
    }
}
@MainActor struct DeckEditor: View {
    @EnvironmentObject var model: AppModel
    @Environment(\.dismiss) var dismiss
    @State var button: DeckButton
    @State private var localError: String?
    @State private var busy = false
    var body: some View {
        VStack {
            Text("Pulsante scena").font(.title2.bold()).padding(.top)
            Form {
                TextField("Nome", text: $button.label)
                HStack { TextField("Media", text: $button.mediaPath); Button("Importa…") {
                    guard let url = FilePicker.files().first else { return }
                    busy = true
                    let store = model.store
                    Task {
                        defer { busy = false }
                        do { button.mediaPath = try await Task.detached { try store.importMedia(url, collection: "Deck") }.value }
                        catch { localError = error.localizedDescription }
                    }
                } }
                if !button.mediaPath.isEmpty { CanvasMediaPreview(path: button.mediaPath).frame(height: 130) }
                HotkeyRecorder(value: $button.hotkey, buttonID: button.id)
                Text("Le scorciatoie funzionano con la regia selezionata e senza un campo di testo attivo. Cmd+Q/W/N/R/F/P e le scorciatoie di regia sono riservate.").font(.caption)
                Toggle("Sovrapponi alla scena", isOn: $button.inScene)
                Picker("Posizione", selection: $button.placement) {
                    ForEach(["bottom-right", "bottom-left", "top-right", "top-left", "center", "custom"], id: \.self) { Text($0).tag($0) }
                }
                TextField("Durata (s, 0 = fino alla ripresa)", value: $button.durationSeconds, format: .number)
                HStack { TextField("X", value: $button.x, format: .number); TextField("Y", value: $button.y, format: .number) }
                HStack { TextField("Larghezza", value: $button.width, format: .number); TextField("Altezza", value: $button.height, format: .number) }
                if let localError { Text(localError).foregroundStyle(.red) }
            }.formStyle(.grouped)
            HStack { Button("Annulla") { dismiss() }; Spacer(); if busy { ProgressView().controlSize(.small) }; Button("Salva") {
                busy = true
                Task {
                defer { busy = false }
                do {
                    guard !button.label.isEmpty, FileManager.default.fileExists(atPath: button.mediaPath), button.durationSeconds >= 0,
                          button.width >= 2, button.height >= 2, button.width % 2 == 0, button.height % 2 == 0, button.x >= 0, button.y >= 0 else { throw OrbisError.invalid("Controlla nome, media e dimensioni del pulsante.") }
                    if !button.hotkey.isEmpty {
                        guard let shortcut = DeckHotkey(button.hotkey), !model.workspace.deck.contains(where: { $0.id != button.id && DeckHotkey($0.hotkey) == shortcut }) else { throw OrbisError.invalid("Scorciatoia non valida, riservata o già in uso.") }
                        button.hotkey = shortcut.canonical
                    }
                    let extensionName = URL(fileURLWithPath: button.mediaPath).pathExtension.lowercased()
                    guard MediaFiles.videos.contains(extensionName) || MediaFiles.isPicture(button.mediaPath) else { throw OrbisError.invalid("Formato del media non supportato.") }
                    _ = try await Probe.media(button.mediaPath, executable: Executables.resolve("ffprobe", configured: model.workspace.ffprobePath))
                    var next = model.workspace
                    if let index = next.deck.firstIndex(where: { $0.id == button.id }) { next.deck[index] = button } else { next.deck.append(button) }
                    try model.store.save(next); model.workspace = next; dismiss()
                } catch { localError = error.localizedDescription }
                }
            }.buttonStyle(.borderedProminent).disabled(busy) }.padding()
        }.frame(width: 660, height: 780).interactiveDismissDisabled(busy)
    }
}
