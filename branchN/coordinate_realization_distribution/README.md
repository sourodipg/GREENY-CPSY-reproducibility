# BranchN — Coordinate-Realization Distribution Audit

This branch is an additive experiment. All pre-existing GREENY source files are frozen.
No manuscript files are modified by this branch.

## Question
Does the M (recurrent-local) one-sided-to-two-sided contrast depend materially on which agents happen to occupy the imposed local neighbourhoods, even when recurrence strength is held essentially constant?

## Design
- Frozen production executable: `../../bin/symmetric_network_matrix`
- Population, forcing, model parameters: unchanged
- Only `coordinate_seed_offset` changes
- Default: 24 coordinate realizations (`0..23`) × 12 matched stochastic seeds
- N=4000, 1000 families, 12000 steps, metric strength 1, shock interval 500, observation window 30 steps
- Random-contact cells are required to be identical across coordinate realizations.
- M recurrence diagnostics are recorded for every realization.
- Primary BranchN quantities are the per-seed M(Y-D) contrasts in rA and rV.

## Interpretation rule
This branch does not try to rescue or reject the M effect with an arbitrary p-value.
It estimates how much the effect moves when only the imposed local neighbourhood realization changes.
A large spread means that the M result is a live coordinate-realization sensitivity, not a pure recurrence-only invariant.

## Run
From the package root:

```bash
bash branchN/coordinate_realization_distribution/scripts/run_branchN.sh
```

Default full run:

```bash
python3 branchN/coordinate_realization_distribution/scripts/run_branchN.py --seeds 12 --offsets 0 1 2 3 ... 23
```

The script uses temporary working directories so that frozen `results/` files are not overwritten.

## Outputs
- `results/raw/offset_XX/matrix_per_seed.csv` — frozen-executable output for each realization
- `results/branchN_summary.csv` — number-aware summary
- `results/branchN_by_offset.csv` — per-realization means and recurrence diagnostics
- `results/branchN_by_seed.csv` — per-seed contrasts across coordinate realizations
- `results/branchN_audit.json` — machine-readable audit
- `results/branchN_report.md` — noob-proof prose interpretation
- `results/branchN_run.log` — complete terminal log
