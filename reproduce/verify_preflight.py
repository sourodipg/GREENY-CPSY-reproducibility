#!/usr/bin/env python3
from pathlib import Path
import subprocess, sys, filecmp
ROOT=Path(__file__).resolve().parents[1]
BIN=ROOT/'bin'/'symmetric_network_matrix'
REF=ROOT/'reference'/'preflight_4seed'
for f in ['matrix_per_seed.csv','matrix_summary.csv','differences.csv']:
    p=ROOT/'results'/f
    if p.exists(): p.unlink()
cmd=[str(BIN),'matrix','4','4','100','1200','400','1.0']
p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True)
if p.returncode:
    print(p.stdout);print(p.stderr,file=sys.stderr);sys.exit(2)
for f in ['matrix_per_seed.csv','matrix_summary.csv','differences.csv']:
    if not filecmp.cmp(ROOT/'results'/f,REF/f,shallow=False):
        print('REFERENCE DIFFER',f);sys.exit(1)
print('REFERENCE PREFLIGHT PASS')
