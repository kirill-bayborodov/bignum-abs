# bignum_abs_standard.json

## Purpose

This version-1 manifest is the compact deterministic regression matrix for `bignum_abs`. The C11 `bench_matrix` consumer launches the project ST and MT runners and records protocol output and timing samples.

## Location and lifecycle

The source manifest is committed at `benchmarks/profiles/bignum_abs_standard.json`. It is reviewed input, not generated output. Changes to profile identifiers or fields require an updated companion document and a new baseline; result JSON files under `benchmarks/reports/` are generated artifacts.

## Schema and vocabulary

The root object contains required integer `schema_version` (currently `1`), required string `description`, and required array `profiles`. Each profile requires string fields `id`, `input_kind`, `operation_kind`, `measure_mode`, `size_profile`, and `capacity_profile`. `input_kind` is `zero`, `nonzero`, or `mixed`; `operation_kind` is `abs`, `normalize`, `abs-normalize`, `noop`, `default`, or `mixed`; `measure_mode` is `end-to-end` or `kernel-only`; `size_profile` is `one`, `quarter`, `half`, `variable`, or `near-capacity`; and `capacity_profile` is `normal` or `near-capacity`. The legacy aliases are accepted only for compatibility with the framework transport and all invoke the same absolute-normalization operation.

| Profile family | Input | Operation | Measurement | Size/capacity |
|---|---|---|---|---|
| Zero smoke | `zero` | `abs` | `end-to-end` | `one`/`normal` |
| Small non-zero | `nonzero` | `abs` | `kernel-only` | `one`/`normal` |
| Medium non-zero | `nonzero` | `abs` | `kernel-only` | `quarter` or `half`/`normal` |
| Variable | `nonzero` or `mixed` | `abs` | `end-to-end` | `variable`/`normal` |
| Boundary | `nonzero` | `abs` | either | `near-capacity`/`near-capacity` |

## Complete example and how to run

A complete minimal fragment is `{"schema_version":1,"description":"bignum_abs smoke","profiles":[{"id":"zero-one-end-to-end","input_kind":"zero","operation_kind":"abs","measure_mode":"end-to-end","size_profile":"one","capacity_profile":"normal"}]}`. Build both runners and execute the pinned C11 matrix tool with:

```bash
make bench_matrix CONFIG=release BENCH_MATRIX_PROFILE=benchmarks/profiles/bignum_abs_standard.json \
  REPORT_NAME=bignum_abs_standard BENCH_MATRIX_REPETITIONS=1 \
  BENCH_MATRIX_ITERATIONS=1001 BENCH_MATRIX_MT_TOTAL_ITERATIONS=2000 MT_THREADS=2
```

The output paths are `benchmarks/reports/bignum_abs_standard_matrix.json` and its summary JSON. Each successful sample has one `benchmark=...` line immediately before `Benchmark finished.`.

## Modification, baseline, and failure handling

To add a profile, choose only documented vocabulary, add a unique stable identifier, update this table and the full companion, then run the matrix and review the new profile set before accepting a baseline. Candidate and baseline manifests must have identical profile identifiers and ST/MT coverage; missing or extra profiles are a failed comparison. Malformed JSON, unsupported schema, invalid vocabulary, or a runner that omits the required completion marker produces a non-zero tool result and must not be aggregated as a valid sample. `near-capacity` is a valid boundary workload, not an intentional invalid-length benchmark.
