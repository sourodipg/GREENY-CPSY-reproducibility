# Test matrix

## T1: probe compilation

Compiles the contact-graph probe from the frozen source with `-Wall -Wextra -Wpedantic -Werror -fopenmp`.

## T2: exact M-cycle invariant

For even `N`, checks that the aggregate M graph has exactly `N` distinct undirected edges, one connected component, mean degree 2, zero degree variance, zero binary triangle clustering, and mean node strength equal to `steps`.

## T3: frozen-kernel parity accounting

Checks the two parity counts sum to the number of timesteps. These counts determine the exact edge weights of the two alternating perfect matchings.

## T4: upstream BranchN linkage

Requires the 24 coordinate-realization × 12 seed BranchN raw result directories and reconstructs the M `Y-D` effects from them. The network extension does not alter those results.

## T5: attribute mixing

Computes Newman-style scalar assortativity over the binary M exposure graph, plus a separately-labelled exposure-weighted endpoint Pearson sensitivity using edge weights as contact counts.

## T6: exposure similarity

Computes unweighted and exposure-weighted mean trait distances across M exposure edges for anxiety and avoidance.

## T7: outcome linkage

Correlates 24 coordinate-realization means of standard graph metrics with the corresponding BranchN M `Y-D` effect. This is exploratory and is never reported as a confirmatory p-value.

## T8: package integrity

Runs the frozen package preflight and self-check after the extension has completed.
