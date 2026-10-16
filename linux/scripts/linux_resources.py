"""Apply intentional Linux resource differences after copying the Windows bundles."""
import json
import re
from pathlib import Path

REMOVED = re.compile(r"tik\s*tok", re.IGNORECASE)
REMOVED_KEYS = {"keyEachLiveHint"}


def linux_bundle(value):
    if isinstance(value, dict):
        return {key: linux_bundle(item) for key, item in value.items()
                if not REMOVED.search(key) and key not in REMOVED_KEYS}
    if isinstance(value, list):
        return [linux_bundle(item) for item in value]
    if isinstance(value, str):
        if not REMOVED.search(value):
            return value
        # The manual's channel step adds a provider-specific parenthesis in every language.
        return re.sub(r"[（(][^()（）]*tik\s*tok[^()（）]*[)）]", "", value,
                      flags=re.IGNORECASE).replace("  ", " ").replace(" .", ".").strip()
    return value


def sanitize_resources(resources: Path):
    for path in (resources / "ui").glob("*.json"):
        original = json.loads(path.read_text())
        filtered = linux_bundle(original)
        if original != filtered:
            path.write_text(json.dumps(filtered, ensure_ascii=False, indent=2) + "\n")
    for path in (resources / "messages").glob("*.properties"):
        original = path.read_text()
        filtered = "".join(line for line in original.splitlines(keepends=True)
                           if not REMOVED.search(line))
        if original != filtered:
            path.write_text(filtered)


if __name__ == "__main__":
    import sys
    sanitize_resources(Path(sys.argv[1]))
