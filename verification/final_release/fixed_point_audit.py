#!/usr/bin/env python3
"""GREENY final release self-check.

This script is intentionally legible: every check states the scientific/software
question before reporting PASS or FAIL. It validates the latest interaction fork
against the authoritative CSVs and the manuscript's reported numbers. It does
not rerun stochastic GREENY trajectories.
"""
from pathlib import Path
import csv, re, sys
ROOT=Path(__file__).resolve().parents[2]
R=ROOT/'results'
IF=ROOT/'branchN/forks/interaction_disambiguation/results'
MAN=ROOT/'deliverables/GREENY_ABC_Impersonal_Manuscript_FINAL.tex'
checks=[]
def chk(name, ok, detail):
    checks.append((name, bool(ok), detail)); print(('PASS' if ok else 'FAIL')+' | '+name+' | '+detail)
# 1 latest result presence
for f in [IF/'interaction_disambiguation_per_seed.csv',IF/'interaction_disambiguation_contrasts.csv',IF/'interaction_disambiguation_summary.csv',IF/'FULL_RUN_CAPTURE.log',MAN]:
    chk('FILE PRESENT', f.exists(), str(f.relative_to(ROOT)))
# 2 560 rows
rows=list(csv.DictReader(open(IF/'interaction_disambiguation_per_seed.csv')))
chk('560 SEED-CONDITION ROWS', len(rows)==560, f'{len(rows)} rows = 56 seeds x 2 regimes x 5 conditions')
conds=sorted(set(r['condition'] for r in rows)); chk('FIVE CONDITIONS', conds==['D','D_SWAP','Y','Y_ABLATE','Y_HALFALPHA'], str(conds))
# 3 finite values
finite=True
for r in rows:
    for k in ['rA','rV','rate','pair_x_corr_post','pair_q_corr_post','repeat1','repeat5','repeat20']:
        try: finite &= float(r[k])==float(r[k])
        except: finite=False
chk('FINITE NUMERICAL OUTPUTS', finite, 'all audited numeric fields parse as finite values')
# 4 contrast exactness: recompute mean paired diffs
from collections import defaultdict
by=defaultdict(dict)
for r in rows: by[(r['mixing'],r['seed'])][r['condition']]=r
expected=[('metric','D_SWAP','D','rA',0.00565727),('metric','D_SWAP','D','rV',0.00583944),('metric','Y_ABLATE','Y','rA',-0.00808485),('metric','Y_ABLATE','Y','rV',-0.0000609025),('metric','Y_HALFALPHA','D','rA',-0.0455439),('metric','Y_HALFALPHA','D','rV',-0.0374726),('random','D_SWAP','D','rA',-0.00311231),('random','D_SWAP','D','rV',-0.00342238),('random','Y_ABLATE','Y','rA',-0.00765531),('random','Y_ABLATE','Y','rV',-0.000207393),('random','Y_HALFALPHA','D','rA',-0.0373827),('random','Y_HALFALPHA','D','rV',-0.034742)]
for mix,a,b,m,val in expected:
    xs=[float(by[(mix,s)][a][m])-float(by[(mix,s)][b][m]) for (mx,s) in by if mx==mix]
    mean=sum(xs)/len(xs)
    chk(f'RECONSTRUCT {mix} {a}-{b} {m}', abs(mean-val)<5e-6, f'computed={mean:.7f} reported={val:.7f}')
# 5 ledger digest equality
ledger_ok=True; bad=[]
for key,d in by.items():
    dig={r['pair_digest'] for r in d.values()}
    if len(dig)!=1: ledger_ok=False; bad.append(key)
chk('COMMON ENCOUNTER LEDGER', ledger_ok, 'all five interaction conditions share one pair digest per seed/regime' if ledger_ok else f'mismatches={bad[:5]}')
# 6 manuscript frozen-term removal
text=MAN.read_text()
chk('NO FROZEN SCIENTIFIC TERMINOLOGY', 'frozen' not in text.lower(), 'the manuscript introduces GREENY directly; version immutability remains an archive concern')
# 7 key manuscript numbers
need=['-0.0687','-0.0652','-0.038213','0.045026','-0.023904','0.032138','-0.00808','-0.04554','D_{swap}','Y_{ablate}','Y_{half\\alpha}']
chk('KEY RESULTS PRESENT', all(x in text for x in need), 'primary, BranchN and new counterfactual results are represented')
# 8 fixed-point definition
allok=all(ok for _,ok,_ in checks)
print('\nFIXED-POINT DECISION')
if allok:
    print('PASS | FIXED POINT REACHED | no unresolved internal result/manuscript contradiction remains in the released evidence chain.')
    print('BOUNDARY | stochastic trajectories are not rerun by this audit; the latest user-supplied production outputs are treated as authoritative.')
else:
    print('FAIL | FIXED POINT NOT REACHED | resolve failed checks before treating the release as final.')
sys.exit(0 if allok else 1)
