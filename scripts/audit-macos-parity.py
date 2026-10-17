#!/usr/bin/env python3
"""Compare every Windows endpoint and Razor handler with its native Mac implementation.

This verifies the inventory and code references. Runtime results are recorded separately;
source references are never counted as successful macOS executions.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--windows-source', type=Path, default=ROOT.parent / 'lazy-stream-windows/src/Orbis.Stream.Core')
parser.add_argument('--check', action='store_true', help='Check the committed inventory without rewriting it')
args = parser.parse_args()

# A reference is a real source symbol, not merely a file that exists.
FILES = {
    'app': 'Sources/OrbisMac/AppModel.swift', 'nav': 'Sources/OrbisMac/NavigationViews.swift',
    'config': 'Sources/OrbisMac/ConfigurationViews.swift', 'scene': 'Sources/OrbisMac/SceneViews.swift',
    'live': 'Sources/OrbisMac/LiveViews.swift', 'session': 'Sources/OrbisMac/LiveSession.swift',
    'media': 'Sources/OrbisMac/MediaPreviewService.swift', 'panel': 'Sources/OrbisMac/MediaPreviewPanel.swift',
    'apple': 'Sources/OrbisMac/AppleServices.swift', 'proxy': 'Sources/OrbisMac/MediaOptimization.swift',
    'stats': 'Sources/OrbisCore/SystemMetrics.swift', 'root': 'Sources/OrbisMac/OrbisApp.swift',
    'updates': 'Sources/OrbisCore/MacUpdates.swift', 'cleanup': 'Sources/OrbisMac/HistoryCleanupSettings.swift',
    'report': 'Sources/OrbisMac/ErrorReportView.swift', 'links': 'Sources/OrbisCore/PlatformLinks.swift',
    'wizard': 'Sources/OrbisMac/StartWizard.swift', 'replay': 'Sources/OrbisCore/PlatformPresets.swift',
    'store': 'Sources/OrbisCore/FLVRelay.swift', 'preview': 'Sources/OrbisMac/CanvasMediaPreview.swift',
}

def refs(*values):
    result = []
    for value in values:
        file, symbol = value.split(':', 1)
        path = FILES[file]
        assert symbol in (ROOT / path).read_text(), (path, symbol)
        result.append({'file': path, 'symbol': symbol})
    return result

ROUTES = {}
def cover(method, path, *symbols, difference=None):
    ROUTES[(method, path)] = {'swift': refs(*symbols), 'status': 'platform-difference' if difference else 'native-implementation',
                              'macOS_execution': 'pending', **({'difference': difference} if difference else {})}

cover('GET','/scene/layouts','scene:ScenesView')
cover('GET','/scene/{pkid:long}','scene:SceneEditor')
cover('POST','/scene/save','scene:private func save()')
cover('DELETE','/scene/{pkid:long}','app:func removeScene(')
cover('GET','/preview/sources','app:func refreshDevices(','apple:static func devices(')
cover('GET','/preview/sources/snapshot','media:func snapshot(')
cover('POST','/preview/sources/prepare','proxy:func prepare(')
cover('GET','/preview/sources/probe','scene:private func sourceResolution(')
cover('GET','/scene/overlays','scene:OverlaysView')
cover('POST','/scene/overlays','app:func importOverlay(')
cover('GET','/scene/overlays/{name}','preview:CanvasMediaPreview')
cover('GET','/scene/overlays/{name}/still','media:func snapshot(')
cover('DELETE','/scene/overlays/{name}','app:func removeOverlay(')
cover('POST','/live/start-scene-live','wizard:private func transmit(','app:func start(')
cover('POST','/live/start-live','wizard:private func chooseFolder(','app:func start(')
cover('POST','/live/start-video-live','app:func replayFrom(','replay:static func starting(')
cover('PUT','/live/stop-live','session:func stop()')
cover('GET','/live/{pkid:int}/scene','session:var takeover:','nav:LiveDeckPanel')
cover('POST','/live/{pkid:int}/scene/resume','session:func resume()')
cover('POST','/live/{pkid:int}/scene/{button:long}','app:func playDeck(','session:func play(')
cover('GET','/scene-buttons','scene:DeckView')
cover('POST','/scene-buttons','scene:DeckEditor')
cover('PUT','/scene-buttons/{pkid:long}','scene:DeckEditor')
cover('DELETE','/scene-buttons/{pkid:long}','app:func removeDeckButton(')
cover('POST','/scene-buttons/media','store:func importMedia(')
cover('GET','/scene-buttons/media/{name}/still','preview:CanvasMediaPreview','media:func snapshot(')
cover('GET','/video/{pkid:int}/error-report','report:private func buildReport()')
cover('GET','/video/getPage','nav:LiveManagementView')
cover('GET','/video/getAllVideo','config:PlaylistView','config:HistoryView')
cover('GET','/video/getAllChannelWithVideoLive','live:LiveView')
cover('PUT','/video/unlockVideo','app:func unlock(')
cover('DELETE','/video/live-history/{pkid:long}','app:func removeHistory(')
cover('DELETE','/video/live-history/older-than/{months:int}','app:func cleanup(')
cover('POST','/video/live-history/cleanup','app:func cleanup(')
cover('GET','/video/live-history/cleanup','config:model.cleanupProgress')
cover('POST','/settings/save','config:ChannelEditor')
cover('PUT','/settings/change','config:ChannelEditor')
cover('GET','/settings/retrive','config:ChannelsView')
cover('GET','/settings/retrive-channels','config:PlaylistView')
cover('GET','/settings/retrive-directories-path','apple:static func files(')
cover('DELETE','/settings/delete','app:func removeChannel(')
cover('GET','/video-setting/getVideoSettings','config:ProfilesView')
cover('POST','/video-setting/save','config:ProfileEditor')
cover('POST','/video-setting/link','app:func assignProfile(','app:func linkVideoProfile(')
cover('PUT','/video-setting/edit','config:ProfileEditor')
cover('DELETE','/video-setting/delete','app:func removeProfile(')
cover('POST','/image/upload','root:homeBackground','store:func importMedia(')
cover('GET','/image/loadimage','root:DashboardView')
cover('GET','/taskManager/statistics/allInfo','stats:func sample(','root:TasksView')
cover('GET','/taskManager/statistics/cpu','stats:cpuPercent')
cover('GET','/taskManager/statistics/ram','stats:memoryUsed')
cover('GET','/taskManager/statistics/swap','stats:swapUsed')
for sensor in ('cpu','gpu'):
    cover('GET',f'/taskManager/statistics/{sensor}/temperature','root:Temperatura CPU/GPU e carico GPU','stats:thermalState',difference='No numerical temperature is claimed. macOS exposes thermal state; the UI marks numerical CPU/GPU temperatures unavailable.')
cover('GET','/preview/live','live:LiveView')
cover('GET','/preview/live/embed','live:PlatformPlayer','links:static func publicURL(')
cover('POST','/preview/live/{pkid:int}/air/dismiss','live:airWarningDismissed')
cover('GET','/preview/live/{pkid:int}/video','live:LiveControlView','session:private func updatePreview()',difference='The native composition preview uses JPEG frames, with a separate WebKit platform player; it does not expose the Windows HTTP media endpoint.')
cover('GET','/preview/live/{pkid:int}/frame','session:private func updatePreview()')
cover('PUT','/preview/live/{pkid:int}/parameters','session:func apply(')
cover('PUT','/preview/live/{pkid:int}/volume','session:func setVolume(')
cover('GET','/updates','updates:static func check(')

HANDLERS = {
    'MainChannelSetting.cshtml.cs': {'OnGet':['config:ChannelsView'], 'OnPostSave':['config:ChannelEditor'], 'OnPostToggle':['app:func setChannelEnabled('], 'OnPostDelete':['app:func removeChannel(']},
    'MainLayout.cshtml.cs': {'OnGet':['scene:SceneEditor']},
    'MainLayoutList.cshtml.cs': {'OnGet':['scene:ScenesView'], 'OnPostDelete':['app:func removeScene(']},
    'MainLive.cshtml.cs': {
        'OnGet':['nav:LiveManagementView'], 'OnGetRows':['nav:LiveManagementRow'], 'OnGetDetailsRows':['nav:LiveDetailsView'],
        'OnPostDeletePlaylist':['app:func removeHistory('], 'OnPostStop':['session:func stop()'], 'OnPostSpot':['session:func enqueueSpot('],
        'OnPostReplay':['app:func replay('], 'OnPostRestart':['replay:static func playlist('], 'OnPostDelete':['app:func deleteHistoryVideo('],
        'OnPostReplayFrom':['app:func replayFrom('], 'OnPostUnlock':['app:func unlock('], 'OnPostLink':['app:func assignProfile('],
        'OnPostStartScene':['wizard:private func transmit('], 'OnPostPlaylist':['wizard:private func chooseFolder(']},
    'MainLiveHistory.cshtml.cs': {'OnGet':['config:HistoryView'], 'OnPostSaveAutoCleanup':['cleanup:HistoryCleanupSettings'], 'OnPostDelete':['app:func removeHistory(']},
    'MainLiveMenu.cshtml.cs': {'OnGet':['nav:LiveHubView']},
    'MainPreview.cshtml.cs': {'OnGet':['live:LiveView'], 'OnPostStop':['session:func stop()'], 'OnPostSpot':['session:func enqueueSpot(']},
    'MainTaskManager.cshtml.cs': {'OnGet':['root:TasksView'], 'OnGetFacts':['stats:func sample(']},
    'MainVideoSetting.cshtml.cs': {'OnGet':['config:ProfilesView'], 'OnPostSave':['config:ProfileEditor'], 'OnPostToggle':['app:func setProfileActive('], 'OnPostDuplicate':['config:copy.id = UUID()'], 'OnPostDelete':['app:func removeProfile(']},
}

source = args.windows_source
endpoint_file = source / 'Hosting/OrbisEndpoints.cs'
text = endpoint_file.read_text()
group = ''; actual = []
for number, line in enumerate(text.splitlines(), 1):
    match = re.search(r'var group = app.MapGroup\("([^"]+)"\)', line)
    if match: group = match[1]
    match = re.search(r'(app|group)\.Map(Get|Post|Put|Delete)\("([^"]*)"', line)
    if match:
        key = (match[2].upper(), (group if match[1] == 'group' else '') + match[3])
        assert key in ROUTES, f'Unmapped Windows endpoint: {key}'
        actual.append({'method': key[0], 'route': key[1], 'windows_line': number, **ROUTES[key]})
assert len(actual) == len(ROUTES), 'Stale endpoint mapping'
handlers = []
for file in sorted((source / 'Pages').glob('*.cs')):
    for name in re.findall(r'public\s+(?:async\s+)?[\w<>?,]+\s+(On(?:Get|Post)\w*)\(', file.read_text()):
        assert name in HANDLERS.get(file.name, {}), f'Unmapped Windows handler: {file.name} {name}'
        handlers.append({'windows_file': 'Pages/' + file.name, 'handler': name, 'swift': refs(*HANDLERS[file.name][name]), 'macOS_execution':'pending'})
assert len(handlers) == sum(len(values) for values in HANDLERS.values()), 'Stale page-handler mapping'
result = {'reference':'lazy-stream-windows/src/Orbis.Stream.Core',
          'scope':'User operations represented by native Swift services and SwiftUI views; the Windows HTTP server is not reproduced.',
          'validation':'Inventory and symbol checks only. Core/FFmpeg runtime tests are separate from native macOS validation.',
          'endpoint_source_sha256':hashlib.sha256(endpoint_file.read_bytes()).hexdigest(),
          'endpoints':actual, 'page_handlers':handlers}
output = ROOT / 'docs/windows-parity-audit.json'
if args.check: assert json.loads(output.read_text()) == result, 'Audit out of date; rerun without --check'
else: output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n')
print(f'{len(actual)} endpoints and {len(handlers)} Razor handlers: all mapped; {sum(e["status"]=="platform-difference" for e in actual)} explicit native differences. macOS execution remains pending.')
