# bignum-abs

`bignum-abs` is a standalone C11/YASM x86-64 module that canonicalizes the absolute value of an unsigned `bignum_t`. The core representation is non-negative and has no sign bit to flip, so a successful call preserves the represented value, removes zero words above the most significant non-zero word, clears the unused tail, and updates `len`. The production implementation is independent YASM conforming to the System V AMD64 ABI; the C11 implementation is retained as a reference and baseline.

## Scope and contract

The public function is declared in `include/bignum_abs.h`:

```c
bignum_abs_status_t bignum_abs(bignum_t *num);
```

The caller owns and allocates `num`; the function allocates no memory, transfers no ownership, and uses no mutable global state. `num` may be `NULL` only when the caller is intentionally testing the null error path. On success, `len <= BIGNUM_CAPACITY`, zero has `len == 0`, all words at indices `len` through `BIGNUM_CAPACITY - 1` are zero, and the represented value is unchanged. On either error, the record is byte-for-byte unchanged. Calls on distinct records are thread-safe; concurrent access to one record requires caller synchronization.

| Status | Meaning and output state |
|---|---|
| `BIGNUM_ABS_SUCCESS` | The caller-owned record was normalized and its unused words were cleared. |
| `BIGNUM_ABS_ERROR_NULL_ARG` | `num` was `NULL`; no storage was accessed and no output exists. |
| `BIGNUM_ABS_ERROR_LENGTH` | `num->len > BIGNUM_CAPACITY`; the record was not modified. |

## Repository and dependencies

The repository contains the `bignum-core` submodule, which defines `bignum_t` and `BIGNUM_CAPACITY`, and a vendored distribution of `benchmark-framework` under `libs/benchmark-framework/dist`. Clone with submodules so the core headers are available:

```bash
git clone --recurse-submodules https://github.com/kirill-bayborodov/bignum-abs.git
cd bignum-abs
git submodule update --init --recursive
```

The normal toolchain is GCC, YASM, GNU Make, pthreads, cppcheck, Valgrind, and the C11 benchmark framework. `perf` and `taskset` are optional for hardware-counter workflows; direct benchmark execution remains available when a container does not expose performance events.

## Build, test, and analysis

Build the production object and dependencies:

```bash
make build CONFIG=release USE_ASM=yes
```

Run the complete deterministic, extended, multithreaded, distribution, and adapter test suite:

```bash
make test CONFIG=release USE_ASM=yes
```

The expected result is `=== Summary: 0 / 5 failed ===`. The same suite can exercise the C11 reference with `USE_ASM=no`. Run safety and static checks as follows:

```bash
make clean
make test_sanitize CONFIG=debug SAN=address USE_ASM=yes
make clean
make test_sanitize CONFIG=debug SAN=undefined USE_ASM=yes
make lint
```

The test sources document their oracle and cover null input, invalid length, zero and non-zero values, stale tails, all valid lengths, deterministic randomized cases, independent concurrent records, distribution linkage, and benchmark-adapter validation.

## API integration example

The following complete example checks the named status and does not allocate or free library-owned memory:

```c
#include "bignum_abs.h"
#include <stdio.h>

int main(void)
{
    bignum_t value = {0};
    bignum_abs_status_t status = bignum_abs(&value);

    if (status != BIGNUM_ABS_SUCCESS) {
        fprintf(stderr, "bignum_abs failed: %d\n", (int)status);
        return 1;
    }
    return 0;
}
```

Compile after building the module and its core dependency:

```bash
gcc example.c build/bignum_abs.o -I./include -I./libs/bignum-core/include -o example -no-pie
./example
```

## Assembly boundary

The YASM symbol `bignum_abs` receives `bignum_t *num` in `rdi` and returns the named status in `rax`. The record layout is 32 `uint64_t` words followed by a `size_t len` field at byte offset `256`. The implementation validates before stores, then trims and clears in place; because no operation after validation can fail, this preserves the error transaction without a stack candidate. No C helper is called. `rsp` and all System V callee-saved registers are preserved, no stack storage is used, and flags are caller-clobbered.

## Benchmarks

The single-thread and multithread runners are `benchmarks/bench_bignum_abs.c` and `benchmarks/bench_bignum_abs_mt.c`. Each successful run emits a machine-readable `benchmark=...` line immediately before `Benchmark finished.`. The line includes mode, seed, input profile, operation profile, measurement scope, iteration count, successful-call count, fingerprint, checksum, elapsed seconds, and nanoseconds per call.

For a reproducible direct comparison, build each implementation separately and keep the workload fixed:

```bash
make clean && make bench_st CONFIG=release USE_ASM=no || true
bin/bench_bignum_abs --iterations 100000 --warmup 1000 --data-count 64 --seed 123456789 \
  --input-kind nonzero --operation-kind abs --measure-mode end-to-end \
  --size-profile half --capacity-profile normal

make clean && make bench_st CONFIG=release USE_ASM=yes || true
bin/bench_bignum_abs --iterations 100000 --warmup 1000 --data-count 64 --seed 123456789 \
  --input-kind nonzero --operation-kind abs --measure-mode end-to-end \
  --size-profile half --capacity-profile normal
```

The `|| true` is needed only in containers where the protected Makefile's optional `perf`/`sysctl` wrapper cannot run; the binary is built before that host-specific wrapper fails. For stable conclusions, use repeated matrix measurements with `make bench_matrix`, retain the revision and build configuration, and compare medians and MAD rather than a single smoke sample.

| Profile | Meaning |
|---|---|
| `one` | One logical word. |
| `quarter` | Approximately one quarter of capacity. |
| `half` | Approximately half of capacity. |
| `variable` | Deterministically varied valid lengths. |
| `near-capacity` | Valid storage-boundary workload; it does not intentionally measure an invalid length. |

The committed manifests are `benchmarks/profiles/bignum_abs_standard.json` and `bignum_abs_full.json`; each has an adjacent `.json.md` companion describing schema version 1, vocabulary, profile meaning, commands, failure handling, and baseline comparison rules. The matrix tools are C11 and do not require Python.

## Distribution and contribution

Create the object and single-header/static-library distributions with:

```bash
make install CONFIG=release
make dist CONFIG=release
```

Generated artifacts can be removed with `make clean`. Contributions must preserve the C/ASM contract, update tests and the adjacent JSON companion when behavior or vocabulary changes, and provide reproducible benchmark context. At minimum, run `make test CONFIG=release USE_ASM=yes` and `make lint` before submission. The documentation quality-gate checklist is maintained in `QUALITY_GATES_DOCUMENTATION_C11_JSON.md` when supplied with the review package.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
