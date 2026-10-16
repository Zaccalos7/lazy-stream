#!/usr/bin/env python3
"""Check the supported Windows scope and prevent removed features returning to Linux."""
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = ROOT / "tests/windows-defaults.json"
sys.path.insert(0, str(ROOT / "scripts"))
from linux_resources import REMOVED, linux_bundle


def strings(block):
    return "".join(json.loads(value) for value in re.findall(r'"(?:[^"\\]|\\.)*"', block))


def windows_defaults(source):
    """Read the eight in-scope seed values directly from Windows DefaultSpec declarations."""
    text = (source / "Hosting/DatabaseBootstrapper.cs").read_text()
    pattern = r'\(\s*([\d_]+),\s*([\d_]+),\s*(null|[\d_]+),\s*(null|[\d_]+),\s*(null|[\d_.]+),\s*\[(.*?)\]'
    result = []
    for platform, symbol in (("Twitch", "Twitch"), ("YouTube", "YouTube"), ("Kick", "Kick"), ("Facebook", "FacebookGaming")):
        for high in (True, False):
            marker = ('("Twitch", ' + str(high).lower() + ') =>') if platform == "Twitch" else (
                "DefaultSpec " + symbol + ("High" if high else "Low") + " =")
            match = re.search(pattern, text[text.index(marker):], re.S)
            assert match, marker
            video, audio, width, height, fps, options = match.groups()
            options = dict(re.findall(r'\("([^"]+)",\s*"([^"]+)"\)', options))
            result.append({"platform": platform, "isDefaultConfiguration": high,
                "videoBitrate": int(video.replace('_', '')) // 1000,
                "audioBitrate": int(audio.replace('_', '')) // 1000,
                "videoWidth": None if width == "null" else int(width),
                "videoHeight": None if height == "null" else int(height),
                "frameRate": None if fps == "null" else float(fps),
                "preset": options["preset"], "profile": options["profile"],
                "x264-params": options.get("x264-params")})
    return result


def main():
    resources = ROOT / "web/resources"
    languages = sorted(path.name for path in (resources / "ui").glob("ui.*.json"))
    assert len(languages) == 21, languages
    for path in (ROOT / "src").glob("*"):
        if path.suffix in (".cpp", ".hpp") and path.name != "database_schema.cpp":
            assert not REMOVED.search(path.read_text()), f"Removed runtime feature: {path}"
    for base in (resources / "ui", resources / "messages", ROOT / "web/pages"):
        for path in base.rglob("*"):
            if path.is_file():
                assert not REMOVED.search(path.read_text()), f"Removed product text: {path}"
    assert not REMOVED.search((ROOT / "web/app.js").read_text()), "Removed headless client feature"
    assert not (ROOT / "src/live_stream_keys.cpp").exists(), "Temporary-key service must be removed"

    source = ROOT.parent.parent / "lazy-stream-windows/src/Orbis.Stream.Core"
    if not source.is_dir():
        print("Supported scope and 21 Linux catalogs: OK; live Windows comparison unavailable")
        return
    expected_defaults = windows_defaults(source)
    if "--refresh-defaults" in sys.argv:
        FIXTURE.write_text(json.dumps(expected_defaults, indent=2) + "\n")
    assert json.loads(FIXTURE.read_text()) == expected_defaults, "Windows defaults fixture out of date"
    for path in (source / "Ui").glob("*.json"):
        assert json.loads((resources / "ui" / path.name).read_text()) == linux_bundle(json.loads(path.read_text())), path
    for path in (source / "Messages").glob("*.properties"):
        expected = "".join(line for line in path.read_text().splitlines(keepends=True) if not REMOVED.search(line))
        assert (resources / "messages" / path.name).read_text() == expected, path

    windows_choices = (source / "Pages/MainChannelSetting.cshtml.cs").read_text()
    linux_choices = (ROOT / "src/native_dialogs.cpp").read_text()
    facebook = re.search(r'FacebookIngestUrl\s*=\s*"([^"]+)"', (source / "Streaming/StreamPlatform.cs").read_text())[1]
    choices = re.findall(r'new\("(twitch|youtube|kick|facebook)",\s*"[^"]+",\s*("[^"]+"|StreamPlatforms.FacebookIngestUrl)', windows_choices)
    assert len(choices) == 4, choices
    for name, value in choices:
        expected = facebook if value.startswith("StreamPlatforms.") else json.loads(value)
        label = {"youtube": "YouTube"}.get(name, name.title())
        match = re.search(r'\{"' + label + r'",\s*"([^"]+)"\}', linux_choices)
        assert match and match[1] == expected, f"{name} ingest differs from Windows"
    windows_marks = (source / "Pages/PageHelpers.cs").read_text()
    linux_marks = (ROOT / "src/native_theme.cpp").read_text()
    for win, cpp in (("Twitch", "twitch"), ("Youtube", "youtube"), ("Kick", "kick"), ("FacebookGaming", "facebook")):
        left = re.search(r'const string ' + win + r'Mark\s*=([^;]+);', windows_marks)[1]
        right = re.search(r'const char\* const ' + cpp + r'Mark\s*=([^;]+);', linux_marks)[1]
        assert re.sub(r'\s+', '', strings(left)) == re.sub(r'\s+', '', strings(right)), f"{cpp} mark differs"
    for language in languages:
        code = language.split('.')[1]
        for figure in ("wizard", "playlist"):
            path = resources / "manual" / code / (figure + ".webp")
            assert path.stat().st_size > 1000, f"Missing Qt manual capture: {path}"
            assert path.read_bytes() != (source.parent / "Orbis.Stream.App/wwwroot/manual" / code / path.name).read_bytes(), f"Old five-platform figure: {path}"
    print("21 UI/manual/message catalogs, four Windows ingests, exact SVG marks, removed runtime paths and 42 Qt figures: OK")


if __name__ == "__main__":
    main()
