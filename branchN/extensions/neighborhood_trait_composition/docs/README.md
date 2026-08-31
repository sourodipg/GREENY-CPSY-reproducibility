# BranchN extension — trait composition of imposed local neighbourhoods

## Question
Does the coordinate-realization sensitivity of the M one-sided-to-two-sided contrast track the **trait composition of the imposed recurrent neighbourhoods**?

## Frozen boundary
No frozen GREENY source file, manuscript, archived result, or production executable is modified. The extension compiles a probe by including the frozen `code/symmetric_network_matrix.cpp` verbatim under a renamed `main`, so population construction, frozen parameters, coordinate generation, and network kernels come from the same source.

## What is measured
For each agent, the M contact kernel uses the two immediate circular neighbours in the fixed rank ordering of the exogenous coordinate `z`. The extension summarizes:

- mean neighbouring anxiety and avoidance;
- local anxiety assortativity: corr(agent A, mean neighbour A);
- local avoidance assortativity: corr(agent V, mean neighbour V);
- cross-trait local alignment A→V and V→A;
- pairwise local trait covariance;
- mean A-V covariance inside the three-agent local neighbourhood;
- mean Euclidean-in-one-dimensional-coordinate neighbour gap.

These are descriptive structure measures, not clinical quantities.

## Main outcome
For each offset and seed:

`M_DY_minus_D_rA = rA(M,Y) - rA(M,D)`

`M_DY_minus_D_rV = rV(M,Y) - rV(M,D)`

Across the 24 coordinate realizations, the extension correlates the offset-level mean trait-composition metrics with the offset-level M contrasts. With only 24 realizations this is exploratory; the resulting correlations are not confirmatory p-values.

## Dependency on BranchN
BranchN must have been run first, producing:

`branchN/coordinate_realization_distribution/results/raw/offset_XXX/matrix_per_seed.csv`

The extension does not rerun GREENY. It reads those frozen BranchN outputs and separately computes the trait-composition descriptors from the exact frozen source.

## Run

Compile only:

```bash
bash branchN/extensions/neighborhood_trait_composition/scripts/run_neighborhood_trait_audit.sh --compile-only
```

Full 24×12 downstream audit after BranchN:

```bash
bash branchN/extensions/neighborhood_trait_composition/scripts/run_neighborhood_trait_audit.sh
```

Outputs:

- `results/neighborhood_traits_by_seed.csv`
- `results/neighborhood_traits_by_offset.csv`
- `results/neighborhood_effect_correlations.csv`
- `results/neighborhood_trait_report.md`

## Interpretation

A strong association means that the identity/composition of the imposed local neighbourhoods is related to the M effect. A weak association means the current summaries do not explain the realization sensitivity, leaving trajectory-level or higher-order structure as the next candidate.

The branch must never be interpreted as evidence that agents naturally form attachments: recurrence is imposed by the matching rule.
