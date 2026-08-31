# Network-structure audit

This audit targets the hidden implementation choices raised during review.

## One-sided recipient selection
The D condition designates one member of each pair through `directed_recipient(seed,t,i,j)`. The selector is deterministic for a fixed seed, encounter time and pair, while its low bit is balanced across a large audit set. The audit reports the empirical left/right designation fraction.

## Latent coordinate
Each agent receives one fixed exogenous coordinate

`z_i ~ Uniform(0,1)`

from a dedicated RNG stream. The metric-local matching code uses only the sorted order of these coordinates. Therefore any strictly increasing transform, such as `z^3`, must induce the same rank order and the same contact ledger.

## Forced recurrence
Metric-local matching is an imposed contact rule. Neighbouring ranks on the fixed coordinate circle are repeatedly paired; recurrence is therefore a property of the contact generator, not an emergent statement about voluntary relationship choice.

The direct structural audit checks repeat-partner probabilities without running the dynamical model. The expected pattern is approximately 0.5 at lag 1, >0.95 within five encounters, and >0.99 within twenty encounters.

## Coordinate-realization sensitivity
`verification/coordinate_realization_audit.py` changes only the seed used for the exogenous coordinate while holding model/forcing seeds fixed. Random-contact cells must remain exactly unchanged. Metric-contact cells may move because the imposed local neighbourhoods change; the result is recorded descriptively rather than labelled a failure.

This test is deliberately separate from the main claim-reconstruction suite because it asks a different question: how much of the metric-local result depends on the particular realization of an otherwise trait-independent exogenous coordinate?
