# GREENY final audit plan

This release follows a recursive audit sequence:

1. **Instantiate the world** — define agents, fixed latent coordinates, evolving state, encounter, target, mismatch, strain, threshold, episode, dose, and final observable before interpretation.
2. **Trace every input** — classify quantities as published anchors, transformed anchors, inherited constants, modelling choices, or safeguards; inheritance is provenance rather than validation.
3. **Formalize encounter semantics** — distinguish pre-event snapshot, compute, and commit; define one-sided, simultaneous two-sided, and sequential two-sided updates precisely.
4. **Audit the network manipulation** — verify deterministic matching, trait independence of the local coordinate, recurrence, distance, partner concentration, and matched-ledger identity.
5. **Audit arbitrary clocks** — perturb the shock interval and observation window and record whether the qualitative result survives.
6. **Reconstruct every headline statistic** — derive cell means, paired contrasts, 2×2 interactions, recurrence benchmarks, and reported residuals directly from replicate-level data.
7. **Run software controls** — compiler warnings, deterministic tests, one-versus-multi-thread reproduction, sanitizer smoke test, finite-value guards, and package self-check.
8. **Render and inspect** — compile the manuscript, render pages, inspect equations/figures/appendices, then rerun the reconstruction after any edit.

The final conclusion remains intentionally conditional on the frozen parameter point and the 500-step shock-clock convention. Exploratory clock checks are retained as evidence against treating the clock as an invariant.
