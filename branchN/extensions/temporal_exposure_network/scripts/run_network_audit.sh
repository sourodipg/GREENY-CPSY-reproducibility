#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../../../.."
exec python3 branchN/extensions/temporal_exposure_network/scripts/run_network_audit.py "$@"
