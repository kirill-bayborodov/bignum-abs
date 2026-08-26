/**
 * @file test_bignum_abs_mt.c
 * @brief Multithreaded independent-object tests for bignum_abs.
 * @details Each worker owns one record and repeatedly normalizes it. The test
 * checks reentrancy and canonical output without sharing mutable records.
 */
#include "bignum_abs.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define WORKERS 8U
#define ITERATIONS 128U

typedef struct abs_worker {
    bignum_t value;
    size_t expected;
    int failed;
} abs_worker_t;

/** @brief Executes repeated normalization for one independently owned record. */
static void *worker_main(void *opaque)
{
    abs_worker_t *worker = (abs_worker_t *)opaque;
    size_t iteration;
    for (iteration = 0U; iteration < ITERATIONS; ++iteration) {
        if (bignum_abs(&worker->value) != BIGNUM_ABS_SUCCESS ||
            worker->value.len != worker->expected) {
            worker->failed = 1;
            break;
        }
    }
    return NULL;
}

int main(void)
{
    abs_worker_t workers[WORKERS];
    pthread_t threads[WORKERS];
    size_t index;
    for (index = 0U; index < WORKERS; ++index) {
        memset(&workers[index], 0, sizeof(workers[index]));
        workers[index].value.len = (index % BIGNUM_CAPACITY) + 1U;
        workers[index].expected = workers[index].value.len;
        workers[index].value.words[workers[index].expected - 1U] = index + 1U;
        assert(pthread_create(&threads[index], NULL, worker_main, &workers[index]) == 0);
    }
    for (index = 0U; index < WORKERS; ++index) {
        assert(pthread_join(threads[index], NULL) == 0);
        assert(workers[index].failed == 0);
    }
    puts("bignum_abs multithread tests: OK");
    return 0;
}
