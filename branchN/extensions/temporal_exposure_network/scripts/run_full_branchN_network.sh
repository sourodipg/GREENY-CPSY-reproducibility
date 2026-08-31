#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
cd "$ROOT"

echo '=== FROZEN GREENY / BRANCHN ==='
python3 branchN/coordinate_realization_distribution/scripts/run_branchN.py \
  --seeds 12 --threads 1 --workers 24 --families 1000 --steps 12000 --n 4000 \
  --offsets 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23

echo '=== LITERATURE-GROUNDED TEMPORAL / EXPOSURE NETWORK AUDIT ==='
python3 branchN/extensions/temporal_exposure_network/scripts/run_network_audit.py \
  --seeds 101 202 303 404 505 606 707 808 909 1010 1111 1212 \
  --workers 24 --families 1000 --steps 12000 --n 4000 \
  --offsets 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23

echo '=== GRAPH KERNEL TEST ==='
python3 branchN/extensions/temporal_exposure_network/tests/test_graph_kernel.py

echo '=== FROZEN PREFLIGHT ==='
python3 reproduce/verify_preflight.py

echo '=== PACKAGE SELF-CHECK ==='
python3 reproduce/package_self_check.py
