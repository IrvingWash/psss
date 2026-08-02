.psx
.create "./bin/fill_mem.bin", 0x80010000

.org 0x80010000

BASE_ADDR equ 0x0000

Main:
    li $t0, 0xA000
    li $t1, 0xA0FF
    li $t2, 0x11

Loop:
    sb   $t2, BASE_ADDR($t0)
    addi $t0, $t0, 1
    blt  $t0, $t1, Loop

End:

.close
