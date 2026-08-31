# BranchN — noob-proof result

'
This branch changes **one thing only**: which agents receive the fixed local coordinate `z`. The population, traits, family weights, forcing, model parameters, number of agents, number of steps, recurrence algorithm, and random-contact dynamics remain fixed. The only changed input is the seed used to generate the exogenous local coordinate z.


## Execution model

The experiment launches one frozen GREENY process per coordinate realization. Each process uses exactly one OpenMP thread and evaluates the full seed set for that realization. Different coordinate realizations run concurrently, but the frozen GREENY executable itself is not modified. Thus the default experiment evaluates 24 coordinate realizations x 12 frozen GREENY seeds = 288 frozen trajectories, scheduled as 24 one-thread processes subject to the worker limit.

Total frozen trajectories = `4`.

## What was asked

If the recurrent-local effect were purely a consequence of **how much recurrence exists**, then different coordinate realizations that preserve the same recurrence should give very similar M one-sided-to-two-sided effects.

Instead, the experiment found:

- recurrence remained approximately constant: `repeat1` mean = 0.522500, range [0.522500, 0.522500]
- `repeat5` mean = 0.962500, range [0.962500, 0.962500]
- `repeat20` mean = 0.995000, range [0.995000, 0.995000]
- M `rA` contrast spread across coordinate-realization means = 0.017072
- M `rV` contrast spread across coordinate-realization means = 0.022329

## Plain-English interpretation

The same amount of forced recurrence can produce different one-sided-to-two-sided effects depending on **which agents are placed into the recurrent local neighbourhoods**. Therefore the M result must not be described as a recurrence-only invariant. The safest interpretation is that the current experiment studies an **imposed recurrent-contact geometry**, and the identity/configuration of the imposed local neighbourhood is itself a sensitivity dimension.

## How much variation belongs to coordinate realization?

A descriptive balanced-design variance decomposition assigns approximately 0.012 of the pooled variation in the M `rA` contrast and 0.019 of the pooled variation in the M `rV` contrast to differences between coordinate-realization means. This is a descriptive variance fraction, not a p-value or a causal variance component estimate.

## Frozen-code checks

- Random-contact cells unchanged across offsets: **True**
- The recurrence regime remained in the expected range for every realization: **True**
- Existing GREENY source was not modified by this branch: **True**

## Status

**SENSITIVITY_DETECTED**

This status is a scientific finding, not a software failure. It should feed into the next manuscript revision rather than be hidden.
