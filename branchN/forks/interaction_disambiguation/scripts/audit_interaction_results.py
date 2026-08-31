#!/usr/bin/env python3
"""Self-aware, noob-readable validator for the interaction-disambiguation fork.

The script does not decide whether a scientific effect is "true". It checks that the
counterfactual simulations are structurally valid, then prints conditional interpretations
for the observed contrasts so the numbers are auditable without reverse-engineering symbols.
"""
from __future__ import annotations
import csv, math, pathlib, statistics

ROOT = pathlib.Path(__file__).resolve().parents[1]
R = ROOT / "results"
RAW = R / "interaction_disambiguation_per_seed.csv"
CON = R / "interaction_disambiguation_contrasts.csv"

EXPECTED = {"D", "Y", "D_SWAP", "Y_ABLATE", "Y_HALFALPHA"}

def say(ok: bool, label: str, detail: str) -> None:
    print(f"{'PASS' if ok else 'FAIL'} | {label} | {detail}")
    if not ok:
        raise SystemExit(2)

def load_csv(path: pathlib.Path):
    with path.open(newline="") as f:
        return list(csv.DictReader(f))

def finite_rows(rows):
    numeric = ["rA","rV","rate","pair_x_corr_post","pair_q_corr_post","repeat1","repeat5","repeat20"]
    for r in rows:
        for k in numeric:
            x=float(r[k])
            if not math.isfinite(x):
                return False, f"non-finite {k} in seed={r['seed']} mixing={r['mixing']} condition={r['condition']}"
    return True, "all reported numerical outputs are finite"

def summarize_contrast(rows, name):
    vals=[]
    for r in rows:
        if r["contrast"]==name and r["metric"]=="rA": vals.append(float(r["mean_diff"]))
    return vals[0] if vals else None

def main():
    if not RAW.exists() or not CON.exists():
        say(False,"RESULT FILES","run the interaction fork first")
    rows=load_csv(RAW); contrasts=load_csv(CON)
    say(len(rows) > 0,"OUTPUT PRESENT",f"{len(rows)} seed-condition rows found")
    ok, detail=finite_rows(rows); say(ok,"FINITE OUTPUTS",detail)
    seeds=sorted({r["seed"] for r in rows}); mixes=sorted({r["mixing"] for r in rows})
    say(set(r["condition"] for r in rows)==EXPECTED,"CONDITIONS PRESENT","D, Y, D_SWAP, Y_ABLATE and Y_HALFALPHA are present")
    expected=len(seeds)*len(mixes)*len(EXPECTED)
    say(len(rows)==expected,"ROW COUNT",f"{len(rows)} rows = {len(seeds)} seeds x {len(mixes)} contact regimes x 5 conditions")
    # Same-contact-ledger check: digest is a direct event-sequence checksum.
    groups={}
    for r in rows: groups.setdefault((r["seed"],r["mixing"]),set()).add(r["pair_digest"])
    bad=[g for g,v in groups.items() if len(v)!=1]
    say(not bad,"ENCOUNTER LEDGER EQUALITY", "all interaction conditions share the same pair digest within seed and contact regime")
    # Recurrence is a property of the contact sequence, but the per-condition counters
    # count incoming updates and therefore should not be compared across D and Y as if they
    # were the same statistic. The encounter-ledger digest is the invariant check here.
    meta_ok = all(
        (r["condition"] in {"D","D_SWAP"} and r["recipient_rule"] in {"original_one_sided","swapped_one_sided"})
        or (r["condition"] in {"Y","Y_ABLATE","Y_HALFALPHA"} and r["recipient_rule"]=="both_update")
        for r in rows
    )
    say(meta_ok,"CONDITION SEMANTICS","recipient and update-rule labels match the declared counterfactuals")
    yhalf_ok = all((r["condition"]!="Y_HALFALPHA") or abs(float(r["alpha_scale"])-0.5)<1e-12 for r in rows)
    yabl_ok = all((r["condition"]!="Y_ABLATE") or abs(float(r["partner_signal_scale"]))<1e-12 for r in rows)
    say(yhalf_ok,"HALF-ALPHA SETTING","Y_HALFALPHA uses alpha_scale=0.5")
    say(yabl_ok,"PARTNER-SIGNAL ABLATION SETTING","Y_ABLATE uses partner_signal_scale=0 while preserving the original target denominator")

    def contrast(name, metric):
        q=[r for r in contrasts if r["contrast"]==name and r["metric"]==metric]
        if len(q)!=len(mixes): return None
        return q

    print("\n=== CONDITIONAL SCIENTIFIC READOUT ===")
    print("These statements are decision rules for reading the simulation; they are not new statistical tests.")
    for mix in mixes:
        print(f"\nCONTACT REGIME: {mix}")
        for nm in ["D_SWAP_minus_D_rA","Y_ABLATE_minus_Y_rA","Y_HALFALPHA_minus_D_rA","Y_HALFALPHA_minus_Y_rA"]:
            q=[r for r in contrasts if r["mixing"]==mix and r["contrast"]==nm and r["metric"]=="rA"]
            if not q: continue
            q=q[0]; mean=float(q["mean_diff"]); lo=float(q["ci_lo"]); hi=float(q["ci_hi"])
            print(f"{nm}: mean={mean:.7f}; 95% interval=[{lo:.7f}, {hi:.7f}]")
            if nm=="D_SWAP_minus_D_rA":
                if lo <= 0 <= hi: print("  -> interval crosses 0: no clear systematic recipient-role difference in rA.")
                else: print("  -> interval excludes 0: recipient designation shows a systematic rA difference.")
            elif nm=="Y_ABLATE_minus_Y_rA":
                print("  -> this is the partner-signal ablation contrast: it quantifies how much the two-sided result changes when the cross-partner state signal is removed.")
            elif nm=="Y_HALFALPHA_minus_D_rA":
                print("  -> this is the update-intensity comparison: two-sided updating with alpha halved versus ordinary one-sided updating.")
            elif nm=="Y_HALFALPHA_minus_Y_rA":
                print("  -> if this is near 0, halving alpha leaves Y close to Y; if it is large, update magnitude materially changes the result.")
    print("\n=== BOUNDARY CONDITIONS ===")
    print("D_SWAP is a role-exchange replay; it is not a new network.")
    print("Y_ABLATE removes only the partner-state contribution in the target numerator while retaining the original target denominator. Therefore it is a partner-signal ablation, not a redefinition of the social-weight normalization.")
    print("Y_HALFALPHA halves the per-update assimilation fraction; it is a targeted update-intensity control, not a claim that every possible notion of exposure dose is exactly matched.")
    print("No agent, encounter or time point is treated as an independent inferential replicate; the seed remains the replicate.")

if __name__ == '__main__': main()
