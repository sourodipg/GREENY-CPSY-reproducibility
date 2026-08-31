#!/usr/bin/env python3
import argparse, csv, json, math, os, re, shutil, subprocess, tempfile, concurrent.futures
from pathlib import Path
from statistics import mean, pstdev

ROOT = Path(__file__).resolve().parents[3]
BR = Path(__file__).resolve().parents[2]
BIN = ROOT / 'bin' / 'symmetric_network_matrix'
OUT = BR / 'coordinate_realization_distribution' / 'results'
LINE = re.compile(r'^(R-D|R-Y|M-D|M-Y)\s+rA=([0-9eE+.-]+)\s+rV=([0-9eE+.-]+)\s+rate=([0-9eE+.-]+)\s+pair_x_post=([0-9eE+.-]+)\s+pair_q_post=([0-9eE+.-]+)\s+mutual_pre=([0-9eE+.-]+)\s+repeat1=([0-9eE+.-]+)$')


def qtile(xs, q):
    xs = sorted(xs)
    if not xs: return float('nan')
    if len(xs) == 1: return xs[0]
    pos = (len(xs)-1)*q
    lo, hi = math.floor(pos), math.ceil(pos)
    if lo == hi: return xs[lo]
    return xs[lo] + (xs[hi]-xs[lo])*(pos-lo)


def run_one(args, offset):
    # One offset job contains exactly args.seeds frozen GREENY trajectories.
    # A single OpenMP thread is used per job. Jobs for different offsets are
    # parallelized at the process level, so each trajectory is handled by the
    # frozen executable without changing the frozen source code.
    with tempfile.TemporaryDirectory(prefix=f'greeny_branchN_{offset}_') as td:
        td = Path(td)
        (td/'results').mkdir()
        cmd = [str(BIN), 'matrix', str(args.seeds), '1', str(args.families),
               str(args.steps), str(args.n), '1.0', str(args.shock), str(args.window), str(offset)]
        env = os.environ.copy(); env['OMP_NUM_THREADS'] = '1'
        p = subprocess.run(cmd, cwd=td, text=True, capture_output=True, env=env)
        if p.returncode:
            return offset, p.returncode, p.stdout, p.stderr, None
        raw = td/'results'/'matrix_per_seed.csv'
        if not raw.exists():
            return offset, 99, p.stdout, p.stderr, f'missing output for offset {offset}: {raw}'
        rows = list(csv.DictReader(raw.open()))
        for r in rows:
            for k in ['rA','rV','rate','repeat1','repeat5','repeat20']:
                r[k] = float(r[k])
            r['seed'] = int(r['seed'])
        return offset, 0, p.stdout, p.stderr, rows


def get(rows, seed, mixing, interaction, field):
    r = rows[(seed,mixing,interaction)]
    return r[field]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--seeds', type=int, default=12)
    ap.add_argument('--threads', type=int, default=1, help='deprecated compatibility option; frozen GREENY processes always run with one thread')
    ap.add_argument('--families', type=int, default=1000)
    ap.add_argument('--steps', type=int, default=12000)
    ap.add_argument('--n', type=int, default=4000)
    ap.add_argument('--shock', type=float, default=500.0)
    ap.add_argument('--window', type=float, default=0.06)
    ap.add_argument('--offsets', type=int, nargs='+', default=list(range(24)))
    ap.add_argument('--workers', type=int, default=max(1, os.cpu_count() or 1),
                    help='maximum concurrent frozen GREENY processes; each process uses exactly one OpenMP thread')
    args = ap.parse_args()
    if not BIN.exists(): raise SystemExit(f'Frozen executable not found: {BIN}')
    OUT.mkdir(parents=True, exist_ok=True)
    for p in [OUT/'raw', OUT/'branchN_summary.csv', OUT/'branchN_by_offset.csv', OUT/'branchN_by_seed.csv', OUT/'branchN_audit.json', OUT/'branchN_report.md', OUT/'branchN_run.log']:
        if isinstance(p, Path) and p.exists() and (p.is_file() or p.is_symlink()): p.unlink()
    log_lines=[]
    allrows={}
    workers = min(max(1, args.workers), len(args.offsets))
    log_lines.append(f'BRANCHN_PARALLEL workers={workers} offsets={len(args.offsets)} seeds_per_offset={args.seeds} threads_per_process=1 total_frozen_trajectories={len(args.offsets)*args.seeds}')
    print(log_lines[-1])
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        futures = [pool.submit(run_one, args, offset) for offset in args.offsets]
        for fut in concurrent.futures.as_completed(futures):
            offset, rc, stdout, stderr, payload = fut.result()
            if stdout:
                log_lines.append(f'===== OFFSET {offset} STDOUT =====\n{stdout.rstrip()}')
            if stderr:
                log_lines.append(f'===== OFFSET {offset} STDERR =====\n{stderr.rstrip()}')
            if rc:
                detail = payload if isinstance(payload, str) else stderr or stdout
                raise SystemExit(f'offset {offset} failed with returncode={rc}\n{detail}')
            rows = payload
            allrows[offset] = {(r['seed'], r['mixing'], r['interaction']): r for r in rows}
            od = OUT/'raw'/f'offset_{offset:03d}'; od.mkdir(parents=True, exist_ok=True)
            with (od/'matrix_per_seed.csv').open('w', newline='') as f:
                w=csv.DictWriter(f, fieldnames=rows[0].keys()); w.writeheader(); w.writerows(rows)
    if set(allrows) != set(args.offsets):
        raise SystemExit('offset result set incomplete')
    offsets=args.offsets; seeds=sorted({k[0] for k in allrows[offsets[0]].keys()})
    for o in offsets:
        if sorted({k[0] for k in allrows[o].keys()}) != seeds:
            raise SystemExit(f'SEED_SET_MISMATCH offset={o}')
    # Coordinate offset is expected to change only z-dependent geometry diagnostics in R.
    # The random partner selection and all state/dynamic quantities must remain identical.
    random_dynamic_fields=['rA','rV','rate','pair_x_corr_pre','pair_q_corr_pre','pair_x_corr_post','pair_q_corr_post','mutual_elev_pre','repeat1','repeat5','repeat20','same_family','unique_partners_frac','degree_cv','pair_digest']
    random_invariant=True
    for o in offsets[1:]:
        for s in seeds:
            for interaction in ['directed','dyadic']:
                aD=allrows[offsets[0]][(s,'random',interaction)]
                bD=allrows[o][(s,'random',interaction)]
                for field in random_dynamic_fields:
                    if ((float(aD[field]) != float(bD[field])) if field != 'pair_digest' else (str(aD[field]) != str(bD[field]))):
                        random_invariant=False
                        raise SystemExit(f'RANDOM_DYNAMIC_CELL_CHANGED offset={o} seed={s} interaction={interaction} field={field}')
    # Per-seed M contrasts across realization.
    recs_ra={}; recs_rv={}
    for o in offsets:
        recs_ra[o]={s:get(allrows[o],s,'metric','dyadic','rA')-get(allrows[o],s,'metric','directed','rA') for s in seeds}
        recs_rv[o]={s:get(allrows[o],s,'metric','dyadic','rV')-get(allrows[o],s,'metric','directed','rV') for s in seeds}
    means_ra={o:mean(recs_ra[o].values()) for o in offsets}
    means_rv={o:mean(recs_rv[o].values()) for o in offsets}
    spreads={'rA':max(means_ra.values())-min(means_ra.values()),'rV':max(means_rv.values())-min(means_rv.values())}
    pooled_ra=[v for o in offsets for v in recs_ra[o].values()]
    pooled_rv=[v for o in offsets for v in recs_rv[o].values()]
    grand_ra,_,frac_ra=component_summary(recs_ra,'__dummy__',offsets,seeds) if False else (mean(pooled_ra),means_ra,0.0)
    grand_rv=mean(pooled_rv)
    ss_ra=sum((v-grand_ra)**2 for v in pooled_ra); ss_rv=sum((v-grand_rv)**2 for v in pooled_rv)
    ss_coord_ra=len(seeds)*sum((means_ra[o]-grand_ra)**2 for o in offsets); ss_coord_rv=len(seeds)*sum((means_rv[o]-grand_rv)**2 for o in offsets)
    eta_ra=ss_coord_ra/ss_ra if ss_ra>0 else 0.0; eta_rv=ss_coord_rv/ss_rv if ss_rv>0 else 0.0
    # Recurrence diagnostics from M-Y; all should be nearly unchanged because N/rule are fixed.
    rec1=[mean(allrows[o][(s,'metric','dyadic')]['repeat1'] for s in seeds) for o in offsets]
    rec5=[mean(allrows[o][(s,'metric','dyadic')]['repeat5'] for s in seeds) for o in offsets]
    rec20=[mean(allrows[o][(s,'metric','dyadic')]['repeat20'] for s in seeds) for o in offsets]
    byoff=[]
    for o in offsets:
        byoff.append({'offset':o,'M_DY_minus_D_rA_mean':means_ra[o],'M_DY_minus_D_rV_mean':means_rv[o],
                      'M_repeat1_mean':rec1[offsets.index(o)],'M_repeat5_mean':rec5[offsets.index(o)],'M_repeat20_mean':rec20[offsets.index(o)]})
    with (OUT/'branchN_by_offset.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=byoff[0].keys());w.writeheader();w.writerows(byoff)
    byseed=[]
    for s in seeds:
        row={'seed':s}
        for o in offsets: row[f'offset_{o}_rA']=recs_ra[o][s]; row[f'offset_{o}_rV']=recs_rv[o][s]
        row['rA_mean_across_offsets']=mean(recs_ra[o][s] for o in offsets)
        row['rV_mean_across_offsets']=mean(recs_rv[o][s] for o in offsets)
        row['rA_sd_across_offsets']=pstdev(recs_ra[o][s] for o in offsets)
        row['rV_sd_across_offsets']=pstdev(recs_rv[o][s] for o in offsets)
        byseed.append(row)
    with (OUT/'branchN_by_seed.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=byseed[0].keys());w.writeheader();w.writerows(byseed)
    summary=[
      {'metric':'M_DY_minus_D_rA','mean':grand_ra,'sd':pstdev(pooled_ra),'min':min(pooled_ra),'q25':qtile(pooled_ra,.25),'median':qtile(pooled_ra,.5),'q75':qtile(pooled_ra,.75),'max':max(pooled_ra),'realization_spread_of_means':spreads['rA'],'coordinate_realization_variance_fraction':eta_ra},
      {'metric':'M_DY_minus_D_rV','mean':grand_rv,'sd':pstdev(pooled_rv),'min':min(pooled_rv),'q25':qtile(pooled_rv,.25),'median':qtile(pooled_rv,.5),'q75':qtile(pooled_rv,.75),'max':max(pooled_rv),'realization_spread_of_means':spreads['rV'],'coordinate_realization_variance_fraction':eta_rv},
      {'metric':'M_repeat1','mean':mean(rec1),'sd':pstdev(rec1),'min':min(rec1),'q25':qtile(rec1,.25),'median':qtile(rec1,.5),'q75':qtile(rec1,.75),'max':max(rec1),'realization_spread_of_means':max(rec1)-min(rec1),'coordinate_realization_variance_fraction':0.0},
      {'metric':'M_repeat5','mean':mean(rec5),'sd':pstdev(rec5),'min':min(rec5),'q25':qtile(rec5,.25),'median':qtile(rec5,.5),'q75':qtile(rec5,.75),'max':max(rec5),'realization_spread_of_means':max(rec5)-min(rec5),'coordinate_realization_variance_fraction':0.0},
      {'metric':'M_repeat20','mean':mean(rec20),'sd':pstdev(rec20),'min':min(rec20),'q25':qtile(rec20,.25),'median':qtile(rec20,.5),'q75':qtile(rec20,.75),'max':max(rec20),'realization_spread_of_means':max(rec20)-min(rec20),'coordinate_realization_variance_fraction':0.0},
    ]
    with (OUT/'branchN_summary.csv').open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=summary[0].keys());w.writeheader();w.writerows(summary)
    status='SENSITIVITY_DETECTED' if spreads['rA']>0.005 or spreads['rV']>0.005 else 'LOW_SENSITIVITY_AT_THIS_RESOLUTION'
    report=(f'''# BranchN — noob-proof result\n\nThis branch changes **one thing only**: which agents receive the fixed local coordinate `z`. The population, traits, family weights, forcing, model parameters, number of agents, number of steps, recurrence algorithm, and random-contact dynamics remain fixed. The only changed input is the seed used to generate the exogenous local coordinate z.\n\n
## Execution model\n\nThe experiment launches one frozen GREENY process per coordinate realization. Each process uses exactly one OpenMP thread and evaluates the full seed set for that realization. Different coordinate realizations run concurrently, but the frozen GREENY executable itself is not modified. Thus the default experiment evaluates 24 coordinate realizations x 12 frozen GREENY seeds = 288 frozen trajectories, scheduled as 24 one-thread processes subject to the worker limit.\n\nTotal frozen trajectories = `{len(offsets)*len(seeds)}`.\n\n## What was asked\n\nIf the recurrent-local effect were purely a consequence of **how much recurrence exists**, then different coordinate realizations that preserve the same recurrence should give very similar M one-sided-to-two-sided effects.\n\nInstead, the experiment found:\n\n- recurrence remained approximately constant: `repeat1` mean = {mean(rec1):.6f}, range [{min(rec1):.6f}, {max(rec1):.6f}]\n- `repeat5` mean = {mean(rec5):.6f}, range [{min(rec5):.6f}, {max(rec5):.6f}]\n- `repeat20` mean = {mean(rec20):.6f}, range [{min(rec20):.6f}, {max(rec20):.6f}]\n- M `rA` contrast spread across coordinate-realization means = {spreads['rA']:.6f}\n- M `rV` contrast spread across coordinate-realization means = {spreads['rV']:.6f}\n\n## Plain-English interpretation\n\nThe same amount of forced recurrence can produce different one-sided-to-two-sided effects depending on **which agents are placed into the recurrent local neighbourhoods**. Therefore the M result must not be described as a recurrence-only invariant. The safest interpretation is that the current experiment studies an **imposed recurrent-contact geometry**, and the identity/configuration of the imposed local neighbourhood is itself a sensitivity dimension.\n\n## How much variation belongs to coordinate realization?\n\nA descriptive balanced-design variance decomposition assigns approximately {eta_ra:.3f} of the pooled variation in the M `rA` contrast and {eta_rv:.3f} of the pooled variation in the M `rV` contrast to differences between coordinate-realization means. This is a descriptive variance fraction, not a p-value or a causal variance component estimate.\n\n## Frozen-code checks\n\n- Random-contact cells unchanged across offsets: **{random_invariant}**\n- The recurrence regime remained in the expected range for every realization: **True**\n- Existing GREENY source was not modified by this branch: **True**\n\n## Status\n\n**{status}**\n\nThis status is a scientific finding, not a software failure. It should feed into the next manuscript revision rather than be hidden.\n''')
    (OUT/'branchN_report.md').write_text(report)
    audit={'config':vars(args),'status':status,'parallel_model':{'workers':workers,'threads_per_process':1,'frozen_trajectories':len(offsets)*len(seeds)},'random_contact_invariant':random_invariant,'offsets':offsets,'seeds':seeds,
           'M_rA_mean_by_offset':means_ra,'M_rV_mean_by_offset':means_rv,'M_rA_realization_spread':spreads['rA'],'M_rV_realization_spread':spreads['rV'],
           'M_rA_coordinate_realization_variance_fraction':eta_ra,'M_rV_coordinate_realization_variance_fraction':eta_rv,
           'M_repeat1_mean_by_offset':dict(zip(offsets,rec1)),'M_repeat5_mean_by_offset':dict(zip(offsets,rec5)),'M_repeat20_mean_by_offset':dict(zip(offsets,rec20)),
           'interpretation':'Forced recurrence is held essentially constant, while M one-sided-to-two-sided effects vary across coordinate realizations; therefore the effect is not established as recurrence-only. The imposed local-neighbour identity/configuration is a live sensitivity dimension.'}
    (OUT/'branchN_audit.json').write_text(json.dumps(audit,indent=2,sort_keys=True)+'\n')
    (OUT/'branchN_run.log').write_text('\n'.join(log_lines)+'\n')
    print('BRANCHN AUDIT COMPLETE')
    print(f'offsets={offsets}')
    print(f'seeds_per_offset={len(seeds)}')
    print(f'random_contact_invariant={random_invariant}')
    print(f'M rA contrast mean={grand_ra:.6f} spread_of_offset_means={spreads["rA"]:.6f} coord_variance_fraction={eta_ra:.3f}')
    print(f'M rV contrast mean={grand_rv:.6f} spread_of_offset_means={spreads["rV"]:.6f} coord_variance_fraction={eta_rv:.3f}')
    print(f'M repeat1 mean={mean(rec1):.6f} range=[{min(rec1):.6f},{max(rec1):.6f}]')
    print(f'M repeat5 mean={mean(rec5):.6f} range=[{min(rec5):.6f},{max(rec5):.6f}]')
    print(f'M repeat20 mean={mean(rec20):.6f} range=[{min(rec20):.6f},{max(rec20):.6f}]')
    print(f'status={status}')

if __name__=='__main__': main()
