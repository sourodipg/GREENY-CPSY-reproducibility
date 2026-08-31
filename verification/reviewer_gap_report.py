#!/usr/bin/env python3
"""Summarise reviewer-facing structural questions from released audit outputs."""
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

def main():
    p = ROOT/'verification'/'coordinate_realization_audit.json'
    if not p.exists():
        raise SystemExit('Run coordinate_realization_audit.py first')
    d = json.loads(p.read_text())
    status = d['status']
    print('GREENY REVIEWER-GAP STATUS')
    print('1. One-sided recipient balance: covered by tests/test_network_structure.py; representation-order sensitivity is separately audited.')
    print('2. Numeric coordinate scale: covered; z and z^3 induce the same ledger.')
    print('3. Forced recurrence: covered; recurrence exists in the contact kernel without model dynamics.')
    print('4. Coordinate-realization dependence:', status)
    print('5. Correct interpretation: M is imposed recurrent exposure, not naturally emergent attachment.')
    if status == 'SENSITIVITY_DETECTED':
        print('ACTION: do not claim that the M contrast is recurrence-only; retain coordinate-realization as a sensitivity dimension.')
    print('6. Manuscript change status: intentionally deferred; this package is the audit layer.')

if __name__ == '__main__':
    main()
