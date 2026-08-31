# BranchN

BranchN is an additive, frozen-code experiment created after the self-aware audit found coordinate-realization sensitivity in the M condition.

No existing GREENY source, manuscript, archived result, or reference fixture is changed.

The sole new scientific question is:

> With the same forced recurrence strength, how much can the M one-sided-to-two-sided effect move solely because the exogenous local neighbourhood assignment changes?

Default: 24 coordinate realizations × 12 seeds at N=4000, 1000 families, 12000 steps.


## Parallel execution

The default BranchN scheduler evaluates 24 coordinate realizations concurrently up to the available worker limit. Each realization invokes the frozen GREENY executable with `threads=1` and evaluates 12 frozen seeds, so the default experiment contains 24 x 12 = 288 frozen GREENY trajectories. The `--threads` option is retained only for command-line compatibility and is deliberately ignored for scientific execution; each frozen process uses one OpenMP thread. No file outside `branchN/` is modified.

The scheduler is process-parallel, not source-parallel: the frozen production executable remains byte-for-byte unchanged.
