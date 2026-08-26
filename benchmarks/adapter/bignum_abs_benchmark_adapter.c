/**
 * @file bignum_abs_benchmark_adapter.c
 * @brief Benchmark-framework adapter for bignum_abs normalization.
 * @details The adapter creates deterministic fixed-capacity records, invokes
 * bignum_abs, and hashes the complete canonical record for observability.
 */
#include "bignum_abs_benchmark_adapter.h"
#include "bignum_abs.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ABS_FNV_OFFSET UINT64_C(1469598103934665603)
#define ABS_FNV_PRIME UINT64_C(1099511628211)

typedef struct abs_benchmark_state {
    bignum_t value;
} abs_benchmark_state_t;

/** @brief Compares two optional workload strings. */
static int equal_text(const char *left, const char *right)
{
    return left != NULL && right != NULL && strcmp(left, right) == 0;
}

/** @brief Advances the deterministic workload generator. */
static uint64_t next_value(uint64_t *state)
{
    if (*state == UINT64_C(0)) {
        *state = UINT64_C(0x9e3779b97f4a7c15);
    }
    *state ^= *state << 7U;
    *state ^= *state >> 9U;
    *state ^= *state << 8U;
    return *state;
}

/** @brief Checks a token against a NULL-terminated allowed-value list. */
static int allowed(const char *value, const char *const *list)
{
    size_t index;
    if (value == NULL || list == NULL) return 0;
    for (index = 0U; list[index] != NULL; ++index) {
        if (equal_text(value, list[index])) return 1;
    }
    return 0;
}

/** @brief Maps a benchmark size profile to a valid word length. */
static size_t choose_length(const benchmark_workload_t *workload, uint64_t *state)
{
    if (equal_text(workload->size_profile, "one") || equal_text(workload->size_profile, "tiny")) return 1U;
    if (equal_text(workload->size_profile, "quarter") || equal_text(workload->size_profile, "small")) return BIGNUM_CAPACITY / 4U;
    if (equal_text(workload->size_profile, "half") || equal_text(workload->size_profile, "medium")) return BIGNUM_CAPACITY / 2U;
    if (equal_text(workload->size_profile, "near-capacity") || equal_text(workload->size_profile, "large")) return BIGNUM_CAPACITY;
    return 1U + (size_t)(next_value(state) % BIGNUM_CAPACITY);
}

/** @brief Fills a deterministic record, optionally with a zero value. */
static void fill_value(bignum_t *value, size_t length, uint64_t *state, int zero)
{
    size_t word;
    memset(value, 0, sizeof(*value));
    if (zero) return;
    value->len = length == 0U ? 1U : length;
    for (word = 0U; word < value->len; ++word) value->words[word] = next_value(state);
    if (value->words[value->len - 1U] == UINT64_C(0)) value->words[value->len - 1U] = UINT64_C(1);
}

/** @brief Initializes one deterministic framework state. */
static benchmark_adapter_status_t initialize(void *opaque, uint64_t index,
                                              const benchmark_workload_t *workload, void *context)
{
    abs_benchmark_state_t *state = (abs_benchmark_state_t *)opaque;
    uint64_t random_state;
    int zero;
    (void)context;
    if (state == NULL || workload == NULL ||
        bignum_abs_benchmark_validate_workload(workload) != BIGNUM_ABS_BENCHMARK_STATUS_SUCCESS) {
        return BENCHMARK_ADAPTER_STATUS_INPUT_ERROR;
    }
    random_state = workload->seed ^ (index + UINT64_C(0x9e3779b97f4a7c15));
    zero = equal_text(workload->input_kind, "zero") ||
           (equal_text(workload->input_kind, "mixed") && (index & 1U) != 0U);
    fill_value(&state->value, choose_length(workload, &random_state), &random_state, zero);
    return BENCHMARK_ADAPTER_STATUS_SUCCESS;
}

/** @brief Normalizes one benchmark state and returns framework status. */
static benchmark_adapter_status_t operation(void *opaque, uint64_t iteration,
                                             const benchmark_workload_t *workload, void *context)
{
    abs_benchmark_state_t *state = (abs_benchmark_state_t *)opaque;
    (void)iteration;
    (void)workload;
    (void)context;
    if (state == NULL || bignum_abs(&state->value) != BIGNUM_ABS_SUCCESS) {
        return BENCHMARK_ADAPTER_STATUS_OPERATION_ERROR;
    }
    return BENCHMARK_ADAPTER_STATUS_SUCCESS;
}

/** @brief Hashes the complete normalized state for benchmark observability. */
static uint64_t checksum(const void *opaque, uint64_t iteration, void *context)
{
    const abs_benchmark_state_t *state = (const abs_benchmark_state_t *)opaque;
    uint64_t hash = ABS_FNV_OFFSET;
    size_t word;
    (void)context;
    if (state == NULL) return UINT64_C(0);
    for (word = 0U; word < BIGNUM_CAPACITY; ++word) {
        hash ^= state->value.words[word];
        hash *= ABS_FNV_PRIME;
    }
    hash ^= (uint64_t)state->value.len;
    hash *= ABS_FNV_PRIME;
    return hash ^ iteration;
}

bignum_abs_benchmark_status_t bignum_abs_benchmark_validate_workload(
    const benchmark_workload_t *workload)
{
    static const char *const input[] = { "zero", "nonzero", "mixed", NULL };
    static const char *const operation_values[] = { "abs", "normalize", "abs-normalize", "noop", "default", "mixed", NULL };
    static const char *const measure[] = { "end-to-end", "kernel-only", NULL };
    static const char *const size[] = { "one", "quarter", "half", "variable", "near-capacity", "tiny", "small", "medium", "large", NULL };
    static const char *const capacity[] = { "normal", "near-capacity", NULL };
    if (workload == NULL) return BIGNUM_ABS_BENCHMARK_STATUS_NULL_ARGUMENT;
    if (!allowed(workload->input_kind, input) || !allowed(workload->operation_kind, operation_values) ||
        !allowed(workload->measure_mode, measure) || !allowed(workload->size_profile, size) ||
        !allowed(workload->capacity_profile, capacity)) return BIGNUM_ABS_BENCHMARK_STATUS_INVALID_PROFILE;
    return BIGNUM_ABS_BENCHMARK_STATUS_SUCCESS;
}

bignum_abs_benchmark_status_t bignum_abs_benchmark_adapter_init(benchmark_adapter_t *adapter)
{
    if (adapter == NULL) return BIGNUM_ABS_BENCHMARK_STATUS_NULL_ARGUMENT;
    *adapter = (benchmark_adapter_t){
        .benchmark_name = "bignum_abs",
        .state_size = sizeof(abs_benchmark_state_t),
        .success_code = BENCHMARK_ADAPTER_STATUS_SUCCESS,
        .adapter_context = NULL,
        .initialize = initialize,
        .operation = operation,
        .checksum = checksum
    };
    return BIGNUM_ABS_BENCHMARK_STATUS_SUCCESS;
}
