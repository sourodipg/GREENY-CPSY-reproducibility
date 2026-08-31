# Results schema

`matrix_per_seed.csv` contains one row per seed × mixing × interaction cell.

`matrix_summary.csv` contains means over seeds.

`differences.csv` contains pre-specified paired contrasts:

- dyadic at random;
- dyadic at metric;
- metric at directed;
- metric at dyadic;
- selected interaction contrasts.

`pair_digest` must match between directed and dyadic rows for the same seed and mixing condition. This is the computational proof that the pair ledger is held fixed for the dyadic comparison.
