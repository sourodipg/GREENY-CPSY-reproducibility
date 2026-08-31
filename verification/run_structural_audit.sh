#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
mkdir -p verification/runs
python3 tests/test_network_structure.py | tee verification/runs/network_structure_audit.log
python3 verification/coordinate_realization_audit.py "$@" | tee verification/runs/coordinate_realization_audit.log
