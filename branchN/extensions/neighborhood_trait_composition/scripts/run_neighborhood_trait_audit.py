#!/usr/bin/env python3
import argparse, csv, math, os, subprocess, tempfile, concurrent.futures
from pathlib import Path

EXT = Path(__file__).resolve().parents[1]
ROOT = Path(__file__).resolve().parents[4]
BR = ROOT / 'branchN'
BRANCHN_OUT = BR / 'coordinate_realization_distribution' / 'results'
OUT = EXT / 'results'
CODE = EXT / 'code' / 'neighborhood_trait_probe.cpp'
BIN = EXT / 'code' / 'neighborhood_trait_probe'
HEADER = ['seed','offset','mean_local_A','mean_local_V','local_A_assort','local_V_assort','local_A_to_V','local_V_to_A','local_pair_cov_A','local_pair_cov_V','local_cross_cov_AV','mean_local_within_cov_AV','mean_rank_gap']

def mean(xs): return sum(xs)/len(xs) if xs else float('nan')
def pearson(x,y):
    if len(x)!=len(y) or len(x)<3:return float('nan')
    mx,my=mean(x),mean(y); dx=[a-mx for a in x]; dy=[b-my for b in y]
    den=math.sqrt(sum(a*a for a in dx)*sum(b*b for b in dy))
    return sum(a*b for a,b in zip(dx,dy))/den if den else float('nan')

def q(xs,q):
    xs=sorted(xs); n=len(xs)
    if not n:return float('nan')
    pos=(n-1)*q; lo=int(math.floor(pos)); hi=int(math.ceil(pos))
    return xs[lo] if lo==hi else xs[lo]+(xs[hi]-xs[lo])*(pos-lo)

def compile_probe():
    cmd=['g++','-O2','-std=c++17','-Wall','-Wextra','-Wpedantic','-Werror','-fopenmp',str(CODE),'-o',str(BIN)]
    p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True)
    if p.returncode: raise SystemExit('Probe compilation failed:\n'+p.stdout+'\n'+p.stderr)

def load_branchn(offsets,seeds):
    result={}
    for o in offsets:
        p=BRANCHN_OUT/'raw'/f'offset_{o:03d}'/'matrix_per_seed.csv'
        if not p.exists():
            raise SystemExit(f'Missing BranchN raw output for offset {o}: {p}\nRun BranchN first.')
        with p.open() as f:
            rows=list(csv.DictReader(f))
        got={int(r['seed']):r for r in rows}
        for s in seeds:
            if s not in got: raise SystemExit(f'Missing seed {s} in {p}')
            result[(o,s)]=got[s]
    return result

def probe_one(args):
    o,s=args
    p=subprocess.run([str(BIN),str(s),str(o)],cwd=ROOT,text=True,capture_output=True)
    if p.returncode: return (o,s,None,p.stderr.strip())
    vals=p.stdout.strip().split(',')
    if len(vals)!=len(HEADER): return (o,s,None,'bad probe row: '+p.stdout)
    row=dict(zip(HEADER,vals))
    for k in HEADER[0:2]: row[k]=int(row[k])
    for k in HEADER[2:]: row[k]=float(row[k])
    return (o,s,row,'')

def main():
    ap=argparse.ArgumentParser(description='BranchN extension: test whether M effect variation tracks trait composition of imposed local neighbourhoods.')
    ap.add_argument('--offsets',type=int,nargs='+',default=list(range(24)))
    ap.add_argument('--seeds',type=int,nargs='+',default=[0,101,202,303,404,505,606,707,808,909,1010,1111],help='Must match the seeds used by the BranchN output.')
    ap.add_argument('--workers',type=int,default=min(24,max(1,os.cpu_count() or 1)))
    ap.add_argument('--compile-only',action='store_true')
    args=ap.parse_args()
    if len(args.offsets) < 3:
        raise SystemExit(f'At least 3 coordinate realizations are required for the cross-realization correlation; received {len(args.offsets)}. Use the full BranchN default (24 offsets) for the intended audit.')
    OUT.mkdir(parents=True,exist_ok=True)
    compile_probe()
    if args.compile_only:
        print('PROBE COMPILE PASS',BIN);return
    # BranchN default seeds are P.seed0 + 101*k, i.e. 101..1212. Keep the
    # default explicit here rather than inventing a second seed schedule.
    if args.seeds == [0,101,202,303,404,505,606,707,808,909,1010,1111]:
        args.seeds=[101+101*k for k in range(12)]
    branchn=load_branchn(args.offsets,args.seeds)
    jobs=[(o,s) for o in args.offsets for s in args.seeds]
    probe_rows=[]
    with concurrent.futures.ThreadPoolExecutor(max_workers=min(args.workers,len(jobs))) as pool:
        for o,s,row,err in pool.map(probe_one,jobs):
            if row is None: raise SystemExit(f'probe failed offset={o} seed={s}: {err}')
            probe_rows.append(row)
    with (OUT/'neighborhood_traits_by_seed.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=HEADER);w.writeheader();w.writerows(sorted(probe_rows,key=lambda r:(r['offset'],r['seed'])))

    byoff=[]
    for o in args.offsets:
        pr=[r for r in probe_rows if r['offset']==o]
        effa=[float(branchn[(o,s)]['rA']) for s in args.seeds]
        effdA=[float(branchn[(o,s)]['rA']) for s in args.seeds]
        # The M DY-D contrast is the only requested outcome quantity.
        mA=[float(branchn[(o,s)]['rA'])-float(branchn[(o,s)]['rA']) for s in []]
        # Explicitly reconstruct from mixing/interaction labels.
        mA=[];mV=[]
        for s in args.seeds:
            r=branchn[(o,s)]
            # rows are one of random/metric x directed/dyadic
            # metric rows are selected below.
            # Build lookup once per offset/seed from raw file below.
        raw=BRANCHN_OUT/'raw'/f'offset_{o:03d}'/'matrix_per_seed.csv'
        with raw.open() as f: rr=list(csv.DictReader(f))
        L={(int(x['seed']),x['mixing'],x['interaction']):x for x in rr}
        for s in args.seeds:
            mA.append(float(L[(s,'metric','dyadic')]['rA'])-float(L[(s,'metric','directed')]['rA']))
            mV.append(float(L[(s,'metric','dyadic')]['rV'])-float(L[(s,'metric','directed')]['rV']))
        row={'offset':o,'M_DY_minus_D_rA_mean':mean(mA),'M_DY_minus_D_rV_mean':mean(mV)}
        for k in HEADER[2:]: row[k+'_mean']=mean([r[k] for r in pr])
        byoff.append(row)
    fields=list(byoff[0].keys())
    with (OUT/'neighborhood_traits_by_offset.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=fields);w.writeheader();w.writerows(byoff)

    metrics=['mean_local_A_mean','mean_local_V_mean','local_A_assort_mean','local_V_assort_mean','local_A_to_V_mean','local_V_to_A_mean','local_pair_cov_A_mean','local_pair_cov_V_mean','local_cross_cov_AV_mean','mean_local_within_cov_AV_mean','mean_rank_gap_mean']
    effectsA=[r['M_DY_minus_D_rA_mean'] for r in byoff]; effectsV=[r['M_DY_minus_D_rV_mean'] for r in byoff]
    corrrows=[]
    for k in metrics:
        xs=[r[k] for r in byoff]
        corrrows.append({'trait_metric':k,'corr_with_M_rA_contrast':pearson(xs,effectsA),'corr_with_M_rV_contrast':pearson(xs,effectsV)})
    with (OUT/'neighborhood_effect_correlations.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=corrrows[0].keys());w.writeheader();w.writerows(corrrows)

    strongestA=max(corrrows,key=lambda r:abs(r['corr_with_M_rA_contrast']) if math.isfinite(r['corr_with_M_rA_contrast']) else -1)
    strongestV=max(corrrows,key=lambda r:abs(r['corr_with_M_rV_contrast']) if math.isfinite(r['corr_with_M_rV_contrast']) else -1)
    ra=[r['M_DY_minus_D_rA_mean'] for r in byoff]; rv=[r['M_DY_minus_D_rV_mean'] for r in byoff]
    report=f'''# BranchN extension — trait composition of imposed local neighbourhoods

## What changed

Nothing in frozen GREENY was changed. The extension reads the already-produced BranchN seed-level outputs and independently reconstructs the same population traits and fixed local coordinate from the frozen source. It then asks whether coordinate-realization differences in the M one-sided-to-two-sided effect track the trait composition of the imposed local neighbourhoods.

## What the neighbourhood means

The M kernel orders agents by the fixed exogenous coordinate `z` and pairs circular neighbours. For each agent, the local neighbourhood is therefore defined as its two immediate neighbours in that fixed ordering. The audit measures the mean anxiety and avoidance of those neighbours, local anxiety and avoidance assortativity, cross-trait neighbourhood alignment, pairwise trait covariance, and the mean local A-V covariance within the three-agent neighbourhood {{agent, left neighbour, right neighbour}}.

## Effect being explained

For each coordinate realization and seed:

`M_DY_minus_D_rA = rA(M,Y) - rA(M,D)`

`M_DY_minus_D_rV = rV(M,Y) - rV(M,D)`

The analysis correlates the coordinate-realization means of these effects with the corresponding 24 coordinate-realization means of local trait-composition metrics. This is exploratory/descriptive with {len(args.offsets)} realization means; correlations are not treated as confirmatory p-values.

## Current run summary

M rA contrast across realizations: mean = {mean(ra):.6f}, min = {min(ra):.6f}, max = {max(ra):.6f}, spread = {max(ra)-min(ra):.6f}.

M rV contrast across realizations: mean = {mean(rv):.6f}, min = {min(rv):.6f}, max = {max(rv):.6f}, spread = {max(rv)-min(rv):.6f}.

The strongest absolute correlation with the M rA contrast is `{strongestA['trait_metric']}` with r = {strongestA['corr_with_M_rA_contrast']:.4f}.

The strongest absolute correlation with the M rV contrast is `{strongestV['trait_metric']}` with r = {strongestV['corr_with_M_rV_contrast']:.4f}.

## Interpretation rule

A strong descriptive correlation would mean that which agents are placed into the imposed recurrent neighbourhoods is associated with the observed M effect. That would support a narrower interpretation in terms of imposed local configuration rather than recurrence alone. A weak correlation would leave the realization sensitivity unexplained by these simple trait-composition summaries and point toward trajectory-level dynamics or higher-order structure.

No claim of natural attachment is made. The local neighbourhood is imposed by the contact generator; any state coupling arises downstream from repeated interaction under the GREENY update rule.
'''
    (OUT/'neighborhood_trait_report.md').write_text(report)
    print('NEIGHBORHOOD TRAIT COMPOSITION AUDIT COMPLETE')
    print(f'offsets={args.offsets} seeds_per_offset={len(args.seeds)}')
    print(f'M rA spread={max(ra)-min(ra):.6f} M rV spread={max(rv)-min(rv):.6f}')
    print(f'STRONGEST_R_A_ASSOCIATION {strongestA["trait_metric"]} r={strongestA["corr_with_M_rA_contrast"]:.4f}')
    print(f'STRONGEST_R_V_ASSOCIATION {strongestV["trait_metric"]} r={strongestV["corr_with_M_rV_contrast"]:.4f}')
    print('Report:',OUT/'neighborhood_trait_report.md')

if __name__=='__main__': main()
