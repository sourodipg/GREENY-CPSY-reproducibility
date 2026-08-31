#!/usr/bin/env bash
set -euo pipefail

# Repository-level verification. Heavy production runs are not launched here.
# This script is intentionally auditable: each stage prints what it checks.

printf '%s\n' '=== GREENY REPOSITORY VERIFICATION ==='
printf '%s\n' '1. Build and run the deterministic structural checks.'
make verify

printf '%s\n' '2. Confirm the released interaction-disambiguation data exist.'
test -s data/interaction_disambiguation/interaction_disambiguation_per_seed.csv
test -s data/interaction_disambiguation/interaction_disambiguation_contrasts.csv

printf '%s\n' '3. Confirm BranchN released summaries exist.'
test -s data/branchN/branchN_by_offset.csv
test -s data/branchN/branchN_by_seed.csv

test -s data/network/network_metrics_by_offset.csv
test -s data/network/network_metrics_by_seed.csv

printf '%s\n' '4. Confirm citation metadata and licenses exist.'
test -s CITATION.cff
test -s LICENSE-CODE
test -s LICENSE-DOCS-DATA

printf '%s\n' 'REPOSITORY VERIFICATION PASS'
