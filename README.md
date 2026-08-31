# GREENY: Reproducible Computational-Model Interrogation

This repository contains the GREENY generative model, reproducibility tests, the BranchN structural-sensitivity analyses, the interaction-disambiguation controls, and machine-readable result files.

The repository is designed to let a reader move from the scientific question to the executable model, then from the executable model to the reported numbers. Generated binaries are intentionally not committed; they are built locally from source. The repository records the parent model core used by the production sources and verifies its recorded checksums.

## What this repository establishes

GREENY is a controlled generative system for interrogating where a model-generated association comes from. The analysis separates four layers:

1. measurement construction;
2. state/trajectory dynamics;
3. imposed temporal contact exposure;
4. dyadic update rules.

The recurrent-local condition is an imposed contact process, not an empirical or emergent social network. The model's attachment terminology identifies the conceptual provenance of two simulated coordinates; it does not turn them into clinical measurements.

## Quick start

Requirements: a C++17 compiler, OpenMP support, Python 3.10+ and the Python packages in `requirements.txt`.

```bash
git clone https://github.com/sourodipg/GREENY-CPSY-reproducibility.git
cd GREENY-CPSY-reproducibility
python3 -m pip install -r requirements.txt
make verify
