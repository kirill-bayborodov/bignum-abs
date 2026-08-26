/**
 * @file test_bignum_abs.c
 * @brief Deterministic contract tests for bignum_abs.
 * @details The suite runs against C11 and YASM implementations and checks
 * status mapping, transactional failures, canonical zeroization, boundary
 * lengths, idempotence, and numeric-value preservation with fixed seeds.
 */
#include "bignum_abs.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/** @brief Initializes a record with deterministic words and a chosen length. */
static void seed_record(bignum_t *value, size_t length, uint64_t seed)
{
    size_t index;
    memset(value, 0, sizeof(*value));
    value->len = length;
    for (index = 0U; index < BIGNUM_CAPACITY; ++index) {
        value->words[index] = seed + UINT64_C(0x9e3779b97f4a7c15) * (index + 1U);
    }
}

/** @brief Checks canonical length and zeroized unused words. */
static void assert_canonical(const bignum_t *value)
{
    size_t index;
    assert(value->len <= BIGNUM_CAPACITY);
    if (value->len != 0U) {
        assert(value->words[value->len - 1U] != 0U);
    }
    for (index = value->len; index < BIGNUM_CAPACITY; ++index) {
        assert(value->words[index] == UINT64_C(0));
    }
}

/** @brief Verifies success, expected normalized length, and preserved value. */
static void expect_success(bignum_t *value, size_t expected_length)
{
    uint64_t before_words[BIGNUM_CAPACITY];
    size_t index;
    memcpy(before_words, value->words, sizeof(before_words));
    assert(bignum_abs(value) == BIGNUM_ABS_SUCCESS);
    assert(value->len == expected_length);
    assert_canonical(value);
    for (index = 0U; index < expected_length; ++index) {
        assert(value->words[index] == before_words[index]);
    }
}

/** @brief Exercises NULL and invalid-length failures transactionally. */
static void test_errors(void)
{
    bignum_t value;
    bignum_t before;
    assert(bignum_abs(NULL) == BIGNUM_ABS_ERROR_NULL_ARG);
    seed_record(&value, BIGNUM_CAPACITY + 1U, UINT64_C(1));
    before = value;
    assert(bignum_abs(&value) == BIGNUM_ABS_ERROR_LENGTH);
    assert(memcmp(&value, &before, sizeof(value)) == 0);
}

/** @brief Exercises zero, trimmed, and maximum-capacity records. */
static void test_boundaries(void)
{
    bignum_t value;
    seed_record(&value, 0U, UINT64_C(17));
    expect_success(&value, 0U);

    seed_record(&value, 3U, UINT64_C(23));
    value.words[2] = UINT64_C(0);
    expect_success(&value, 2U);

    seed_record(&value, BIGNUM_CAPACITY, UINT64_C(31));
    expect_success(&value, BIGNUM_CAPACITY);
}

/** @brief Confirms that a canonical result is byte-stable on repeated calls. */
static void test_idempotence(void)
{
    bignum_t value;
    bignum_t before;
    seed_record(&value, 7U, UINT64_C(41));
    value.words[6] = UINT64_C(0);
    expect_success(&value, 6U);
    before = value;
    assert(bignum_abs(&value) == BIGNUM_ABS_SUCCESS);
    assert(memcmp(&value, &before, sizeof(value)) == 0);
}

/** @brief Runs every valid declared length with deterministic tail trimming. */
static void test_fixed_seed_matrix(void)
{
    bignum_t value;
    size_t length;
    uint64_t seed = UINT64_C(0x123456789abcdef0);
    for (length = 0U; length <= BIGNUM_CAPACITY; ++length) {
        size_t expected = length;
        seed_record(&value, length, seed + length);
        if (length != 0U && (length % 3U) == 0U) {
            value.words[length - 1U] = UINT64_C(0);
            expected = length - 1U;
        }
        expect_success(&value, expected);
    }
}

int main(void)
{
    test_errors();
    test_boundaries();
    test_idempotence();
    test_fixed_seed_matrix();
    puts("bignum_abs deterministic tests: OK");
    return 0;
}
