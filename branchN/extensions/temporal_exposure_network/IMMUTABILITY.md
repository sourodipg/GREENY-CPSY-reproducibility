# Immutability contract

The extension is additive. It must not modify:

- `code/symmetric_network_matrix.cpp`
- `code/network_kernels.hpp`
- `results/`
- `verification/authoritative/`
- manuscript sources or PDFs
- prior BranchN outputs.

The probe includes frozen implementation files for exact reconstruction but writes only under this extension's `results/` directory.
