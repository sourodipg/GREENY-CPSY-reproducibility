#!/usr/bin/env python3
"""
GREENY public-repository self-check.

Checks the released GitHub repository structure only.

A PASS means:
  * required public reproducibility files are present;
  * required scientific directories are present;
  * the manuscript/educational-guide files are not present;
  * no obsolete archive assumptions are being used.

A PASS does NOT establish clinical validity, empirical validity,
or correctness of every scientific claim.
"""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]

REQUIRED_FILES = [
    "README.md",
    "QUICKSTART.md",
    "CITATION.cff",
    "LICENSE-CODE",
    "LICENSE-DOCS-DATA",
    "LICENSE.md",
    "requirements.txt",
    "Makefile",

    # Core model source
    "code/network_kernels.hpp",
    "code/symmetric_network_experiment.cpp",
    "code/symmetric_network_matrix.cpp",

    # Parent model source required for provenance/reproduction
    "parent/reproduction_package/code/greeny/src/cli.hpp",
    "parent/reproduction_package/code/greeny/src/params.hpp",
    "parent/reproduction_package/code/greeny/src/population.hpp",
    "parent/reproduction_package/code/greeny/src/simulate.hpp",

    # Released primary data
    "data/primary/matrix_per_seed.csv",
    "data/primary/matrix_summary.csv",

    # BranchN data
    "data/branchN/branchN_by_offset.csv",
    "data/branchN/branchN_by_seed.csv",
    "data/branchN/branchN_summary.csv",

    # Network data
    "data/network/network_metrics_by_offset.csv",
    "data/network/network_metrics_by_seed.csv",
    "data/network/temporal_network_report.md",

    # Interaction-disambiguation data
    "data/interaction_disambiguation/interaction_disambiguation_per_seed.csv",
    "data/interaction_disambiguation/interaction_disambiguation_contrasts.csv",
    "data/interaction_disambiguation/interaction_disambiguation_summary.csv",

    # Reproduction helpers
    "reproduce/verify_preflight.py",

    # Released verification artefacts
    "verification/claim_reconstruction.json",
    "verification/claim_residuals.csv",
    "verification/claim_residuals.json",
    "verification/coordinate_realization_audit.json",
    "verification/final_release/fixed_point_audit.py",
]

REQUIRED_DIRS = [
    "code",
    "parent",
    "data",
    "branchN",
    "tests",
    "verification",
    "reproduce",
    "provenance",
    "reference",
    "docs",
]

FORBIDDEN_PATH_FRAGMENTS = [
    "paper/",
    "GREENY_CPSY_Submission",
    "Supplementary_Methods",
    "Guided_Recursive_Explanation",
]


def check_required_files() -> bool:
    missing = [p for p in REQUIRED_FILES if not (ROOT / p).is_file()]
    if missing:
        print("FAIL | required public files are missing:")
        for p in missing:
            print(f"  {p}")
        return False

    print(f"PASS | required public files present ({len(REQUIRED_FILES)} checked)")
    return True


def check_required_dirs() -> bool:
    missing = [p for p in REQUIRED_DIRS if not (ROOT / p).is_dir()]
    if missing:
        print("FAIL | required public directories are missing:")
        for p in missing:
            print(f"  {p}")
        return False

    print(f"PASS | scientific repository domains present ({len(REQUIRED_DIRS)} checked)")
    return True


def check_public_scope() -> bool:
    hits = []

    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue

        rel = path.relative_to(ROOT).as_posix()

        if any(fragment in rel for fragment in FORBIDDEN_PATH_FRAGMENTS):
            hits.append(rel)

    if hits:
        print("FAIL | forbidden manuscript/guide paths found:")
        for p in sorted(hits):
            print(f"  {p}")
        return False

    print("PASS | public repository contains no manuscript/guide artefacts")
    return True


def check_obsolete_paths() -> bool:
    """
    These paths belonged to an earlier internal package layout and should
    NOT be required by the public repository self-check.
    """
    obsolete = [
        ROOT / "FROZEN_PARENT_TAG.md",
        ROOT / "parent/reproduction_package/README.md",
        ROOT / "history",
    ]

    present = [str(p.relative_to(ROOT)) for p in obsolete if p.exists()]

    if present:
        print("NOTE | obsolete internal-layout paths are present:")
        for p in present:
            print(f"  {p}")
        print("      They are not required by this public self-check.")

    print("PASS | public self-check does not depend on obsolete internal-layout paths")
    return True


def main() -> int:
    print("=== GREENY PUBLIC REPOSITORY SELF-CHECK ===")

    checks = [
        check_required_files(),
        check_required_dirs(),
        check_public_scope(),
        check_obsolete_paths(),
    ]

    print()

    if all(checks):
        print("SELF-CHECK PASS")
        print(
            "This check validates repository completeness and release scope. "
            "It does not establish empirical or clinical validity."
        )
        return 0

    print("SELF-CHECK FAIL")
    return 1


if __name__ == "__main__":
    sys.exit(main())
