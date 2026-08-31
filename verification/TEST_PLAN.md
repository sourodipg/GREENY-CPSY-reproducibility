# Reproducibility test plan

## Fast checks already executed

- `make verify`
- `make sanitizer`
- 4-seed, 4000-agent, 12000-step baseline matrix at the frozen clock
- 4-seed observation-window checks at 15 and 60 steps
- 4-seed 250-step shock-clock check with a 30-step observation window
- 4-seed 1000-step shock-clock check with a 30-step observation window
- direct reconstruction of the archived 56-seed 224-row matrix
- direct reconstruction of the archived 2×2 interaction estimands
- manuscript first-person scan over scientific prose

## Definitive run

The archived 56-seed matrix is the numerical authority for the submitted headline estimates. A fresh definitive run can be reproduced with:

```bash
THREADS=15 SEEDS=56 make full
```

The command writes `results/matrix_per_seed.csv`, `results/matrix_summary.csv`, and `results/differences.csv`.

## Optional robustness extensions

```bash
THREADS=15 SEEDS=12 MIX_STRENGTH=0.25 make fast
THREADS=15 SEEDS=12 MIX_STRENGTH=0.50 make fast
THREADS=15 SEEDS=12 MIX_STRENGTH=0.75 make fast
THREADS=15 SEEDS=12 SHOCK_INTERVAL=250 SYMPTOM_WINDOW=0.12 make fast
THREADS=15 SEEDS=12 SHOCK_INTERVAL=1000 SYMPTOM_WINDOW=0.03 make fast
```

The clock probes are exploratory. They should not be pooled with the definitive 56-seed inference unless a separate analysis plan is specified.
