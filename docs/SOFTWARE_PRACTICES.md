# Software-engineering controls

This experiment uses a small set of high-impact scientific-software practices:

- immutable parent provenance;
- deterministic and isolated RNG streams;
- common-random-number pairing for intervention comparisons;
- explicit CLI defaults and shell-variable initialization before `set -u`;
- fail-fast dimension and finite-value checks;
- unit/invariant tests for perfect matching and symmetry;
- 1-thread versus multi-thread reproducibility checks;
- seed-level raw outputs rather than aggregate-only results;
- sanitizer builds for small cases;
- compiler warnings promoted to errors for the extension source;
- run manifests and source hashes.

These practices are consistent with published reproducibility guidance emphasizing versioning, controlled randomness, environment capture, automated testing, and repeatable benchmark protocols.
