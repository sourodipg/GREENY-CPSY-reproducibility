# BranchN extension — literature-grounded temporal/contact-network audit

## Noob answer

The graph is an **imposed contact/exposure graph**, not an emergent friendship or attachment network. The frozen M kernel repeatedly places neighbouring agents in a fixed coordinate ordering into contact.

## Run

Offsets: 24; seeds per offset: 12; frozen trajectories represented: 288.

## Structural result

For even N, the frozen metric-local matching has only two distinct alternating perfect matchings because cyclic shifts differing by two produce the same unordered pairing. Their union is therefore a cycle through the rank-ordered agents.

Across the audited realizations, the graph should therefore have degree 2 for every node, one connected component, zero binary triangle clustering for N=4000, and total node strength equal to the number of simulation steps because every node participates in exactly one pair per timestep.

The actual output provides these values rather than assuming them.

## Exposure intensity

Edge weight means the number of times a pair is forced to interact. This is a standard weighted-network quantity; it should be read as **repeated imposed exposure**, not relationship strength in a psychological sense.

## Trait mixing

The primary attribute statistics are scalar edge-end assortativity for anxiety and avoidance, reported both unweighted and weighted by exposure count. Newman introduced scalar assortativity for node attributes; weighted-network literature motivates retaining contact weights rather than silently binarizing them.

## BranchN question

The upstream BranchN experiment holds recurrence nearly fixed while changing which agents occupy the local neighbourhoods. The downstream question is therefore whether standard graph structure or trait mixing explains the resulting variation in the M one-sided-to-two-sided contrast.

Strongest descriptive association with M `rA`: `A_assort_weighted_mean`, r = -0.3042.

Strongest descriptive association with M `rV`: `A_assort_unweighted_mean`, r = -0.2735.

These 24-realization correlations are exploratory descriptions, not confirmatory p-values.

## Literature

- Holme & Saramäki (2012): temporal/contact-sequence representation and time-aggregated graphs.
- Newman (2003): scalar attribute assortativity / mixing.
- Newman (2004): weighted networks.
- Barrat et al. (2004): degree/strength structure in weighted networks.
- Opsahl & Panzarasa (2009): weighted clustering.
- Blonder & Dornhaus (2012): time-ordered versus time-aggregated network analysis.
- Karsai et al. (2014): repeated interactions and temporal network structure.

See `docs/LITERATURE_NETWORK_METHODS.md` for full references and methodological notes.
