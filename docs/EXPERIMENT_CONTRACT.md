# Experiment contract — GREENY 2×2 network extension

## Purpose
Test two orthogonal structural levers around the frozen GREENY dynamics:

1. **interaction reciprocity:** directed/non-dyadic versus dyadic/reciprocal updates;
2. **contact mixing:** uniform random pairing versus metric-local pairing.

## Four primary cells

- R-D: random pair matching, one receiver updates;
- R-Y: random pair matching, both participants update;
- M-D: metric-local pair matching, one receiver updates;
- M-Y: metric-local pair matching, both participants update.

The same pair ledger is used for R-D/R-Y within each seed, and separately the same pair ledger is used for M-D/M-Y. Thus the dyadic contrast is a paired intervention on the update rule, not on who met whom.

## Metric construction

Each agent receives a fixed latent social coordinate z_i from a dedicated RNG stream independent of attachment. Metric-local contact pairs adjacent points on the unit circle after a deterministic random phase shift. `metric_strength` is a per-timestep probability of using this metric-local pairing instead of uniform random pairing; 0 is exactly random mixing and 1 is fully metric-local.

This is an exogenous network stress-test, not a claim that attachment itself creates social distance.

## State-update rule

Both participant states in the dyadic cell are computed from the common pre-event state at the start of the timestep and committed together. The directed cell updates only the designated receiver; the other participant retains its pre-event state.

## Primary observables

Global: r_A, r_V, episode rate.

Local: pre/post pair-state correlations, mutual elevation, repeat-partner probabilities at lags 1/5/20.

Network: same-family contact fraction, mean metric distance, unique-partner fraction, temporal degree CV.

## Statistical rule

Seed-level outputs are primary. Paired contrasts use the same seeds. The sign-flip permutation p-value is deterministic (20,000 sign flips from a fixed RNG seed) and is supplementary to the confidence interval; it is not a substitute for a pre-specified scientific effect size.
