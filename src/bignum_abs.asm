; @file bignum_abs.asm
; @brief Independent System V AMD64 implementation of bignum_abs.
; @details The function validates the caller-owned record before any store. A valid
; record cannot fail during normalization, so the implementation trims zero most
; significant words, clears the unused tail, and publishes the new length in place.
; This avoids a temporary record and full-record copies while preserving the error
; transaction: NULL and over-capacity inputs are returned unchanged.
;
; C/ASM boundary: rdi = bignum_t *num; rax = bignum_abs_status_t. The record is
; an array of BIGNUM_CAPACITY uint64_t words followed by a size_t len field.
; System V AMD64 callee-saved registers are not modified; rsp is unchanged and no
; stack storage is used. Flags are caller-clobbered and carry no return semantics.

%define BIGNUM_CAPACITY       32
%define BIGNUM_WORD_SIZE      8
%define BIGNUM_LEN_OFFSET     (BIGNUM_CAPACITY * BIGNUM_WORD_SIZE)
%define STATUS_SUCCESS         0
%define STATUS_NULL_ARG       -1
%define STATUS_LENGTH         -2

section .text

global bignum_abs

bignum_abs:
    ; Validate before stores so every rejected call leaves the record untouched.
    test    rdi, rdi
    jz      .error_null
    mov     r8, [rdi + BIGNUM_LEN_OFFSET]
    cmp     r8, BIGNUM_CAPACITY
    ja      .error_length

    ; Find the canonical length by removing zero words from the high end.
    mov     rcx, r8
.trim:
    test    rcx, rcx
    jz      .trim_done
    cmp     qword [rdi + rcx*8 - 8], 0
    jne     .trim_done
    dec     rcx
    jmp     .trim
.trim_done:

    ; Clear only the unused suffix. The validated prefix is never overwritten.
    mov     rdx, rcx
    lea     rdx, [rdi + rdx*8]
    mov     r9, BIGNUM_CAPACITY
    sub     r9, rcx
    xor     eax, eax
.clear_tail:
    test    r9, r9
    jz      .publish
    mov     [rdx], rax
    add     rdx, BIGNUM_WORD_SIZE
    dec     r9
    jmp     .clear_tail

.publish:
    ; All potentially failing validation is complete; publish length last.
    mov     [rdi + BIGNUM_LEN_OFFSET], rcx
    mov     eax, STATUS_SUCCESS
    ret

.error_null:
    mov     eax, STATUS_NULL_ARG
    ret

.error_length:
    mov     eax, STATUS_LENGTH
    ret

section .note.GNU-stack noalloc noexec nowrite progbits

; end of file
