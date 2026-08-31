# BranchN extension: temporal/contact-network audit

This is an additive, literature-grounded audit layer. The frozen GREENY source, executable, manuscript, BranchN experiment and archived results are not modified.

The extension analyzes the **imposed temporal contact process** used by the recurrent-local M condition. It reconstructs the exact M exposure graph from the frozen matching kernel and joins standard network metrics to the existing BranchN effect contrasts.

Start here:

```bash
bash branchN/extensions/temporal_exposure_network/scripts/run_network_audit.sh
```

For a smoke run:

```bash
bash branchN/extensions/temporal_exposure_network/scripts/run_network_audit.sh --offsets 0 1 2 --seeds 101 202 303 --workers 3
```

The extension does not call the graph a social network and does not infer natural attachment. Edge weight means repeated **imposed exposure**.
