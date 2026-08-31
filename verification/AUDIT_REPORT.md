# GREENY recursive reproducibility and residual audit

## Executive status

The final package separates the manuscript, source code, authoritative 56-seed results, rerunnable executable, and audit evidence. The central 224-row network matrix is retained as the numerical authority. The seed is the inferential unit; agents, encounters, and time points inside a seed are not treated as independent replicates.

## Recursive audit result

The reader-facing model specification now introduces the computational objects before the equations: latent traits, evolving state, encounter, target, mismatch, strain, threshold, episode, dose, and final observable. Encounter semantics distinguish a common pre-event snapshot from the calculation and commit stages. The one-sided, simultaneous two-sided, and sequential two-sided regimes are therefore defined operationally rather than by informal language.

The provenance chain distinguishes published anchors, transformations, inherited constants, and modelling choices. The 500-step shock interval and the 30-step observation window are explicitly treated as modelling conventions. Inheritance is provenance only; it is not treated as independent validation.

## Arithmetic reconstruction

The authoritative 56-seed matrix contains 224 condition-by-seed rows. Cell means and the 2×2 interaction are reconstructed directly from seed-level values. The maximum checked arithmetic residual is 4.875e-7, below the displayed six-decimal precision of the headline table.

The 2×2 interaction is recomputed as `(M-Y - M-D) - (R-Y - R-D)`, rather than as a single cell difference. The final source implements that definition explicitly. This corrects an earlier output-writer ambiguity in the extension source; the archived 56-seed result itself is independently reconstructed from the raw matrix.

## Implementation controls

Completed checks include compiler warnings promoted to errors, deterministic perfect-matching tests, common-ledger equality between matched interaction modes, a metric/attachment independence sanity check, finite-output guards, one-thread versus multi-thread reproducibility checks, and an AddressSanitizer/UndefinedBehaviorSanitizer smoke run. The package also contains explicit source checks for the common-ledger contract, correct interaction calculation, clock overrides, and metric-strength bounds.

## Clock audit

Four-seed fixed-N checks were executed for 15-step and 60-step observation windows, a 250-step shock interval with a 30-step observation window, and a 1000-step shock interval with a 30-step observation window. The 15-, 60-, and 250-step checks preserved a negative directed-to-two-sided contrast. The 1000-step check produced a near-zero random-contact contrast and a small positive recurrent-local contrast. The clock is therefore not treated as a universal invariant, and the primary inference remains conditional on the released 500-step shock convention.

## Definitive versus exploratory evidence

The released 56-seed outputs are the definitive numerical basis for the headline estimates. The fresh 56-seed C++ rerun was attempted in this environment but did not complete within the available execution window; no fresh full-run completion is claimed. The archived matrix was instead reconstructed directly. The sequential-reciprocal and anxiety-assortative boundary results remain archived manuscript results unless a corresponding seed-level source export is supplied.

## Package-level rerun

The whole package can be re-audited with:

```bash
cd GREENY_2X2_NETWORK_EXPERIMENT_2026-08-29
make verify
make sanitizer
THREADS=15 SEEDS=56 make full
python3 verification/reconstruct_claims.py
```
