#!/usr/bin/env python3
import argparse,csv,math,os,subprocess,concurrent.futures,shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
EXT=ROOT/'branchN/extensions/temporal_exposure_network'
BIN=EXT/'code/contact_graph_probe'
BR=ROOT/'branchN/coordinate_realization_distribution/results'
OUT=EXT/'results'
SEEDS=[101+101*k for k in range(12)]
HEADER=['seed','offset','N','steps','families','parity_even','parity_odd','n_edges','density','components','degree_mean','degree_sd','strength_mean','strength_sd','edge_weight_min','edge_weight_max','edge_weight_mean','edge_weight_cv','global_clustering','A_assort_unweighted','V_assort_unweighted','A_assort_weighted','V_assort_weighted','mean_abs_dA','mean_abs_dV','weighted_abs_dA','weighted_abs_dV']

def compile_probe():
    cmd=['g++','-O3','-std=c++17','-Wall','-Wextra','-Wpedantic','-Werror','-fopenmp',str(EXT/'code/contact_graph_probe.cpp'),'-o',str(BIN)]
    p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True)
    if p.returncode: raise SystemExit('probe compilation failed:\n'+p.stdout+'\n'+p.stderr)

def load_effects(offsets,seeds):
    out={}
    for o in offsets:
        p=BR/'raw'/f'offset_{o:03d}'/'matrix_per_seed.csv'
        if not p.exists(): raise SystemExit(f'Missing BranchN output: {p}')
        rows=list(csv.DictReader(p.open()))
        L={(int(r['seed']),r['mixing'],r['interaction']):r for r in rows}
        for s in seeds:
            if (s,'metric','directed') not in L or (s,'metric','dyadic') not in L: raise SystemExit(f'Missing metric cells offset={o} seed={s}')
            out[(o,s,'rA')]=float(L[(s,'metric','dyadic')]['rA'])-float(L[(s,'metric','directed')]['rA'])
            out[(o,s,'rV')]=float(L[(s,'metric','dyadic')]['rV'])-float(L[(s,'metric','directed')]['rV'])
    return out

def pearson(x,y):
    if len(x)<3:return float('nan')
    mx=sum(x)/len(x);my=sum(y)/len(y);a=b=c=0
    for xx,yy in zip(x,y):
        dx=xx-mx;dy=yy-my;a+=dx*dy;b+=dx*dx;c+=dy*dy
    return a/math.sqrt(b*c) if b>0 and c>0 else float('nan')

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--offsets',type=int,nargs='+',default=list(range(24)))
    ap.add_argument('--seeds',type=int,nargs='+',default=SEEDS)
    ap.add_argument('--workers',type=int,default=min(24,max(1,os.cpu_count() or 1)))
    ap.add_argument('--steps',type=int,default=12000);ap.add_argument('--n',type=int,default=4000);ap.add_argument('--families',type=int,default=1000)
    args=ap.parse_args();OUT.mkdir(parents=True,exist_ok=True)
    compile_probe();eff=load_effects(args.offsets,args.seeds)
    jobs=[(o,s) for o in args.offsets for s in args.seeds]
    def run(job):
        o,s=job;p=subprocess.run([str(BIN),str(s),str(o),str(args.steps),str(args.n),str(args.families)],cwd=ROOT,text=True,capture_output=True)
        if p.returncode:return o,s,None,p.stderr
        vals=p.stdout.strip().split(',');return o,s,dict(zip(HEADER,[int(v) if k in ['seed','offset','N','steps','families','parity_even','parity_odd','n_edges','components'] else float(v) for k,v in zip(HEADER,vals)])),''
    rows=[]
    with concurrent.futures.ThreadPoolExecutor(max_workers=min(args.workers,len(jobs))) as pool:
        for o,s,row,err in pool.map(run,jobs):
            if row is None: raise SystemExit(f'probe failed offset={o} seed={s}: {err}')
            row['M_DY_minus_D_rA']=eff[(o,s,'rA')];row['M_DY_minus_D_rV']=eff[(o,s,'rV')];rows.append(row)
    rows.sort(key=lambda r:(r['offset'],r['seed']))
    for r in rows:
        if r['n_edges'] != r['N'] or r['degree_mean'] != 2.0 or r['components'] != 1:
            raise SystemExit(f'M frozen-cycle invariant failed offset={r["offset"]} seed={r["seed"]}: n_edges={r["n_edges"]} degree_mean={r["degree_mean"]} components={r["components"]}')
        if abs(r['global_clustering']) > 1e-15:
            raise SystemExit(f'M cycle clustering invariant failed offset={r["offset"]} seed={r["seed"]}: {r["global_clustering"]}')
    with (OUT/'network_metrics_by_seed.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=HEADER+['M_DY_minus_D_rA','M_DY_minus_D_rV']);w.writeheader();w.writerows(rows)
    byoff=[]
    for o in args.offsets:
        rr=[r for r in rows if r['offset']==o]
        d={'offset':o}
        for k in HEADER[5:]:d[k+'_mean']=sum(float(r[k]) for r in rr)/len(rr)
        d['M_DY_minus_D_rA_mean']=sum(r['M_DY_minus_D_rA'] for r in rr)/len(rr);d['M_DY_minus_D_rV_mean']=sum(r['M_DY_minus_D_rV'] for r in rr)/len(rr)
        byoff.append(d)
    with (OUT/'network_metrics_by_offset.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=byoff[0].keys());w.writeheader();w.writerows(byoff)
    graph_metrics=['degree_mean_mean','strength_mean_mean','edge_weight_mean_mean','edge_weight_cv_mean','A_assort_unweighted_mean','V_assort_unweighted_mean','A_assort_weighted_mean','V_assort_weighted_mean','mean_abs_dA_mean','mean_abs_dV_mean','weighted_abs_dA_mean','weighted_abs_dV_mean']
    cr=[]
    ea=[r['M_DY_minus_D_rA_mean'] for r in byoff];ev=[r['M_DY_minus_D_rV_mean'] for r in byoff]
    for g in graph_metrics:
        xs=[r[g] for r in byoff];cr.append({'graph_metric':g,'corr_with_M_rA_contrast':pearson(xs,ea),'corr_with_M_rV_contrast':pearson(xs,ev)})
    with (OUT/'network_metric_effect_correlations.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=cr[0].keys());w.writeheader();w.writerows(cr)
    finiteA=[r for r in cr if math.isfinite(r['corr_with_M_rA_contrast'])]; finiteV=[r for r in cr if math.isfinite(r['corr_with_M_rV_contrast'])]
    strongestA=max(finiteA,key=lambda r:abs(r['corr_with_M_rA_contrast'])) if finiteA else {'graph_metric':'not_estimable','corr_with_M_rA_contrast':float('nan')}
    strongestV=max(finiteV,key=lambda r:abs(r['corr_with_M_rV_contrast'])) if finiteV else {'graph_metric':'not_estimable','corr_with_M_rV_contrast':float('nan')}
    report=f'''# BranchN extension — literature-grounded temporal/contact-network audit

## Noob answer

The graph is an **imposed contact/exposure graph**, not an emergent friendship or attachment network. The frozen M kernel repeatedly places neighbouring agents in a fixed coordinate ordering into contact.

## Run

Offsets: {len(args.offsets)}; seeds per offset: {len(args.seeds)}; frozen trajectories represented: {len(args.offsets)*len(args.seeds)}.

## Structural result

For even N, the frozen metric-local matching has only two distinct alternating perfect matchings because cyclic shifts differing by two produce the same unordered pairing. Their union is therefore a cycle through the rank-ordered agents.

Across the audited realizations, the graph should therefore have degree 2 for every node, one connected component, zero binary triangle clustering for N={args.n}, and total node strength equal to the number of simulation steps because every node participates in exactly one pair per timestep.

The actual output provides these values rather than assuming them.

## Exposure intensity

Edge weight means the number of times a pair is forced to interact. This is a standard weighted-network quantity; it should be read as **repeated imposed exposure**, not relationship strength in a psychological sense.

## Trait mixing

The primary attribute statistics are scalar edge-end assortativity for anxiety and avoidance, reported both unweighted and weighted by exposure count. Newman introduced scalar assortativity for node attributes; weighted-network literature motivates retaining contact weights rather than silently binarizing them.

## BranchN question

The upstream BranchN experiment holds recurrence nearly fixed while changing which agents occupy the local neighbourhoods. The downstream question is therefore whether standard graph structure or trait mixing explains the resulting variation in the M one-sided-to-two-sided contrast.

Strongest descriptive association with M `rA`: `{strongestA['graph_metric']}`, r = {strongestA['corr_with_M_rA_contrast']:.4f}.

Strongest descriptive association with M `rV`: `{strongestV['graph_metric']}`, r = {strongestV['corr_with_M_rV_contrast']:.4f}.

These 24-realization correlations are exploratory descriptions, not confirmatory p-values.

## Literature

- Holme & Saramäki (2012): temporal/contact-sequence representation and time-aggregated graphs.
- Newman (2003): scalar attribute assortativity / mixing.
- Newman (2004): weighted networks.
- Barrat et al. (2004): degree/strength structure in weighted networks.
- Opsahl & Panzarasa (2009): weighted clustering.
- Blonder & Dornhaus (2012): time-ordered versus time-aggregated network analysis.
- Karsai et al. (2014): repeated interactions and temporal network structure.

See `docs/LITERATURE_NETWORK_METHODS.md` for full references and methodological notes.
'''
    (OUT/'temporal_network_report.md').write_text(report)
    print('TEMPORAL EXPOSURE NETWORK AUDIT COMPLETE')
    print(f'offsets={len(args.offsets)} seeds_per_offset={len(args.seeds)}')
    print(f'graph_instances={len(rows)}')
    print('EXPECTED_M_UNION_GRAPH=cycle_on_rank_ordered_agents')
    print(f'STRONGEST_R_A_ASSOCIATION {strongestA["graph_metric"]} r={strongestA["corr_with_M_rA_contrast"]:.4f}')
    print(f'STRONGEST_R_V_ASSOCIATION {strongestV["graph_metric"]} r={strongestV["corr_with_M_rV_contrast"]:.4f}')
    print('Report:',OUT/'temporal_network_report.md')
if __name__=='__main__':main()
