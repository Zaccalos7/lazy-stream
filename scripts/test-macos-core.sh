#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
swift_bin="${SWIFT_BIN:-swift}"
"$swift_bin" test "$@"
