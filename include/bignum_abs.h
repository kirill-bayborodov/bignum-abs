/**
 * @file bignum_abs.h
 * @brief Public API for canonical normalization of an unsigned bignum_t.
 * @details bignum_t stores a non-negative fixed-capacity integer and therefore
 * has no sign bit to change. bignum_abs validates the record, removes zero words
 * above the most significant non-zero word, clears the unused tail, and commits
 * the canonical length. The caller owns the record and allocates its storage;
 * the function allocates no memory and uses no mutable global state. The API is
 * safe for concurrent calls only when each thread owns a distinct record.
 */
#ifndef BIGNUM_ABS_H
#define BIGNUM_ABS_H

#include <bignum.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reports validation or completion status for bignum_abs.
 * @details A success status means the caller-owned record was normalized. Both
 * error statuses guarantee that no store was made to the record.
 */
typedef enum bignum_abs_status {
    BIGNUM_ABS_SUCCESS = 0, /**< Normalization completed; the record is canonical and its unused words are zero. */
    BIGNUM_ABS_ERROR_NULL_ARG = -1, /**< The required caller-owned record pointer was NULL; no output exists and retry is valid after supplying storage. */
    BIGNUM_ABS_ERROR_LENGTH = -2 /**< The input length exceeded BIGNUM_CAPACITY; the record is unchanged and retry is valid after repairing the length. */
} bignum_abs_status_t;

/**
 * @brief Normalizes the absolute value of an unsigned bignum in place.
 * @details Since bignum_t represents only non-negative values, the numeric value
 * is preserved. The function first validates num and its length. On valid input
 * it scans from the declared high word, trims zero words, clears the unused
 * suffix, and writes the normalized length last. There is no allocation and the
 * operation is constant-space; the running time is O(BIGNUM_CAPACITY) worst case.
 * @param num [in,out] Caller-owned, writable bignum_t storage. It must be
 * non-NULL, valid for the complete call, and not concurrently modified. The
 * input and output are the same object; no ownership is transferred.
 * @return BIGNUM_ABS_SUCCESS when num is canonical; BIGNUM_ABS_ERROR_NULL_ARG
 * when num is NULL; or BIGNUM_ABS_ERROR_LENGTH when num->len is too large.
 * @pre num is either NULL or points to a writable bignum_t whose storage remains
 * live for the duration of the call.
 * @post On success, num->len is at most BIGNUM_CAPACITY, zero has len == 0,
 * words at indices len..BIGNUM_CAPACITY-1 are zero, and the represented value
 * is unchanged. On either error, num is byte-for-byte unchanged.
 * @warning The function is thread-safe for independent records only; callers
 * must synchronize access when threads share the same bignum_t.
 * @complexity O(BIGNUM_CAPACITY) time and O(1) auxiliary space.
 */
bignum_abs_status_t bignum_abs(bignum_t *num);

#ifdef __cplusplus
}
#endif

#endif /* BIGNUM_ABS_H */
