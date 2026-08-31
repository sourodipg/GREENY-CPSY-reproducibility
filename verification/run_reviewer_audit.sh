#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
mkdir -p verification/runs
{
  echo '===== CORE VERIFY ====='
  make verify
  echo '===== SANITIZER ====='
  make sanitizer
  echo '===== CLAIM RECONSTRUCTION ====='
  python3 verification/reconstruct_claims.py
  echo '===== RECIPIENT ORDER AUDIT ====='
  python3 verification/directed_recipient_order_audit.py
  echo '===== COORDINATE REALIZATION AUDIT (FAST) ====='
  make coordinate-audit
  echo '===== REVIEWER GAP REPORT ====='
  python3 verification/reviewer_gap_report.py
  echo '===== PREFLIGHT ====='
  python3 reproduce/verify_preflight.py
  echo '===== PACKAGE SELF-CHECK ====='
  python3 reproduce/package_self_check.py
} 2>&1 | tee verification/runs/GREENY_REVIEWER_AUDIT_ALL.log
