.psx
.create "./bin/divide.bin", 0x80010000

.org 0x80010000

; Divides t0 by t1

Main:
    li $t0, 0x001B ; num = 27
    li $t1, 0x0003 ; den = 3
    li $t2, 0      ; res

Loop:
    sub  $t0, $t1
    addi $t2, 0x0001 ; This is a shorthand for addi $t2, $t2, 0x0001
    bge  $t0, $t1, Loop
    nop

End:

.close
