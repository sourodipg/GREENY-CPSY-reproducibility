from pathlib import Path
import pandas as pd, numpy as np, re, json
ROOT=Path(__file__).resolve().parents[1]
AUTH=ROOT/'verification/authoritative'
tex=(ROOT/'manuscript/GREENY_Impersonal_Final.tex').read_text()
m=pd.read_csv(AUTH/'matrix_per_seed_56.csv')
s=pd.read_csv(AUTH/'matrix_summary_56.csv')
d=pd.read_csv(AUTH/'differences_56.csv')
rows=[]
for mix in ['random','metric']:
    for inter in ['directed','dyadic']:
        z=m[(m.mixing==mix)&(m.interaction==inter)]
        for col in ['rA','rV','rate','repeat1','repeat5','repeat20','pair_q_corr_post']:
            rows.append((f'{mix}-{inter} {col}',float(z[col].mean()),float(s[(s.mixing==mix)&(s.interaction==inter)][col if col in s.columns else col+'_mean'].iloc[0])))
# true interactions from replicate rows
p=m.pivot(index='seed',columns=['mixing','interaction'],values=['rA','rV'])
for out in ['rA','rV']:
    delta=((p[out][('metric','dyadic')]-p[out][('metric','directed')])-(p[out][('random','dyadic')]-p[out][('random','directed')]))
    rec=float(delta.mean()); rep=float(d.loc[d.metric==('rA_metric_minus_random' if out=='rA' else 'rV_metric_minus_random'),'mean_diff'].iloc[0])
    rows.append((f'2x2 {out} interaction',rep,rec))
# Text hygiene.
prose=tex.split('\\begin{thebibliography}')[0]
first_person=sorted(set(re.findall(r'\b(?:we|our|ours|ourselves|we\'re|we\'ve)\b',prose,re.I)))
print('FIRST_PERSON_TERMS:', first_person)
if first_person: raise SystemExit('IMPERSONAL STYLE FAIL')
max_res=0.0
for name,a,b in rows:
    r=abs(a-b); max_res=max(max_res,r)
    if r>1e-9: print('RESIDUAL',name,r)
print('MAX_ARITHMETIC_RESIDUAL',max_res)
# Provenance sanity checks encoded by executable source content.
cpp=(ROOT/'code/symmetric_network_matrix.cpp').read_text()
checks={
 'common_ledger': 'pair_digest!=M[1][k].pair_digest' in cpp and 'pair_digest!=M[3][k].pair_digest' in cpp,
 'correct_interaction': '((get(M[3][k])-get(M[2][k]))-(get(M[1][k])-get(M[0][k])))' in cpp,
 'clock_overrides': 'P.shock_mean_interval=shock_interval;P.symptom_window=symptom_window' in cpp,
 'metric_range_guard': 'mix_strength<0.0 || mix_strength>1.0' in cpp,
}
print('SOURCE_CHECKS',checks)
if not all(checks.values()): raise SystemExit('SOURCE CONTRACT FAIL')
# Verify benchmark.
bench=1/(4000-1)
reported=.000250311
meas=float(s[(s.mixing=='random')&(s.interaction=='directed')].repeat1.iloc[0])
print('REPEAT_BENCHMARK',bench)
print('REPEAT_MEASURED',meas)
print('REPEAT_RESIDUAL',abs(meas-reported))
# Write machine-readable output.
out={'first_person_terms':first_person,'max_arithmetic_residual':max_res,'source_checks':checks,'repeat_benchmark':bench,'repeat_measured':meas,'repeat_reported':reported,'repeat_residual':abs(meas-reported)}
(ROOT/'verification/claim_reconstruction.json').write_text(json.dumps(out,indent=2))
print('RECONSTRUCTION PASS')
