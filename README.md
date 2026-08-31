# GREENY: Reproducible Computational-Model Interrogation

This repository contains the GREENY generative model, reproducibility tests, the BranchN structural-sensitivity analyses, the interaction-disambiguation controls, the manuscript and supplement, and machine-readable result files.

The repository is designed to let a reader move from the scientific question to the executable model, then from the executable model to the reported numbers. Generated binaries are intentionally not committed; they are built locally from source. The repository records the parent model core used by the production sources and verifies its recorded checksums.

## What this repository establishes

GREENY is a controlled generative system for interrogating where a model-generated association comes from. The analysis separates four layers:

1. measurement construction;
2. state/trajectory dynamics;
3. imposed temporal contact exposure;
4. dyadic update rules.

The recurrent-local condition is an imposed contact process, not an empirical or emergent social network. The model’s attachment terminology identifies the conceptual provenance of two simulated coordinates; it does not turn them into clinical measurements.

## Quick start

Requirements: a C++17 compiler, OpenMP support, Python 3.10+ and the Python packages in `requirements.txt`.

```bash
git clone https://github.com/sourodipg/GREENY-CPSY-reproducibility.git
cd GREENY-CPSY-reproducibility
python3 -m pip install -r requirements.txt
make verify
```

A successful `make verify` performs the structural network audit, compiles and exercises GREENY, checks the four-cell contract, checks common encounter ledgers, reconstructs the interaction algebra, and runs the independent reference-manifest check.

## Reproduce the main simulation

The default production configuration is recorded in `docs/EXPERIMENT_CONTRACT.md` and the root `Makefile`.

```bash
make full
```

The main output is written locally to `results/` and is deliberately git-ignored. Authoritative released tables used for the manuscript are retained under `data/`.

## BranchN structural sensitivity

BranchN asks whether the recurrent-local result is invariant to the exogenous coordinate realization while recurrence is held fixed.

```bash
bash branchN/coordinate_realization_distribution/scripts/run_branchN.sh
```

The production design used 24 coordinate realizations × 12 seeds = 288 trajectories. The latest authoritative summaries are retained under `data/branchN/`.

## Interaction disambiguation

The interaction fork contains three counterfactual controls in addition to the original directed and two-sided conditions:

- `D_SWAP`: exchange the one-sided recipient while holding the encounter ledger fixed;
- `Y_ABLATE`: retain two-sided updating but remove the partner-state contribution from the target numerator;
- `Y_HALFALPHA`: retain two-sided updating while halving the per-update assimilation fraction.

The production run used 56 seeds, two contact regimes and five interaction conditions = 560 seed-condition rows.

```bash
bash branchN/forks/interaction_disambiguation/scripts/run_interaction_fork.sh
```

The latest released results are in `data/interaction_disambiguation/`.

## Network analysis

The temporal/exposure network extension reconstructs the imposed contact sequence and its weighted aggregate exposure representation. The aggregate recurrent-local graph is a cycle on the coordinate-ranked agents; this is a property of the matching construction, not a claim about human social-network topology.

See:

- `branchN/extensions/temporal_exposure_network/docs/LITERATURE_NETWORK_METHODS.md`
- `branchN/extensions/temporal_exposure_network/docs/METHODS.md`
- `data/network/`

## Associated manuscript

The manuscript is maintained separately from this code/data repository. This repository is the reproducibility and analysis archive referred to by the manuscript. The paper is intentionally not duplicated here.

When the paper is publicly available, add its DOI or journal URL to this section and to `CITATION.cff` if appropriate.

## Results and statistical unit

The inferential replicate is the **simulation seed**. Agents, contacts, repeated encounters and time points are not treated as independent inferential replicates. BranchN coordinate realizations are a structural-sensitivity dimension, not 288 independent social-network observations.

For the BranchN reports, `spread` means the max–min range of the coordinate-realization means:

`spread = max(realization mean) - min(realization mean)`.

It is not a standard deviation, confidence interval or p-value.

## Reproducibility and provenance

The repository separates:

- model source;
- curated parent-core provenance;
- authoritative results;
- audit scripts and test fixtures;
- manuscript/supplement;
- documentation.

Source hashes are recorded in `provenance/`. The repository does not treat a Git branch as a permanent scientific identifier. For citation, use the versioned release/DOI once archived.

## Citation

The repository includes `CITATION.cff`. After creating the public GitHub repository, archive the intended release with Zenodo and insert the resulting DOI into the paper’s Data Accessibility Statement. GitHub documents the GitHub→Zenodo workflow and recommends a license and release-based archival identifier for stable citation.

## License

Code is released under the MIT License in `LICENSE-CODE`. Documentation and the supplied manuscript/data materials are released under CC BY 4.0 in `LICENSE-DOCS-DATA`, subject to the author's ownership and any later publisher-specific terms.

## Reproducibility boundary

A passing software test verifies the assertion encoded by that test. It does not prove that GREENY is a valid clinical model, that the simulated attachment coordinates are clinical measurements, or that the synthetic contact process is an empirically realistic social network.
