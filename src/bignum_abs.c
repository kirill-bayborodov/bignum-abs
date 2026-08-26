/**
 * @file bignum_abs.c
 * @brief C11 reference implementation of bignum_abs.
 * @details The unsigned core representation has no sign to flip. This reference
 * validates the caller-owned record, copies it to a private candidate, computes
 * the canonical length, clears the unused suffix, and publishes only after all
 * validation has completed. The candidate gives byte-for-byte unchanged output
 * on every rejected call and has O(BIGNUM_CAPACITY) time and space complexity.
 */
#include "bignum_abs.h"
#include <string.h>

/**
 * @brief Finds the canonical length of a validated fixed-capacity candidate.
 * @details The scan walks from the declared high word toward zero and stops at
 * the first non-zero word. It does not inspect outside the validated length and
 * never modifies the candidate.
 * @param value [in] Caller-owned candidate with len <= BIGNUM_CAPACITY.
 * @return The number of significant words, in the inclusive range 0..capacity.
 */
static size_t normalized_length(const bignum_t *value)
{
    size_t length = value->len;
    while (length != 0U && value->words[length - 1U] == UINT64_C(0)) {
        --length;
    }
    return length;
}

/**
 * @brief Clears words outside a normalized bignum length.
 * @details Clearing the suffix establishes the representation invariant that
 * unused storage cannot contain stale data. The prefix is deliberately preserved.
 * @param value [in,out] Private candidate owned by the caller of this helper.
 * @param length [in] Valid normalized length no greater than BIGNUM_CAPACITY.
 */
static void clear_unused_words(bignum_t *value, size_t length)
{
    if (length < BIGNUM_CAPACITY) {
        memset(&value->words[length], 0,
               (BIGNUM_CAPACITY - length) * sizeof(value->words[0]));
    }
}

bignum_abs_status_t bignum_abs(bignum_t *num)
{
    bignum_t candidate;
    size_t length;

    /* Validate before copying or publishing so error paths cannot partially write. */
    if (num == NULL) {
        return BIGNUM_ABS_ERROR_NULL_ARG;
    }
    if (num->len > BIGNUM_CAPACITY) {
        return BIGNUM_ABS_ERROR_LENGTH;
    }

    candidate = *num;
    length = normalized_length(&candidate);
    clear_unused_words(&candidate, length);
    candidate.len = length;

    /* No failure remains after validation; publish the complete candidate atomically. */
    *num = candidate;
    return BIGNUM_ABS_SUCCESS;
}
