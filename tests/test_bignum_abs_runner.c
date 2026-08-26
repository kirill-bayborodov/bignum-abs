/**
 * @file test_bignum_abs_runner.c
 * @brief Distribution smoke test for the public bignum_abs API.
 * @details This runner is compiled from the generated distribution and exits
 * successfully only when the public header, library symbol and status contract
 * produce a canonical normalized record.
 */
#include "bignum_abs.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    bignum_t value;
    memset(&value, 0, sizeof(value));
    value.len = 2U;
    value.words[0] = UINT64_C(0x1234);
    value.words[1] = UINT64_C(0);
    assert(bignum_abs(&value) == BIGNUM_ABS_SUCCESS);
    assert(value.len == 1U);
    assert(value.words[0] == UINT64_C(0x1234));
    assert(value.words[1] == UINT64_C(0));
    puts("bignum_abs distribution runner: OK");
    return 0;
}
