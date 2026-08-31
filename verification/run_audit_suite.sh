#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
mkdir -p verification/runs
make verify | tee verification/runs/make_verify.log
make sanitizer | tee verification/runs/make_sanitizer.log
./bin/symmetric_network_matrix matrix 4 4 1000 12000 4000 1.0 500 0.06 | tee verification/runs/base4.log
./bin/symmetric_network_matrix matrix 4 4 1000 12000 4000 1.0 250 0.12 | tee verification/runs/clock250_4.log
./bin/symmetric_network_matrix matrix 4 4 1000 12000 4000 1.0 500 0.03 | tee verification/runs/window15_4.log
./bin/symmetric_network_matrix matrix 4 4 1000 12000 4000 1.0 500 0.12 | tee verification/runs/window60_4.log
./bin/symmetric_network_matrix matrix 4 4 1000 12000 4000 1.0 1000 0.03 | tee verification/runs/clock1000_4.log
python3 verification/reconstruct_claims.py | tee verification/runs/reconstruct_claims.log
