.intel_syntax noprefix

.global test_loop_mov
.global test_loop_nop
.global test_loop_dec

test_loop_mov:
        xor rax, rax
.loop_mov:
        mov BYTE PTR [rdi + rax * 1], al
        inc rax
        cmp rax, rsi
        jb .loop_mov
        ret

test_loop_nop:
        xor rax, rax
.loop_nop:
        .byte 0x0f, 0x1f, 0x00
        inc rax
        cmp rax, rsi
        jb .loop_nop
        ret

test_loop_dec:
        dec rsi
        cmp rsi, 0
        jne test_loop_dec
        ret
