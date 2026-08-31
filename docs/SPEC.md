# Symmetric dyadic network extension — exact specification

## Parent
GREENY remains unchanged. It uses independent directed partner draws and updates the focal agent's state from the partner's previous-step belief.

## Matched encounter ledger
At each timestep, generate a random perfect matching M_t of the N agents (N must be even). Every unordered pair {i,j} occurs once in M_t. The ledger is generated deterministically from `(seed,t)` using a counter-based SplitMix64/Fisher–Yates construction, so it is independent of simulation state, thread count, and condition.

## Matched-directed control
For every {i,j}, orient the edge deterministically from the pair-order bit. Only the receiver is updated. The source is otherwise unchanged by that event. Every agent is receiver exactly once per timestep.

## Symmetric condition
For every {i,j}, both participants are updated. For both updates, the complete old state `prev` is used: neither member can see the other's newly updated state. Thus, for agent i,

S_i = [w_T T + k_i(w_F F_i + w_O x_j)] / [w_T + k_i(w_F+w_O)]

and analogously for j. Each participant then applies the same GREENY health/episode state machine with its own parameters. New participant states are committed simultaneously at the pair level.

The map is swap-equivariant: if participant labels and all participant-local parameters are exchanged, the pair output exchanges. This is a structural symmetry; it does not imply S_i=S_j or x'_i=x'_j.

## High-signal estimands
- paired change in r_A, r_V, odds ratios and episode rate;
- paired change in mean pre-event joint elevation;
- probability of repeated same pair within lag k;
- conditional mutual-elevation probability on repeated pairs;
- pair-state concordance / covariance on repeated encounters;
- degree/recurrence concentration of the encounter graph.

Primary causal contrast: symmetric minus matched-directed, because the encounter ledger is identical. GREENY is contextual, not the counterfactual for reciprocity.
