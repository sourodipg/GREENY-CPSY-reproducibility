#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PKG="$(cd "$ROOT/.." && pwd)"
cd "$PKG"

echo '===== BRANCHN: FROZEN SOFTWARE PREFLIGHT ====='
python3 tests/test_network_structure.py
python3 tests/test_suite.py

echo '===== BRANCHN: COORDINATE-REALIZATION DISTRIBUTION ====='
python3 branchN/coordinate_realization_distribution/scripts/run_branchN.py "$@"

echo '===== BRANCHN: DOWNSTREAM FROZEN-CODE AUDITS ====='
python3 verification/directed_recipient_order_audit.py
python3 reproduce/verify_preflight.py
python3 reproduce/package_self_check.py

echo '===== BRANCHN: RESULT ====='
cat branchN/coordinate_realization_distribution/results/branchN_report.md
