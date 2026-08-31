from pathlib import Path
import csv, math
r=Path('results')
for f in ['pair_dyad_paired.csv','paired_statistics.csv','summary.csv']:
    assert (r/f).exists(), f
for row in csv.DictReader((r/'pair_dyad_paired.csv').open()):
    for k,v in row.items():
        if k=='seed': continue
        x=float(v); assert math.isfinite(x), (k,v)
for row in csv.DictReader((r/'summary.csv').open()):
    for k,v in row.items():
        if k=='condition': continue
        x=float(v); assert math.isfinite(x), (k,v)
print('RESULT INVARIANTS PASS')
