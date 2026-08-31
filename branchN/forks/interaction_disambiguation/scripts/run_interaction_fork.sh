#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
FORK="$ROOT/branchN/forks/interaction_disambiguation"
cd "$FORK"

SEEDS="${SEEDS:-56}"
THREADS="${THREADS:-32}"
FAMILIES="${FAMILIES:-1000}"
STEPS="${STEPS:-12000}"
N="${N:-4000}"
MIX="${MIX:-1.0}"
OFFSET="${OFFSET:-0}"

printf '%s\n' '=== INTERACTION-DISAMBIGUATION FORK ==='
printf 'Question 1: does swapping the one-sided recipient change the result?\n'
printf 'Question 2: does removing the partner signal from two-sided updating change the result?\n'
printf 'Question 3: does halving alpha make two-sided updating resemble one-sided updating?\n'
printf 'Design: same seed, population, forcing and encounter ledger within each contact regime.\n'
printf 'Run: seeds=%s threads=%s families=%s steps=%s N=%s metric_strength=%s coordinate_offset=%s\n' "$SEEDS" "$THREADS" "$FAMILIES" "$STEPS" "$N" "$MIX" "$OFFSET"
printf '%s\n' '------------------------------------------------------------'

g++ -O3 -std=c++17 -Wall -Wextra -Wpedantic -Werror -fopenmp \
  code/interaction_disambiguation.cpp -o code/interaction_disambiguation

export OMP_NUM_THREADS="$THREADS"
./code/interaction_disambiguation run "$SEEDS" "$THREADS" "$FAMILIES" "$STEPS" "$N" "$MIX" "$OFFSET" \
  2>&1 | tee results/interaction_disambiguation_run.log

printf '%s\n' '------------------------------------------------------------'
python3 scripts/audit_interaction_results.py
