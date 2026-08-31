# Release note

This is the final audit-oriented GREENY package assembled after recursive review of the manuscript PDF and curated comments.

The manuscript adds explicit model-world instantiation, encounter semantics, geometry, recursive provenance, an audit-level residual definition, and an exploratory clock robustness branch. The source corrects the 2×2 interaction output calculation so that the printed interaction is the difference of the two matched update contrasts. The definitive 56-seed matrix is retained and reconstructed directly from seed-level values.

Fresh tests executed in the build include `make verify`, sanitizer smoke testing, a four-seed frozen-clock run, 15-step and 60-step observation-window runs, a 250-step shock-clock run, a 1000-step shock-clock run, source-contract checks, first-person prose scanning, and direct arithmetic reconstruction of the archived 56-seed matrix.

A fresh 56-seed C++ rerun was attempted but did not complete within the available environment execution window. This is explicitly recorded rather than represented as completed.
