# Noob guide

Think of the previous BranchN result as putting different people into the same kind of cage.

The cage strength stays the same:
`repeat1`, `repeat5`, and `repeat20` stay essentially fixed.

What changes is **who is next to whom** on the fixed local coordinate.

This extension asks a simple question:

> Do the neighbourhoods that produce a larger M effect contain a systematically different mix of anxiety/avoidance traits?

A few examples:

- `mean_local_A`: typical anxiety of an agent's two imposed neighbours.
- `mean_local_V`: typical avoidance of those neighbours.
- `local_A_assort`: whether anxious agents tend to sit next to other anxious agents in the imposed ordering.
- `local_V_assort`: analogous measure for avoidance.
- `local_A_to_V`: whether anxious agents tend to sit next to neighbours with higher avoidance.
- `local_pair_cov_A`: whether neighbouring anxiety values move together.
- `mean_local_within_cov_AV`: whether anxiety and avoidance are locally associated inside each tiny three-agent neighbourhood.

Then the extension asks whether those numbers move together with the M effect across the 24 coordinate realizations.

This does not prove why the effect changes. It only checks the most obvious explanation first: **different trait composition of the imposed local cages**.
