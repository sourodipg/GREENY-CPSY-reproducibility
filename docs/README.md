# How to run

From the package root:

```bash
make -B verify
make sanitizer
THREADS=15 SEEDS=12 FAMILIES=1000 STEPS=12000 N=4000 make fast
THREADS=15 SEEDS=56 FAMILIES=1000 STEPS=12000 N=4000 make full
```

For a controlled intermediate mixing level:

```bash
THREADS=15 SEEDS=12 MIX_STRENGTH=0.50 make fast
```

The final scientific output is in `results/`. The `reference/` and `parent/` directories are provenance artifacts; do not overwrite them during analysis.
