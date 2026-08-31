#!/usr/bin/env python3
import csv, hashlib, subprocess, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BIN=ROOT/'bin'/'symmetric_network_matrix'

def run(*args):
    p=subprocess.run([str(BIN),*map(str,args)],cwd=ROOT,text=True,capture_output=True)
    if p.returncode:
        print(p.stdout); print(p.stderr,file=sys.stderr); raise SystemExit(p.returncode)
    return p.stdout

# Parent provenance check: no extension test may silently alter the frozen parent.
manifest=ROOT/'reference'/'PARENT_MANIFEST.sha256'
p=subprocess.run(['sha256sum','-c',str(manifest)],cwd=ROOT,text=True,capture_output=True)
if p.returncode:
    print(p.stdout); print(p.stderr,file=sys.stderr); raise SystemExit('PARENT IMMUTABILITY FAIL')

run('verify')
run('matrix',2,1,20,400,80,1.0)
one=(ROOT/'results'/'matrix_per_seed.csv').read_bytes()
run('matrix',2,4,20,400,80,1.0)
four=(ROOT/'results'/'matrix_per_seed.csv').read_bytes()
if one!=four: raise SystemExit('THREAD REPRODUCIBILITY FAIL')

# Four-cell contract.
rows=list(csv.DictReader((ROOT/'results'/'matrix_per_seed.csv').open()))
if {(r['mixing'],r['interaction']) for r in rows} != {('random','directed'),('random','dyadic'),('metric','directed'),('metric','dyadic')}:
    raise SystemExit('FOUR-CELL MATRIX FAIL')
# Paired ledger identity is encoded in the digest equality.
for seed in sorted({r['seed'] for r in rows}):
    for mixing in ('random','metric'):
        a=next(r for r in rows if r['seed']==seed and r['mixing']==mixing and r['interaction']=='directed')
        b=next(r for r in rows if r['seed']==seed and r['mixing']==mixing and r['interaction']=='dyadic')
        if a['pair_digest'] != b['pair_digest']:
            raise SystemExit('COMMON LEDGER FAIL')

# Difference-in-differences contract must be computed from the four matched cells.
# This catches accidental labelling of a single cell contrast as the 2x2 interaction.
rows_by_seed={(r['seed'],r['mixing'],r['interaction']):r for r in rows}
for seed in sorted({r['seed'] for r in rows}):
    def val(mix,inter,col): return float(rows_by_seed[(seed,mix,inter)][col])
    for col in ('rA','rV'):
        expected=(val('metric','dyadic',col)-val('metric','directed',col))-(val('random','dyadic',col)-val('random','directed',col))
        # The summary interaction is checked independently by reconstruct_claims.py; this local check
        # guarantees the per-seed algebraic quantity exists and is finite.
        if not __import__('math').isfinite(expected): raise SystemExit('INTERACTION ALGEBRA FAIL')

summary=list(csv.DictReader((ROOT/'results'/'differences.csv').open()))
for col in ('rA','rV'):
    got=float(next(r for r in summary if r['contrast']==f'2x2_interaction_{col}')['mean_diff'])
    per_seed=[]
    for seed_id in sorted({r['seed'] for r in rows}):
        def sval(mix,inter): return float(rows_by_seed[(seed_id,mix,inter)][col])
        per_seed.append((sval('metric','dyadic')-sval('metric','directed'))-(sval('random','dyadic')-sval('random','directed')))
    expected=sum(per_seed)/len(per_seed)
    if abs(got-expected)>1e-6: raise SystemExit(f'INTERACTION RECONSTRUCTION FAIL {col}: {got} != {expected}')


# metric_strength=0 must generate the same random ledger in both labelled mixing cells.
run('matrix',2,1,20,400,80,0.0)
rows0=list(csv.DictReader((ROOT/'results'/'matrix_per_seed.csv').open()))
for seed in sorted({r['seed'] for r in rows0}):
    for inter in ('directed','dyadic'):
        a=next(r for r in rows0 if r['seed']==seed and r['mixing']=='random' and r['interaction']==inter)
        b=next(r for r in rows0 if r['seed']==seed and r['mixing']=='metric' and r['interaction']==inter)
        if a['pair_digest'] != b['pair_digest']:
            raise SystemExit('ZERO-STRENGTH FAIL')

print('TEST SUITE PASS')
