#!/usr/bin/env python3
"""Generate a file-by-file Windows→Linux port map from the frozen Windows inventory."""

from __future__ import annotations

import hashlib
import json
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INVENTORY = ROOT / "docs/windows-inventory.json"
OUTPUT = ROOT / "docs/PORTING_MAP.md"

# These sources have no equivalent behavior in the current C++ port.
MISSING = {
    "src/Orbis.Stream.Core/Logging/FileLoggerProvider.cs": "Logger applicativo e rotazione mancanti",
    "src/Orbis.Stream.Core/Services/YouTubeAirWatch.cs": "Monitoraggio automatico YouTube mancante",
    "src/Orbis.Stream.Core/Services/YouTubeAirState.cs": "Stato automatico YouTube mancante",
    "src/Orbis.Stream.Core/Streaming/Rtmp/Amf0.cs": "Protocollo RTMP nativo mancante",
    "src/Orbis.Stream.Core/Streaming/Rtmp/RtmpPublisher.cs": "Publisher RTMP nativo mancante",
    "src/Orbis.Stream.Core/Services/LiveChangeNotifier.cs": "Notifiche push live mancanti",
    "src/Orbis.Stream.Core/Domain/FfmpegCodecCatalog.cs": "Catalogo codec completo mancante",
}

SERVICE_TARGETS = {
    "EncoderTuningService": "src/media_job.cpp",
    "MediaProxyService": "src/media_job.cpp",
    "ImageService": "src/media_job.cpp; src/server.cpp",
    "SourceSnapshotService": "src/server.cpp; src/core.cpp",
    "SourceCatalogService": "src/core.cpp",
    "SystemInfoService": "src/core.cpp; src/metrics.cpp",
    "StreamingService": "src/stream.cpp",
    "FfmpegVideoPlaylistStreamer": "src/stream.cpp",
    "LivePreviewService": "src/stream.cpp; src/server.cpp",
    "LiveTakeovers": "src/stream.cpp",
    "SceneService": "src/scene_service.cpp; src/scene_service.hpp; src/server.cpp",
    "OverlayLibrary": "src/server.cpp; src/core.cpp",
    "MediaStore": "src/server.cpp",
    "NaturalFileNameComparer": "src/core.cpp",
    "LiveHistoryCleanupService": "src/history_cleanup_service.cpp; src/history_cleanup_service.hpp; src/core.cpp; src/stream.cpp",
    "BackgroundTaskExecutor": "src/stream.cpp; src/media_job.cpp",
}

STREAM_TARGETS = {
    "FfmpegCommandBuilder": "src/stream.cpp",
    "FfmpegProbe": "src/core.cpp",
    "FlvPacedRelay": "src/stream.cpp",
    "FlvSink": "src/stream.cpp",
    "LivePreviewFrames": "src/stream.cpp",
    "LivePlatformEmbeds": "src/media_job.cpp",
    "StreamPlatform": "src/stream_platform.cpp; src/stream.cpp; src/media_job.cpp",
    "LiveStreamKeys": "src/live_stream_keys.cpp; src/stream.cpp",
    "MonitorTarget": "src/stream.cpp",
    "OverlayMedia": "src/stream.cpp",
    "BitrateLadder": "src/stream.cpp",
    "LiveOutput": "src/stream.cpp",
    "HybridWaiter": "src/stream.cpp",
    "FfmpegStreamingSession": "src/stream.cpp",
}

TEST_TARGETS = {
    "DatabaseTests": "tests/core_tests.cpp",
    "PlaylistTests": "tests/integration.py",
    "PacedRelayTests": "tests/core_tests.cpp",
    "EndToEndApiTests": "tests/integration.py",
    "FfmpegCommandBuilderTests": "tests/integration.py",
    "FfmpegStreamingTests": "tests/integration.py",
    "MediaProxyTests": "tests/integration.py",
    "StyleSheetTests": "tests/browser.mjs",
    "SceneDeckTests": "tests/browser.mjs",
    "LivePreviewTests": "tests/integration.py",
    "EncoderQualityTests": "tests/integration.py",
}

HTML_TARGETS = {
    "MainMenu": "web/pages/MainMenu.html; web/app.js",
    "MainLiveMenu": "web/pages/MainLiveMenu.html; web/app.js",
    "MainSetting": "web/pages/MainSetting.html; web/app.js",
    "MainLayoutList": "web/pages/MainLayoutList.html; web/app.js",
    "MainPreview": "web/pages/MainPreview.html; web/app.js",
    "MainChannelSetting": "web/pages/MainChannelSetting.html; web/app.js",
    "MainVideoSetting": "web/pages/MainVideoSetting.html; web/app.js",
    "MainLiveHistory": "web/pages/MainLiveHistory.html; web/app.js",
    "MainLive": "web/pages/MainLive.html; web/app.js; src/live_rows.cpp",
    "_LiveRows": "web/pages/_LiveRows.html; web/app.js; src/live_rows.cpp",
    "_StartWizardSetting": "web/pages/_StartWizardSetting.html; web/app.js",
    "_StartWizardConfiguration": "web/pages/_StartWizardConfiguration.html; web/app.js",
}


def copied_assets() -> dict[str, str]:
    """Index hashes to distinguish byte-identical resources from merely similar files."""
    copied = {}
    for path in (ROOT / "web/resources").rglob("*"):
        if path.is_file():
            copied[hashlib.sha256(path.read_bytes()).hexdigest()] = path.relative_to(ROOT).as_posix()
    return copied


def counterpart(source: dict, copied: dict[str, str]) -> tuple[str, str, str]:
    """Return a conservative target and status; a related file never counts as verified parity."""
    path = source["path"]
    name = Path(path).stem
    if source["sha256"] in copied:
        return copied[source["sha256"]], "COPIATO", "Byte identici all'inventario"
    if path in MISSING:
        return "—", "MANCANTE", MISSING[path]
    if path.endswith(".css"):
        return "web/style.css", "PARZIALE", "Riferimento CSS conservato; la GUI corrente usa stile e widget Qt"
    if path.endswith(".js"):
        return "src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; web/app.js", "PARZIALE", "Interazioni trasferite in Qt; JavaScript conservato come riferimento"
    if path.endswith(".cshtml"):
        if name in HTML_TARGETS:
            return "src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/native_media.cpp; " + HTML_TARGETS[name], "PARZIALE", "Flusso trasferito in widget Qt; HTML conservato come riferimento"
        target = "web/index.html; web/app.js" if "/Shared/_Layout" in path else "web/app.js"
        return target, "PARZIALE", "Pagina Razor da riprodurre nel client Linux"
    if "/Pages/" in path and path.endswith(".cs"):
        if name == "MainLive.cshtml":
            return "src/live_rows.cpp; src/server.cpp; web/app.js", "PARZIALE", "Righe/snapshot e wizard portati; locking video e notifiche push Windows ancora diversi"
        if name == "MainLiveHistory.cshtml":
            return "src/history_cleanup_service.cpp; src/server.cpp; web/app.js", "PARZIALE", "Filtri e pulizia portati; modello dati e scene temporanee ancora diversi"
        return "src/native_window.cpp; src/native_dialogs.cpp; src/native_composer.cpp; src/server.cpp", "PARZIALE", "Flusso trasferito in Qt; validazioni da confrontare"
    if "/Services/" in path and path.endswith(".cs"):
        if name == "SceneService":
            return SERVICE_TARGETS[name], "PARZIALE", "CheckSlots/CheckLayout e protezione storico portati; contratto e repository ancora diversi"
        return SERVICE_TARGETS.get(name, "src/core.cpp; src/stream.cpp"), "PARZIALE", "Servizio da verificare metodo per metodo"
    if "/Streaming/" in path and path.endswith(".cs"):
        return STREAM_TARGETS.get(name, "src/stream.cpp"), "PARZIALE", "Semantica streaming da verificare metodo per metodo"
    if "/Data/" in path and path.endswith(".cs"):
        if name == "DatabaseSchema":
            return "src/database_schema.cpp; src/core.cpp", "PARZIALE", "Schema e migrazioni Linux separati"
        return "src/core.cpp", "PARZIALE", "Schema SQLite Linux diverso da quello Windows"
    if "/Domain/" in path and path.endswith(".cs"):
        return "src/core.hpp; src/core.cpp", "PARZIALE", "Modello JSON Linux da confrontare proprietà per proprietà"
    if "/Hosting/" in path and path.endswith(".cs"):
        return "src/server.cpp; src/main.cpp", "PARZIALE", "Route e vita del servizio da confrontare"
    if "/Http/" in path and path.endswith(".cs"):
        return "src/server.cpp", "PARZIALE", "Errori e formato risposte da confrontare"
    if "/I18n/" in path and path.endswith(".cs"):
        return "src/native_i18n.cpp; src/native_i18n.hpp; web/resources/ui; tests/native_tests.cpp", "PARZIALE", "21 cataloghi riusati nella GUI Qt; revisione linguistica non certificata"
    if "/SystemInfo/" in path and path.endswith(".cs"):
        return "src/metrics.cpp; src/core.cpp", "PARZIALE", "Metriche con contatori specifici di Linux"
    if "/Configuration/" in path and path.endswith(".cs"):
        return "src/main.cpp", "PARZIALE", "Opzioni runtime da confrontare"
    if "/Contracts/" in path and path.endswith(".cs"):
        return "src/server.cpp; src/core.cpp", "PARZIALE", "Contratto HTTP Linux ancora diverso"
    if "/Orbis.Stream.App/" in path and path.endswith(".cs"):
        return "src/main.cpp; src/native_window.cpp", "PARZIALE", "Shell Qt nativa predefinita; API solo in headless"
    if "/Orbis.Stream.Tests/" in path and path.endswith(".cs"):
        target = TEST_TARGETS.get(name)
        return (target, "PARZIALE", "Casi Windows da trasferire") if target else (
            "—", "MANCANTE", "Test equivalente non presente"
        )
    if path.endswith(".csproj") or path == "Directory.Build.props":
        return "CMakeLists.txt; VERSION", "PARZIALE", "Build e versione Linux indipendenti"
    if path.startswith("Installer/"):
        return "CMakeLists.txt; scripts/create-deb.sh", "PARZIALE", "Installer Debian da confrontare con quello Windows"
    if path == "README.md":
        return "README.md", "PARZIALE", "Manuale Linux specifico"
    return "—", "RIFERIMENTO", "File Windows senza traduzione di codice diretta"


def main() -> None:
    """Write one auditable row per Windows source, preserving the inventory hash for later diffs."""
    inventory = json.loads(INVENTORY.read_text())
    copied = copied_assets()
    rows = []
    counts = Counter()
    for source in inventory["files"]:
        target, status, note = counterpart(source, copied)
        counts[status] += 1
        rows.append(
            f"| `{source['path']}` | `{source['sha256'][:12]}` | "
            f"{target} | {status} | {note} |"
        )
    lines = [
        "# Mappa file per file: Windows → Linux",
        "",
        "Riferimento: `windows-inventory.json`, checkout Windows 2.1.2 del 12 ottobre 2026. "
        "Generare di nuovo con `python3 scripts/porting_map.py` dopo aver aggiornato l'inventario.",
        "",
        "`COPIATO` significa hash identico; `PARZIALE` significa solo che esiste codice Linux "
        "correlato, non che il comportamento sia equivalente; `MANCANTE` richiede implementazione; "
        "`RIFERIMENTO` indica documenti o file specifici di Windows. Nessun file di codice è "
        "segnato come verificato sulla sola base del nome.",
        "",
        f"Totale {len(rows)} file: " + ", ".join(f"{key} {counts[key]}" for key in sorted(counts)) + ".",
        "",
        "| File Windows | SHA-256 (12) | File Linux | Stato | Lavoro residuo |",
        "|---|---|---|---|---|",
        *rows,
        "",
    ]
    document = "\n".join(lines)
    if "--check" in sys.argv:
        if not OUTPUT.exists() or OUTPUT.read_text() != document:
            raise SystemExit("PORTING_MAP.md non aggiornato: eseguire python3 scripts/porting_map.py")
    else:
        OUTPUT.write_text(document)
    print(f"{OUTPUT}: {len(rows)} file")


if __name__ == "__main__":
    main()
