# BranchN noob-proof reading guide

## The one question

The M condition puts agents into a fixed local neighbourhood using an exogenous coordinate `z`.
The question is whether the observed M one-sided-to-two-sided effect is really a consequence of **recurrence itself**, or whether it also depends on **which agents happen to be neighbours** in that imposed geometry.

## What stays fixed

Population traits, intelligence, family weights, forcing, model parameters, N, family count, number of steps, the M pairing algorithm, and the random-contact dynamics stay fixed.

Only the seed used for the exogenous coordinate `z` changes.

## The three things to read

1. `repeat1`, `repeat5`, `repeat20` = how strongly the contact cage recycles partners. These should remain nearly unchanged across coordinate realizations.

2. `M_DY_minus_D_rA` = rA under two-sided minus rA under one-sided, within the M contact regime.
   Example: -0.06 means two-sided updating lowers rA by about 0.06 in that realization.

3. `M_DY_minus_D_rV` = the same idea for rV.

## What would support “recurrence alone”

If recurrence is the main explanation, the recurrence numbers should stay fixed and the M contrasts should barely move when only the coordinate realization changes.

## What would refute that narrow claim

If recurrence stays fixed but the M contrasts move substantially, then the effect is not recurrence-only. The imposed neighbourhood identity/configuration is itself a sensitivity dimension.

## Important distinction

A high `repeat20` is not evidence that agents naturally formed attachment. It is a direct diagnostic of the externally imposed contact rule. The state-coupling mechanism comes from the interaction equation that lets one partner's state influence the other's update.

## Why the rank test matters

If replacing `z` by a strictly increasing transform such as `z^3` leaves the contact ledger unchanged, the numeric scale of `z` is irrelevant. The experiment then depends on the **ordering induced by z**, not on the arbitrary values themselves.

## Why the coordinate-realization test matters

Changing the coordinate realization changes the ordering and therefore which agents become neighbours, while leaving the recurrence algorithm itself unchanged. This is the cleanest low-cost attack on the remaining question: “is the M effect tied to this particular imposed neighbourhood assignment?”
