# GREENY: five-minute reproduction path

1. Install a C++17 compiler with OpenMP and Python 3.10+.
2. Clone the repository.
3. Run `python3 -m pip install -r requirements.txt`.
4. Run `make verify`.
5. Read `docs/EXPERIMENT_CONTRACT.md`.
6. Use `make full` for the main simulation.
7. Use the BranchN and interaction-fork scripts for the extended analyses.

The released tables under `data/` are the machine-readable records used for the reported analyses. New local outputs are written to `results/` and are not automatically committed.
