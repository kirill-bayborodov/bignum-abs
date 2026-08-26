/**
 * @file test_bignum_abs_extra.c
 * @brief Extended randomized value-preservation tests for bignum_abs.
 * @details Fixed-seed records cover every valid length and arbitrary stale tails.
 * The test verifies that normalization changes representation only, never value.
 */
#include "bignum_abs.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/** @brief Produces a deterministic pseudo-random 64-bit value. */
static uint64_t next_word(uint64_t *state)
{
    *state ^= *state << 7U;
    *state ^= *state >> 9U;
    *state ^= *state << 8U;
    return *state;
}

/** @brief Returns the canonical length without modifying the input. */
static size_t canonical_length(const bignum_t *value)
{
    size_t length = value->len;
    while (length != 0U && value->words[length - 1U] == UINT64_C(0)) {
        --length;
    }
    return length;
}

/** @brief Checks representation invariants after a successful call. */
static void check_canonical(const bignum_t *value, size_t expected)
{
    size_t index;
    assert(value->len == expected);
    for (index = expected; index < BIGNUM_CAPACITY; ++index) {
        assert(value->words[index] == UINT64_C(0));
    }
    if (expected != 0U) {
        assert(value->words[expected - 1U] != UINT64_C(0));
    }
}

int main(void)
{
    uint64_t state = UINT64_C(0x6a09e667f3bcc909);
    size_t iteration;
    for (iteration = 0U; iteration < 10000U; ++iteration) {
        bignum_t value;
        bignum_t before;
        size_t index;
        size_t expected;
        value.len = (size_t)(next_word(&state) % (BIGNUM_CAPACITY + 1U));
        for (index = 0U; index < BIGNUM_CAPACITY; ++index) {
            value.words[index] = next_word(&state);
        }
        before = value;
        expected = canonical_length(&value);
        assert(bignum_abs(&value) == BIGNUM_ABS_SUCCESS);
        check_canonical(&value, expected);
        for (index = 0U; index < expected; ++index) {
            assert(value.words[index] == before.words[index]);
        }
    }

    {
        bignum_t invalid;
        bignum_t before;
        memset(&invalid, 0xa5, sizeof(invalid));
        invalid.len = BIGNUM_CAPACITY + 1U;
        before = invalid;
        assert(bignum_abs(&invalid) == BIGNUM_ABS_ERROR_LENGTH);
        assert(memcmp(&invalid, &before, sizeof(invalid)) == 0);
    }
    puts("bignum_abs extended tests: OK");
    return 0;
}
