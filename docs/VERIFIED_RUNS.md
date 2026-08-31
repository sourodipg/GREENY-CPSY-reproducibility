# Verified runs in this build environment

- `make verify`: PASS (ledger, symbolic swap equivariance, smoke, finite-output guards, thread reproducibility).
- 12-seed, N=4000, 12000-step preflight at 15 threads: PASS; outputs retained under `reference/preflight_12seed/`.
- 4-seed family-scaling preflight, N=4000, 12000 steps, F={1000,500,250,100}, 8 threads: PASS; outputs retained under `reference/scale_fast_4seed/`.
- 2-seed N=4000/12000-step performance preflight: PASS; runtime about 30 s at 15 threads in this environment before the later memory/performance optimisations.

A definitive 56-seed run is intentionally left to the user's machine; the package is designed to run it via `THREADS=15 SEEDS=56 make full`.
