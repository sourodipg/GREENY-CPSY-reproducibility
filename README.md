# GREENY: Reproducible Computational-Model Interrogation

This repository contains the GREENY generative model, the released simulation results, reproducibility tests, the BranchN structural-sensitivity analyses, and the interaction-disambiguation controls used in the associated computational study.

The repository is intended to answer a practical question:

> **Can another researcher build GREENY from source, run the checks, reproduce the released analyses, and trace each reported number back to machine-readable output?**

The repository is a computational companion. The manuscript is maintained separately and is intentionally not duplicated here.

---

## 1. Start here

For a first-time user, use this order:

1. Read this file once.
2. Run the quick verification.
3. Inspect the released data under `data/`.
4. Only then run the heavier production analyses.

The quickest safe path is:

```bash
git clone https://github.com/sourodipg/GREENY-CPSY-reproducibility.git
cd GREENY-CPSY-reproducibility
python3 -m pip install -r requirements.txt
bash scripts/verify_repository.sh
```

For a read-only clone, HTTPS also works:

```bash
git clone https://github.com/sourodipg/GREENY-CPSY-reproducibility.git
```

### What success should look like

`bash scripts/verify_repository.sh` runs the quick software tests and then checks that the released data, citation metadata and licenses are present. The end of a successful run should include:

```text
STRUCTURE AUDIT PASS
LATENT_RANK_INVARIANCE PASS
TEST SUITE PASS
REPOSITORY VERIFICATION PASS
```

If only `make verify` is run, the expected final software-test message is `TEST SUITE PASS`; the repository-level data/file checks are performed by `bash scripts/verify_repository.sh`.

A passing verification means the encoded software assertions passed. It does **not** mean that GREENY is an empirically validated clinical model.

---

## 2. What is in the repository?

The repository is organized by scientific function rather than by the order in which the work was developed.

```text
code/          GREENY production source
parent/        parent model core needed to reproduce the production implementation
data/          released machine-readable results
branchN/       structural-sensitivity and contact-network analyses
tests/         executable software tests
verification/  claim reconstruction, residual checks and audit procedures
reproduce/     reproduction/preflight helpers
reference/     reference manifests and preflight fixtures
provenance/    release lineage, checksums and version information
docs/          model specification, methods and literature notes
```

Generated binaries are not committed. They are compiled locally from source.

---

## 3. The scientific structure in one picture

GREENY is easiest to understand as four linked layers:

```text
measurement construction
        |
        v
state / trajectory dynamics
        |
        v
imposed temporal contact exposure
        |
        v
dyadic update rules
        |
        v
final model observable and association
```

The recurrent-local condition is an **imposed contact process**. It is not an empirically fitted social network and does not represent natural friendship or attachment formation.

The two attachment-related coordinates are model coordinates whose conceptual provenance is connected to attachment measurement literature. Their use in GREENY does not make the simulated values clinical observations.

---

## 4. Before running anything: know what is expensive

There are three different kinds of action in this repository.

### A. Quick verification — start here

```bash
make verify
```

This is intended to finish quickly. It compiles GREENY and exercises the deterministic structural and contract tests. It does **not** launch the full 56-seed production simulation.

### B. Main production reproduction

```bash
THREADS=32 SEEDS=56 make full
```

The default production dimensions are:

```text
N = 4000 agents
families = 1000
steps = 12000
seeds = 56
```

`THREADS=32` is a sensible setting on a 32-core/64-thread Xeon host. The code can use more or fewer threads, but a thread count is a computational setting, not an inferential replicate.

The production run writes newly generated files under `results/`. Those generated outputs are intentionally git-ignored. The released manuscript-analysis tables are already preserved under `data/`.

For a direct comparison against the small recorded reference run, use:

```bash
make verify-reference
```

That command runs the 4-seed preflight and compares the generated files with the repository's reference fixtures.

### C. Extended analyses

BranchN and the interaction-disambiguation fork are heavier. Run them only after `make verify` passes.

---

## 5. Exact reproduction path for the main experiment

The steps below distinguish **checking the repository**, **reproducing the main simulation**, and **rerunning the extended analyses**. A reader does not need to run everything merely to verify the release.

### Step 1 — enter the repository

```bash
cd GREENY-CPSY-reproducibility
```

### Step 2 — install Python requirements

```bash
python3 -m pip install -r requirements.txt
```

### Step 3 — verify the installation and released package

```bash
bash scripts/verify_repository.sh
```

This runs the quick deterministic tests and confirms that the released result tables, citation metadata and licenses are present.

If this fails, stop. Do not interpret a partial run as a scientific result. Check the error and the relevant file in `verification/` or `docs/`.

### Step 4 — reproduce the main matrix

For the paper-scale run:

```bash
THREADS=32 SEEDS=56 make full
```

For a smaller smoke test while learning the package:

```bash
SEEDS=2 THREADS=4 FAMILIES=20 STEPS=400 N=80 make full
```

The smaller run is for checking that the machinery works. It is **not** a replacement for the released 56-seed analysis.

### Step 5 — inspect the generated output

```bash
ls -lh results/
```

The main generated files are typically:

```text
results/matrix_per_seed.csv
results/matrix_summary.csv
results/differences.csv
results/full.log
```

Use `data/primary/` for the released reference tables used by the reported analysis.

---

## 6. How to interpret the main four-cell experiment

The primary comparison has two contact conditions and two interaction conditions:

```text
                    one-sided       two-sided
random                 R-D             R-Y
recurrent-local        M-D             M-Y
```

The important point is that the two interaction conditions use matched encounter ledgers within the corresponding contact regime. The simulation seed is the inferential replicate.

Do not treat the 4000 agents, the individual encounters, or the 12000 simulation steps as independent statistical observations.

The four-cell difference-in-differences quantity is reconstructed from the four matched cells. See:

```text
docs/EXPERIMENT_CONTRACT.md
tests/test_suite.py
verification/reconstruct_claims.py
```

---

## 7. BranchN: coordinate-realization sensitivity

BranchN asks a narrow question:

> **If recurrence is held effectively fixed, does changing which agents occupy the exogenous local coordinate change the recurrent-local effect?**

The released production design is:

```text
24 coordinate realizations × 12 seeds = 288 trajectories
```

From the repository root:

```bash
bash branchN/coordinate_realization_distribution/scripts/run_branchN.sh
```

For an explicitly smaller test:

```bash
python3 branchN/coordinate_realization_distribution/scripts/run_branchN.py \
  --seeds 2 \
  --threads 1 \
  --workers 2 \
  --families 20 \
  --steps 400 \
  --n 80 \
  --offsets 0 1
```

### Important reading rule

BranchN coordinate realizations are a **sensitivity dimension**, not 288 independent human-network observations.

For BranchN, `spread` means:

```text
spread = maximum realization-level mean - minimum realization-level mean
```

It is not a standard deviation, confidence interval, or p-value.

See:

```text
branchN/BRANCHN.md
branchN/coordinate_realization_distribution/docs/NOOB_GUIDE.md
branchN/coordinate_realization_distribution/docs/USER_OBSERVED_12SEED_RESULT.md
```

---

## 8. Temporal/exposure network analysis

The network extension reconstructs the imposed contact sequence and its weighted exposure representation.

It is important to distinguish:

```text
contact event       = who met whom at a particular step
exposure weight     = how many times a pair met
social relationship = NOT inferred by this model
```

The aggregate recurrent-local graph is a cycle on the coordinate-ranked agents. This is a mathematical property of the matching construction, not a claim that human social networks are cycles.

To run the full BranchN + network audit:

```bash
bash branchN/extensions/temporal_exposure_network/scripts/run_full_branchN_network.sh
```

The literature-grounded methods and definitions are documented in:

```text
branchN/extensions/temporal_exposure_network/docs/LITERATURE_NETWORK_METHODS.md
branchN/extensions/temporal_exposure_network/docs/METHODS.md
branchN/extensions/temporal_exposure_network/docs/OUTPUTS.md
data/network/temporal_network_report.md
```

---

## 9. Interaction-disambiguation controls

The interaction fork addresses three distinct questions while preserving the encounter ledger within each seed/contact regime.

```text
D_SWAP
    Does changing which participant receives the one-sided update
    change the result?

Y_ABLATE
    Does removing the partner-state contribution from the two-sided
    target materially change the result?

Y_HALFALPHA
    Does halving the per-update assimilation fraction make the
    two-sided result approach the one-sided result?
```

The released production design is:

```text
56 seeds × 2 contact regimes × 5 interaction conditions = 560 rows
```

Run it with:

```bash
SEEDS=56 THREADS=32 FAMILIES=1000 STEPS=12000 N=4000 \
bash branchN/forks/interaction_disambiguation/scripts/run_interaction_fork.sh
```

Released outputs are under:

```text
data/interaction_disambiguation/
```

The fork's own interpretation limits are recorded in:

```text
branchN/forks/interaction_disambiguation/docs/METHOD.md
branchN/forks/interaction_disambiguation/docs/SELF_AWARE_STATUS.md
```

The three controls should not be conflated:

```text
D_SWAP      = recipient-role exchange
Y_ABLATE    = partner-signal ablation
Y_HALFALPHA = update-intensity sensitivity
```

`Y_HALFALPHA` is not claimed to match every possible definition of exposure dose exactly.

---

## 10. Reproduce the released tables versus rerun the simulations

These are different activities.

### Read the released results

Start here if the goal is to inspect the reported numbers without spending hours computing:

```text
data/primary/
data/branchN/
data/network/
data/interaction_disambiguation/
```

Each directory contains a README or result index describing the files.

### Rerun the analyses

Use the scripts in `reproduce/`, `branchN/`, and the root `Makefile`.

A rerun can produce machine-dependent runtime or floating-point differences. What matters scientifically is that the declared conditions, ledgers, replicate structure and reconstruction checks agree with the experiment contract.

---

## 11. Verification versus validation

The repository deliberately distinguishes several ideas that are often mixed together.

### Verification

Does the implementation do what the code specification says?

Examples:

```text
structure audits
ledger-equality checks
symbolic/analytical invariants
thread reproducibility checks
claim reconstruction
residual checks
```

### Sensitivity analysis

Does the result change when an important modelling choice is changed?

Examples:

```text
BranchN coordinate realizations
local parameter perturbations
update-rule counterfactuals
```

### Validation

Does GREENY adequately represent an empirical system?

That is **not established by this repository**.

The attachment-related population anchors and other literature-informed values are therefore documented by provenance rather than presented as proof of clinical validity.

---

## 12. Reproducibility and provenance

The repository records:

```text
source code
parent-core provenance
released numerical outputs
experiment contracts
hashes/checksums
verification scripts
reference fixtures
```

The versioned Git release and its archived DOI are the stable citation objects. A Git branch such as `main` should not be treated as a permanent scientific identifier.

See:

```text
provenance/README.md
provenance/VERSION.txt
provenance/RELEASE_MANIFEST.txt
```

---

## 13. Common mistakes

### Mistake: starting with the full production run

Don't.

Start with:

```bash
make verify
```

Then use the released data to understand the structure.

### Mistake: using all CPU threads indiscriminately

More threads are not automatically better. For the paper-scale reproduction on a 32-core/64-thread machine, `THREADS=32` is a reasonable starting point. Change it only for a computational reason.

### Mistake: interpreting a smoke run as the paper result

A smoke run verifies machinery. The reported production results come from the declared production configurations.

### Mistake: treating every row as a statistical replicate

The replicate is the **simulation seed**.

### Mistake: calling repeated imposed contact “attachment”

The recurrent-local mechanism imposes exposure. It does not infer or create a natural social relationship.

### Mistake: treating the graph audit as evidence of a realistic social network

The recurrent-local aggregate graph is a cycle because that is how the matching rule is constructed.

### Mistake: confusing `spread` with uncertainty

`spread` is a max-minus-min range of realization-level means. It is not a standard error or confidence interval.

---

## 14. If something fails

Use this sequence.

### First: record the environment

```bash
gcc --version
g++ --version
python3 --version
uname -a
nproc
```

### Second: rerun the smallest check

```bash
make verify
```

### Third: save the complete terminal output

```bash
make verify 2>&1 | tee verification/local_verify.log
```

### Fourth: inspect the relevant section

```text
verification/README_TESTS.md
verification/AUDIT_PLAN.md
verification/TEST_PLAN.md
branchN/*/docs/
```

Do not silently edit the production model to make a failing test pass. A failing test is information about the implementation/environment and should be diagnosed before interpretation.

---

## 15. Repository boundary

This repository is designed to support computational reproducibility and auditability.

It does **not** establish that:

- GREENY is a validated clinical model;
- the simulated attachment coordinates are clinical measurements;
- the synthetic contact process is an empirically realistic social network;
- a model-generated association is a causal effect in patients;
- a passing software audit proves the underlying scientific model is true.

The purpose of the intervention framework is narrower: to make it possible to ask which specified component of a generative model changes a model-generated association under controlled counterfactuals.

---

## 16. Citation

Citation metadata are provided in `CITATION.cff`.

For a paper-associated release, cite the **versioned release/archived DOI**, not an unversioned `main` branch, when a persistent DOI is available.

The associated manuscript is maintained separately from this repository.

---

## 17. License

Software is released under the MIT License in `LICENSE-CODE`.

Documentation and supplied research data are released under the terms stated in `LICENSE-DOCS-DATA`, subject to the author's ownership and any later publisher-specific terms.

---

## 18. Suggested reading order

For someone encountering GREENY for the first time:

```text
README.md
   ↓
QUICKSTART.md
   ↓
docs/EXPERIMENT_CONTRACT.md
   ↓
docs/SPEC.md
   ↓
data/RESULTS_INDEX.md
   ↓
verification/README_TESTS.md
   ↓
branchN/BRANCHN.md
   ↓
branchN/forks/interaction_disambiguation/docs/METHOD.md
```

A reader interested primarily in the network construction can then move to:

```text
branchN/extensions/temporal_exposure_network/docs/
```

---

## Final practical rule

**Do not start by trying to reproduce everything.**

Start by proving to yourself that the repository is intact:

```bash
make verify
```

Then inspect the released tables.

Then run the full 56-seed reproduction if an independent rerun is required.

Then run BranchN and the interaction-disambiguation fork if the corresponding sensitivity results need to be regenerated.

This order keeps the computational cost proportional to the question being asked and makes failures easier to diagnose.
