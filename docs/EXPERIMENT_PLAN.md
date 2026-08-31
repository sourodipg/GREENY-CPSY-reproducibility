# Experiment plan

## Primary
56 paired seeds, N=4000, 12000 steps, families=1000, frozen GREENY parameters. Conditions: MATCHED-DIRECTED and SYMMETRIC-MATCHING.

## Reference
GREENY frozen reference is run through the parent implementation for each seed and reported alongside the matched ledger conditions, but not used as the primary causal counterfactual.

## Secondary finite-mixing probe
12 paired seeds at families={1000,500,250,100}. Total N remains 4000. This asks whether reciprocity/local coupling changes when family size changes, while holding the total population fixed. It is exploratory and is not fitted.

## Statistical report
Use paired seed differences. Report mean, SD, standard error, 95% t interval, paired permutation p-value and Cohen's d_z. For correlation outcomes, also report Fisher-z sensitivity in the analysis script. No multiple-testing correction is used to turn exploratory network diagnostics into headline claims; they are labelled secondary.
