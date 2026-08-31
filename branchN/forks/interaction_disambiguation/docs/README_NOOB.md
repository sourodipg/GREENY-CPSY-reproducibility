# Interaction-disambiguation fork — noob guide

This is an additive BranchN fork. The parent GREENY files are not edited.

## What is already in the package

- Parent GREENY unit and structure checks.
- Corrected ratio/construction audit and numerical residual reconstruction.
- Main 56-seed matched network experiment: random and recurrent-local contact; one-sided and two-sided updating.
- Simultaneous-versus-sequential two-sided control.
- Anxiety-assortative boundary condition.
- Local 10% parameter sign-stability runs.
- BranchN coordinate-realization sensitivity: 24 coordinate realizations x 12 seeds = 288 trajectories.
- Neighborhood trait-composition audit.
- Literature-grounded temporal/exposure-network audit.
- This interaction-disambiguation fork.

## What the new fork adds

### D_SWAP — recipient-swapped one-sided updating

For the exact same encounter `(i,j)`, if the ordinary one-sided condition updates `i`, the fork updates `j` instead. No new encounter, forcing history, population draw or random recipient draw is introduced.

Question: **does recipient designation itself change the result?**

### Y_ABLATE — two-sided partner-signal ablation

Both participants still update, but the partner-state term in the target numerator is set to zero. The original denominator is retained. This isolates the partner-signal channel rather than changing the relationship-weight normalization.

Question: **how much of the two-sided result depends on actual cross-partner state coupling?**

### Y_HALFALPHA — two-sided update with half assimilation fraction

Both participants update using the ordinary two-sided target, but the per-update assimilation fraction is multiplied by 0.5.

Question: **does reduced update intensity make the two-sided result resemble one-sided updating?**

This is a targeted dose/intensity control, not a claim that all meanings of exposure dose are perfectly matched.

## Primary execution

From the package root:

```bash
bash branchN/forks/interaction_disambiguation/scripts/run_interaction_fork.sh
```

Defaults are 56 seeds, 32 OpenMP threads, 1000 families, 12000 steps, N=4000, recurrent-local contact strength 1, coordinate offset 0.

The script compiles the separate fork executable, runs all five interaction conditions under both random and recurrent-local contact, verifies identical encounter-ledger digests within each seed/contact regime, then runs the self-aware diagnostic.

## Output files

- `results/interaction_disambiguation_per_seed.csv`: every seed x condition result.
- `results/interaction_disambiguation_contrasts.csv`: paired contrasts with mean, SD, SE, 95% interval, sign-flip p and d_z.
- `results/interaction_disambiguation_summary.csv`: condition means.
- `results/interaction_disambiguation_run.log`: complete console output.

## Reading the result

The diagnostic prints conditional prose after the numbers. In particular:

- If the D_SWAP interval for `rA` crosses zero, there is no clear systematic recipient-role difference in that outcome.
- `Y_ABLATE - Y` measures the change caused by removing the partner-state signal while retaining two-sided updating.
- `Y_HALFALPHA - D` and `Y_HALFALPHA - Y` show how sensitive the result is to update magnitude.

The code does not declare a mechanistic victory from a single correlation. It reports what each counterfactual establishes and what remains unresolved.
