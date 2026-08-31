# Literature verification record

The methodology was checked against the following published network literature before implementation.

## Temporal/contact representation

Holme & Saramäki (2012), *Temporal Networks*, Physics Reports 519(3), 97–125, DOI 10.1016/j.physrep.2012.03.001. The review describes contact-sequence representations and distinguishes temporal networks from time-aggregated graphs.

Blonder & Dornhaus (2012), *Temporal dynamics and network analysis*, Methods in Ecology and Evolution, DOI 10.1111/j.2041-210X.2012.00236.x. The paper distinguishes time-ordered and time-aggregated network representations and emphasizes preservation of event ordering when temporal order matters.

## Attribute mixing / assortativity

Newman (2003), *Mixing patterns in networks*, Physical Review E 67, 026126, DOI 10.1103/PhysRevE.67.026126. The paper introduces assortative-mixing measures for scalar vertex attributes and degree mixing.

## Weighted networks / strength

Newman (2004), *Analysis of weighted networks*, arXiv:cond-mat/0407503. Weighted connections are treated as edges carrying intensity/weight rather than only binary presence.

Barrat et al. (2004), *The architecture of complex weighted networks*, PNAS 101(11), 3747–3752, DOI 10.1073/pnas.0400087101. The analysis motivates separate degree and strength information in weighted networks.

Opsahl & Panzarasa (2009), *Clustering in weighted networks*, Social Networks 31(2), 155–163, DOI 10.1016/j.socnet.2009.02.002. Weighted clustering is used in the literature when edge intensity carries information; the present extension uses only the standard binary clustering diagnostic because the frozen M aggregate graph has an exact cycle structure.

## Repeated contacts / temporal interaction

Karsai, Perra & Vespignani (2014), *Time varying networks and the weakness of strong ties*, Scientific Reports 4, 4001, DOI 10.1038/srep04001. Recurrent communication patterns and temporal network structure are treated as distinct from a generic static graph; this supports retaining recurrence diagnostics and avoiding an interpretation of repeated contact as automatically equivalent to natural tie formation.

## Implementation consequence for GREENY

The literature motivates the *representation and diagnostics*, not a claim that GREENY's imposed M process is an empirical social network. The graph is therefore named an imposed temporal contact/exposure network throughout this extension.
