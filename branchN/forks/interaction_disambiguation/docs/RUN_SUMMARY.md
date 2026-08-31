# What to expect after the run

The final diagnostic is deliberately conditional. It will first verify structure:

- all five conditions are present;
- the expected seed x contact-regime x condition row count exists;
- all numerical outputs are finite;
- encounter-ledger digests are identical within each seed/contact regime;
- recurrence diagnostics remain invariant across interaction conditions.

Only after those checks pass does it print scientific readouts.

The most important lines are:

`D_SWAP_minus_D_rA` and `D_SWAP_minus_D_rV`

`Y_ABLATE_minus_Y_rA` and `Y_ABLATE_minus_Y_rV`

`Y_HALFALPHA_minus_D_rA` and `Y_HALFALPHA_minus_D_rV`

`Y_HALFALPHA_minus_Y_rA` and `Y_HALFALPHA_minus_Y_rV`

The output prose states exactly what each line means before suggesting an interpretation. This is intentional: the audit should be readable without reconstructing notation from the source code.
