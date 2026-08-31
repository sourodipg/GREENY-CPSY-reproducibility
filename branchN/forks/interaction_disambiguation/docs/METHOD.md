# Method: interaction-disambiguation fork

## Scientific purpose

The parent one-sided-to-two-sided intervention changes more than one implementation feature at once: the number of participants updated per encounter changes, the partner pathway is activated for the second participant, and total state-update exposure increases. This fork separates three candidate contributors without changing the encounter sequence.

## Counterfactuals

Let an encounter contain focal agents `i` and `j`. Let `U_i(i,j)` denote the GREENY update of `i` using `j`'s current state. All state variables are dimensionless model quantities and time is measured in discrete simulation steps.

**D (production one-sided):** one designated recipient updates; the complementary participant does not.

**D_SWAP:** the deterministic recipient decision is complemented. If D updates `i`, D_SWAP updates `j`.

**Y:** both participants update from the same pre-encounter state.

**Y_ABLATE:** both participants update, but the partner-state contribution is removed from the target numerator. The original denominator is retained. This is a signal ablation, not a relationship-weight renormalization.

**Y_HALFALPHA:** both participants update with the same two-sided target, but the per-update assimilation fraction is multiplied by 0.5.

## Matching and replication

Within each seed and contact regime, every interaction condition receives the identical encounter sequence. Deterministic pair digests are required to match. The seed is the stochastic replicate. Agents, encounters and time points are not treated as independent inferential observations.

Default full run: N=4000, 1000 families, 12000 steps, 56 seeds, recurrent-local strength 1 and coordinate offset 0; the random-contact conditions are run in the same executable for direct comparison.

## Statistical output

For each paired contrast, the fork reports the mean within-seed difference, sample standard deviation, standard error, 95% normal-approximation interval, deterministic 20,000-draw sign-flip probability, and standardized mean difference `d_z = mean/SD`.

These summaries are descriptive/confirmatory only to the extent already defined by the parent design. No additional multiplicity claim is made by this fork.
