#!/usr/bin/env python3
"""Fast tests of the frozen M graph reconstruction; does not need BranchN raw outputs."""
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[4]
BIN=ROOT/'branchN/extensions/temporal_exposure_network/code/contact_graph_probe'

def get(seed=101,offset=0,steps=12000,n=4000,fam=1000):
    p=subprocess.run([str(BIN),str(seed),str(offset),str(steps),str(n),str(fam)],cwd=ROOT,text=True,capture_output=True)
    assert p.returncode==0,p.stderr
    vals=p.stdout.strip().split(',')
    return {"seed":int(vals[0]),"offset":int(vals[1]),"N":int(vals[2]),"steps":int(vals[3]),"families":int(vals[4]),
            "parity_even":int(vals[5]),"parity_odd":int(vals[6]),"n_edges":int(vals[7]),"density":float(vals[8]),"components":int(vals[9]),
            "degree_mean":float(vals[10]),"degree_sd":float(vals[11]),"strength_mean":float(vals[12]),"strength_sd":float(vals[13]),
            "edge_weight_min":float(vals[14]),"edge_weight_max":float(vals[15]),"edge_weight_mean":float(vals[16]),"edge_weight_cv":float(vals[17]),
            "global_clustering":float(vals[18]),"A_assort_unweighted":float(vals[19]),"V_assort_unweighted":float(vals[20]),
            "A_assort_weighted":float(vals[21]),"V_assort_weighted":float(vals[22]),"mean_abs_dA":float(vals[23]),"mean_abs_dV":float(vals[24]),
            "weighted_abs_dA":float(vals[25]),"weighted_abs_dV":float(vals[26])}

if __name__=='__main__':
    r=get()
    assert r['n_edges']==r['N']
    assert r['components']==1
    assert abs(r['degree_mean']-2.0)<1e-12
    assert r['degree_sd']<1e-12
    assert abs(r['global_clustering'])<1e-15
    assert abs(r['strength_mean']-r['steps'])<1e-12
    print('M_CYCLE_INVARIANTS PASS')
    print('n_edges=',r['n_edges'],'degree=',r['degree_mean'],'components=',r['components'],'clustering=',r['global_clustering'])
    print('strength_mean=',r['strength_mean'],'edge_weight_range=',r['edge_weight_min'],r['edge_weight_max'])
