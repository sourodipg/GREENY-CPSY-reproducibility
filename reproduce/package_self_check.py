from pathlib import Path
import hashlib, zipfile, sys, subprocess
root=Path('.')
required=['Makefile','README.md','FROZEN_PARENT_TAG.md','code/symmetric_network_experiment.cpp','tests/symbolic_symmetry.py','tests/check_family_run.py','docs/EXPERIMENT_CONTRACT.md','parent/reproduction_package/README.md']
missing=[x for x in required if not (root/x).exists()]
if missing: raise SystemExit('MISSING: '+', '.join(missing))
for z in root.glob('history/*.zip'):
    with zipfile.ZipFile(z) as f:
        bad=f.testzip()
    if bad: raise SystemExit(f'BAD ZIP {z}: {bad}')
print('PACKAGE SELF-CHECK PASS')
