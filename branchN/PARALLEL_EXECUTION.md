# BranchN parallel execution

## What is executed

The default experiment contains 24 coordinate realizations x 12 frozen GREENY seeds = 288 frozen GREENY trajectories.

Each coordinate realization is executed by one invocation of the frozen `bin/symmetric_network_matrix` executable with:

- `seeds=12`
- `threads=1`
- production `N=4000`
- `families=1000`
- `steps=12000`

Therefore each frozen GREENY trajectory is evaluated by a single OpenMP thread. The 24 coordinate-realization processes are scheduled concurrently with a bounded worker pool.

## Why not 288 processes?

The frozen executable accepts a seed count but does not expose a start-seed argument. A 12-seed invocation therefore evaluates the exact frozen seed sequence in one process while retaining one OpenMP thread. Splitting into 288 separate processes would require changing or wrapping the frozen seed semantics, which would be a less clean reproducibility boundary.

## Default scheduling

`--workers` defaults to the number of available logical CPUs, capped by the 24 coordinate realizations. A machine with 32 logical CPUs therefore runs 24 one-thread processes concurrently; a machine with 16 logical CPUs runs 16 concurrently and queues the rest.

## Frozen boundary

Only files under `branchN/` are modified by the BranchN runner. The production GREENY source, executable, manuscript, archived results and reference fixtures remain unchanged.
