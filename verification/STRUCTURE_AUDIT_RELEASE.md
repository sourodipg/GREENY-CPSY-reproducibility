# Targeted structure audit release

This release adds a small, high-signal audit layer without changing the manuscript or the default scientific model.

## What was added

- `code/network_kernels.hpp` centralizes the exact contact/selection kernels used by the executable.
- `tests/network_structure_audit.cpp` checks one-sided designation balance, latent-rank invariance under `z -> z^3`, and direct recurrence produced by the metric pairing rule.
- `tests/test_network_structure.py` compiles and runs the structure audit under strict warnings-as-errors.
- `verification/coordinate_realization_audit.py` changes only the coordinate RNG realization and compares the resulting metric D-to-Y contrasts while requiring random-contact cells to remain unchanged.
- `verification/run_structural_audit.sh` provides a one-command structural audit wrapper.

## Interpretation

The recurrent-local condition is an imposed exposure process. High `repeat1`, `repeat5`, and `repeat20` are properties of the contact generator and must not be interpreted as naturally emerging attachment or voluntary relationship formation.

The coordinate audit separates the arbitrary numerical representation of the coordinate from its rank ordering. A strictly increasing transform must leave the metric ledger unchanged. A different coordinate realization may change which agents occupy the local neighbourhoods; such changes are reported as a sensitivity quantity rather than silently absorbed into the main claim.

## Recommended commands

Fast:

```bash
make verify
```

Targeted coordinate realization sensitivity:

```bash
python3 verification/coordinate_realization_audit.py
```

Quick version:

```bash
python3 verification/coordinate_realization_audit.py --seeds 2 --threads 2 --families 20 --steps 400 --n 80 --offsets 0 1 2
```

Full production rerun, when desired:

```bash
THREADS=$(nproc) SEEDS=56 make full
```
