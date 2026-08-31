# Literature-grounded temporal/contact-network audit

## Scope

This extension does not introduce a new social-network model. It analyzes the contact process already imposed by the frozen GREENY implementation. The primary object is therefore a temporal contact network: a set of contact events indexed by time. Holme and Saramäki describe this representation as a contact sequence of events `(i,j,t)` and distinguish it from the time-aggregated graph obtained by collapsing repeated contacts between the same vertices.

The weighted aggregate uses an edge weight `w_ij` equal to the number of contact events between agents `i` and `j` over the specified run. Degree is the number of distinct neighbours and strength is the sum of incident edge weights. These are standard quantities for weighted-network analysis; Newman and Barrat et al. provide canonical treatments of weighted networks, degree/strength and mixing.

Trait mixing is assessed using scalar assortativity: the association between an attribute on one end of an edge and the corresponding attribute on the other end. Newman introduced assortative mixing measures for scalar vertex attributes. Because GREENY contacts are undirected at the contact level, the primary attribute-assortativity statistic is an endpoint correlation over undirected edges. A contact-weighted version uses `w_ij` as the edge weight and is reported separately so the repeated-contact intensity is not silently discarded.

The extension also reports the distribution of edge weights, node degree, node strength, graph density, connected components, and clustering diagnostics. These are descriptive structural quantities. The temporal representation is retained because collapsing to a static graph loses event ordering; Holme and Saramäki and Blonder et al. discuss this distinction explicitly.

## Why the GREENY M graph has a special structure

The frozen metric-local kernel sorts agents by a fixed exogenous coordinate `z` and pairs adjacent ranks with a cyclic shift. For even `N`, shifts differing by 2 produce the same perfect matching, so only shift parity changes which of the two alternating perfect matchings is active. The union over time is therefore a cycle on the rank-ordered agents.

This fact is derived and tested by the extension rather than treated as an empirical claim. The exact M exposure graph should therefore have:

- one connected component;
- degree exactly 2 for every agent;
- binary clustering coefficient 0 for `N > 3`;
- `N` undirected edges in the time-aggregated simple graph;
- node strength equal to the total number of contacts experienced by the node, i.e. one contact per timestep in the perfect matching;
- two edge-weight classes when the two matching parities occur with different frequencies, with weights equal to the counts of the two shift parities.

These are implementation consequences of the frozen contact kernel, not empirical social-network regularities.

## Attribute analysis

For each offset and seed, the extension computes attribute assortativity on the M aggregate graph for anxiety `A` and avoidance `V` in two forms:

1. **Unweighted edge-end assortativity:** Pearson correlation of the attribute values at the two ends of the simple graph's edges.
2. **Exposure-weighted edge-end assortativity:** the same endpoint covariance/correlation with each edge weighted by its contact count `w_ij`.

The weighted statistic is reported as a sensitivity analysis because weighted networks can contain information that binary projections discard. No empirical threshold is imposed on the coefficient; the primary use is descriptive comparison across coordinate realizations.

The extension additionally computes edge-weight versus trait-distance summaries, including weighted means of `|A_i-A_j|` and `|V_i-V_j|`. These answer whether the most frequently repeated imposed contacts are disproportionately between similar or dissimilar trait values.

## Temporal diagnostics

The extension retains the already-validated same-partner recurrence measures (`repeat1`, `repeat5`, `repeat20`) and adds contact-gap summaries from the exact parity process: edge reuse counts and the distribution of edge weights. These do not infer natural relationship formation. They characterize imposed exposure.

## Null/sensitivity interpretation

The 24 coordinate realizations in BranchN are treated as alternative exogenous assignments of agents to positions on the imposed ring. Associations between graph quantities and the BranchN M one-sided-to-two-sided contrasts are exploratory realization-level analyses, not confirmatory p-values. A graph statistic is considered mechanistically informative only when the statistic changes across coordinate realizations and tracks the outcome effect without being reducible to the fixed recurrence intensity.

## References

1. Holme, P. & Saramäki, J. (2012). Temporal networks. *Physics Reports*, 519(3), 97–125. doi:10.1016/j.physrep.2012.03.001.
2. Newman, M. E. J. (2003). Mixing patterns in networks. *Physical Review E*, 67, 026126. doi:10.1103/PhysRevE.67.026126.
3. Newman, M. E. J. (2004). Analysis of weighted networks. arXiv:cond-mat/0407503.
4. Barrat, A., Barthélemy, M., Pastor-Satorras, R. & Vespignani, A. (2004). The architecture of complex weighted networks. *PNAS*, 101(11), 3747–3752. doi:10.1073/pnas.0400087101.
5. Opsahl, T. & Panzarasa, P. (2009). Clustering in weighted networks. *Social Networks*, 31(2), 155–163. doi:10.1016/j.socnet.2009.02.002.
6. Blonder, B. & Dornhaus, A. (2012). Temporal dynamics and network analysis. *Methods in Ecology and Evolution*, 3, 958–972. doi:10.1111/j.2041-210X.2012.00236.x.
7. Karsai, M., Perra, N. & Vespignani, A. (2014). Time varying networks and the weakness of strong ties. *Scientific Reports*, 4, 4001. doi:10.1038/srep04001.
