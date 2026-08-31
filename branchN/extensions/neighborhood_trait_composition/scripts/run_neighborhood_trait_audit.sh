#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../../../.."
exec python3 branchN/extensions/neighborhood_trait_composition/scripts/run_neighborhood_trait_audit.py "$@"
