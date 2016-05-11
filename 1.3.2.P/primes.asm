
; calculate 500 first primes and print vertically in 10 columns

segment     .bss
    primes  resq    500     ; 500 primes
    number  resq    1       ; the number of primes

segment     .text
    global  main
    extern  printf

main:
    .cand   equ 0                   ; prime candidate
    push    rbp                     ; prepare stack frame
    mov     rbp, rsp
    sub     rsp, 16

    mov     rax, 2                  ; first prime
    mov     [primes + 0 * 8], rax   ; saving first prime
    mov     rax, 3                  ; second prime
    mov     [primes + 1 * 8], rax   ; saving second prime
    mov     rax, 2                  ; number of primes
    mov     [number], rax           ; saving number of primes so far

    mov     rax, 3
    mov     [rsp + .cand], rax

.loop_prime:
    mov     rdi, [rsp + .cand]      ; get current candidate
    add     rdi, 2                  ; next candidate
    mov     [rsp + .cand], rdi      ; save current candidate
    mov     rsi, primes             ; pointer to primes
    xor     eax, eax
    call    is_prime

    cmp     rax, 0
    je      .loop_prime             ; it is not a prime test next

    mov     rdi, [rsp + .cand]      ; the confirmed prime
    mov     rax, [number]           ; the position
    mov     [primes + rax * 8], rdi ; save prime in array
    inc     rax                     ; increase number of known primes
    mov     [number], rax           ; save number of known primes
    cmp     rax, 500
    jl      .loop_prime

    ; now print
    mov     rdi, primes
    xor     eax, eax
    call    print_primes

    xor     eax, eax
    leave
    ret

segment     .text
is_prime:   ; (candidate, *primes)
    .candidate  equ 0               ; the prime candidate

    push    rbp                     ; prepare stack frame
    mov     rbp, rsp
    sub     rsp, 16                 ; making room for .candidate and keeping stack aligned in 16 bits

    mov     [rsp + .candidate], rdi ; saving candidate
    mov     r8, rsi                 ; saving pointer to primes
    mov     rcx, 0                  ; rcx will point to current prime testing the candidate

.try_next:
    inc     rcx                     ; next prime

    mov     rdi, [r8 + rcx * 8]     ; load current prime in rdi

    mov     rax, [rsp + .candidate] ; rdx:rax will have candidate to divide by current prime
    xor     edx, edx

    idiv    rdi                     ; rdx:rax / rdi ==> rax has cotient / rdx has remainder

    cmp     rdx, 0                  ; remainder is zero
    je      .not_prime              ; it is not prime

    cmp     rax, rdi                ; if quotient greater thatn divisor,
    jg      .try_next               ; need to test next prime

    mov     rax, 1                  ; else it is a prime return 1
    leave
    ret

.not_prime:                         ; if not prime return 0
    xor     eax, eax
    leave
    ret

print_primes:   ; (*primes)
segment     .data
    .format:
        db "%04ld ", 0
    .newline:
        db 0xa, 0
segment     .text
    .primes equ 0                   ; the primes pointer
    .line   equ 8
    .column equ 16

    push    rbp                     ; prepare stack frame
    mov     rbp, rsp
    sub     rsp, 32                 ; making room for pointer and size

    mov     [rsp + .primes], rdi    ; saving pointer to primes
    mov     rcx, 0                  ; rcx will point to current prime to print
    mov     [rsp + .line], rcx      ; line iterator
    mov     [rsp + .column], rcx    ; column iterator

.print_next:
    mov     rdi, [rsp + .line]
    mov     rsi, [rsp + .column]
    xor     eax, eax
    call    calculate_index         ; rax will have the index

    lea     rdi, [.format]
    mov     rdx, [rsp + .primes]
    mov     rsi, [rdx + rax * 8]
    xor     eax, eax
    call    printf

    mov     rsi, [rsp + .column]
    inc     rsi                     ; next column
    mov     [rsp + .column], rsi    ; save next column
    cmp     rsi, 10
    jne     .print_next

    lea     rdi, [.newline]
    xor     eax, eax
    call    printf

    mov     rsi, 0                  ; carriage return
    mov     [rsp + .column], rsi    ; save column
    mov     rdi, [rsp + .line]
    inc     rdi                     ; next line
    mov     [rsp + .line], rdi      ; save next line
    cmp     rdi, 50                 ; verify end
    jl      .print_next

    xor     eax, eax
    leave
    ret

calculate_index: ; line, column
    push    rbp                     ; prepare stack frame
    mov     rbp, rsp

    mov     rcx, rdi
    mov     rax, rsi
    imul    rax, 50
    add     rax, rcx

    leave
    ret
