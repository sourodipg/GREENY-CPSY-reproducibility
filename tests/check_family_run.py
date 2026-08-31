import csv, sys
from pathlib import Path
F=int(sys.argv[1]); seeds=int(sys.argv[2]); N=int(sys.argv[3]);
root=Path(f"results/families_{F}")
log=(root/'run.log').read_text()
needle=f"seeds={seeds} "
if needle not in log or f"N={N} " not in log or f"families={F} " not in log:
    raise SystemExit(f"CONFIG CHECK FAIL: {root}")
rows=list(csv.DictReader((root/'paired.csv').open()))
if len(rows)!=seeds: raise SystemExit(f"ROW COUNT FAIL: expected {seeds}, got {len(rows)}")
for row in rows:
    for k,v in row.items():
        if k=='seed': continue
        float(v)
print(f"FAMILY RUN PASS F={F} N={N} seeds={seeds}")
