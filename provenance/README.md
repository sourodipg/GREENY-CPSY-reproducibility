# Provenance

The production network sources retain their original include paths so that the scientific implementation is not silently rewritten during repository packaging. The small `parent/reproduction_package/code/greeny/src/` tree contains exactly the parent headers required by the production source.

`reference/PARENT_MANIFEST.sha256` records the exact hashes of those parent-core files used by the repository test suite. This is a deliberately curated provenance snapshot, not a claim to reproduce every historical build artifact in the larger development archive.

The remaining provenance documents record release version, source hashes and the status of the final audit.
