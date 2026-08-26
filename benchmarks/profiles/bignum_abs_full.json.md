# bignum_abs_full.json

## Purpose

This version-1 manifest is the extended performance matrix for the in-place canonical normalization operation. The C11 `bench_matrix` runner consumes it, launches the project-owned ST and MT binaries, and writes raw samples; the C11 `benchmark_stats` tool aggregates medians, means, standard deviation, MAD, and regression status.

## Location and lifecycle

The reviewed source is `benchmarks/profiles/bignum_abs_full.json`. It is committed input and must be changed together with this document. Matrix and summary JSON files under `benchmarks/reports/` are generated outputs and are not source profiles.

## Schema, vocabulary, and profile table

The root requires integer `schema_version: 1`, a string `description`, and a `profiles` array. Every profile requires `id`, `input_kind`, `operation_kind`, `measure_mode`, `size_profile`, and `capacity_profile`. Allowed input values are `zero`, `nonzero`, and `mixed`; operation values are `abs`, `normalize`, `abs-normalize`, `noop`, `default`, and `mixed`; measurement values are `end-to-end` and `kernel-only`; size values are `one`, `quarter`, `half`, `variable`, and `near-capacity`; capacity values are `normal` and `near-capacity`.

The full manifest contains 12 profiles: one zero smoke case; two one-word cases; four quarter/half non-zero cases; two variable or mixed cases; and three valid near-capacity cases. Each profile is run in ST and MT mode, so `R` repetitions produce `12 × 2 × R` samples. The profile identifier is the compatibility key: a candidate with missing or extra identifiers is not comparable with the baseline.

## Complete run

Build the runners and run the matrix with fixed conditions:

```bash
make bench_matrix CONFIG=release \
  BENCH_MATRIX_PROFILE=benchmarks/profiles/bignum_abs_full.json \
  REPORT_NAME=bignum_abs_full BENCH_MATRIX_REPETITIONS=7 \
  BENCH_MATRIX_ITERATIONS=200000 BENCH_MATRIX_MT_TOTAL_ITERATIONS=320000 \
  MT_THREADS=2
```

The generated files are `benchmarks/reports/bignum_abs_full_matrix.json` and `benchmarks/reports/bignum_abs_full_matrix_summary.json` (the exact summary filename is reported by the Make target). For a direct tool invocation, the consumer binary is `libs/benchmark-framework/dist/tools/bench_matrix`; use the corresponding `benchmark_stats` binary for aggregation. Every accepted runner output must contain a machine-readable `benchmark=...` line immediately before `Benchmark finished.`.

## Baseline and comparison

A baseline is accepted only after recording source revision, compiler configuration, host/CPU constraints, seed, repetitions, data count, warm-up, thread count, affinity, and measurement mode. A later candidate must use the identical manifest and coverage. Regression is meaningful only when the candidate median exceeds both the configured percentage threshold and the robust MAD noise floor. A malformed manifest, unsupported schema, invalid vocabulary, incomplete profile set, missing completion marker, timeout, or failed status returns non-zero and is excluded from the baseline.

## Transport semantics

The adapter validates the fields before initialization, deterministically constructs fixed-capacity `bignum_t` records, invokes `bignum_abs`, and checks the named success status. Legacy operation aliases are transport compatibility values; they do not represent unrelated bit or shift operations. Near-capacity profiles remain valid and are intended to measure normalization near the storage boundary, while invalid lengths belong to the API test suite rather than performance aggregates.
