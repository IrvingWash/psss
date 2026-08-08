.psx
.create "./bin/main.bin", 0x80010000

.org 0x80010000

IO_BASE_ADDR equ 0x1F80

GP0 equ 0x1810 ; 1F80_1810_h
GP1 equ 0x1814 ; 1F80_1814_h

Main:
    lui $t0, IO_BASE_ADDR ; load upper part of $t0 with IO_BASE_ADDR, so we have 1F80xxxx in $t0

    ; ==========================================================================
    ; Setup display control
    ; ==========================================================================
    ; Reset GPU
    li $t1, 0x00000000 ; Command to reset GPU is 0x00 and we have no parameters
    sw $t1, GP1($t0)   ; Store word from $t1 into GP1 offsetted by $t0
    ; Display Enable
    li $t1, 0x03000000 ; 0x03 is Display Enable command, 0 parameter is "on"
    sw $t1, GP1($t0)
    ; Display Mode 320x420, 15-bit, NTSC
    li $t1, 0x08000001 ; 0x08 is Display Mode, 1 in LSB is "horizontal res - 320"
    sw $t1, GP1($t0)
    ; Horizontal Range
    li $t1, 0x06C60260 ; 0x06 is Horizontal Range, and I copy pasted the params, so...
    sw $t1, GP1($t0)
    ; Vertical Range
    li $t1, 0x07042018 ; 0x07 is Vertical Range
    sw $t1, GP1($t0)

    ; ==========================================================================
    ; Setup VRAM Access
    ; ==========================================================================
    ; Drawing Mode Settings
    li $t1, 0xE1000400
    sw $t1, GP0($t0)
    ; Drawing Area Top-Left
    li $t1, 0xE3000000
    sw $t1, GP0($t0)
    ; Drawing Area Bottom-Right
    li $t1, 0xE403BD3F
    sw $t1, GP0($t0)
    ; Drawing Area Offset
    li $t1, 0xE5000000
    sw $t1, GP0($t0)

    ; ==========================================================================
    ; Clear the Screen
    ; ==========================================================================
    li $t1, 0x02FF0000 ; Set color to blue, 0x02 is "Fill Rectangle in VRAM"
    sw $t1, GP0($t0)
    li $t1, 0x00000000 ; Top-left corner coordinate
    sw $t1, GP0($t0)
    li $t1, 0x00EF013F ; width and height - 239 x 319
    sw $t1, GP0($t0)

    ; ==========================================================================
    ; Draw Flat-shaded Triangle
    ; ==========================================================================
    lui $a0, IO_BASE_ADDR
    li $s0, 0x0000FF
    li $s1, 50
    li $s2, 50
    li $s3, 100
    li $s4, 30
    li $s5, 110
    li $s6, 100
    jal DrawFlatTriangle
    nop

LoopForever:
    j LoopForever
    nop

End:

; $a0 - IO_BASE_ADDR
; $s0 - Color - 0xBBGGRR
; $s1 - x1
; $s2 - y1
; $s3 - x2
; $s4 - y2
; $s5 - x3
; $s6 - y3
DrawFlatTriangle:
    lui $t0, 0x2000   ; 0x20 - flat-shaded triangle
    or  $t1, $t0, $s0 ; $t1 |= $t0 | $s0
    sw  $t1, GP0($a0)

    sll  $s2, $s2, 16     ; Make room for the x coordinate. We hade 0xYYYY, now we have 0xYYYY0000
    andi $s1, $s1, 0xFFFF ; Erase upper bits, because we expect x1 to be 16 bit in the lower bit:
    ;   0xAB030032
    ; & 0x0000FFFF
    ;   ----------
    ;   0x00000032
    or   $t1, $s1, $s2    ; Combine XXXX and YYYY
    sw   $t1, GP0($a0)    ; Send to the GPU

    sll  $s4, $s4, 16
    andi $s3, $s3, 0xFFFF
    or   $t1, $s3, $s4
    sw   $t1, GP0($a0)

    sll  $s6, $s6, 16
    andi $s5, $s5, 0xFFFF
    or   $t1, $s5, $s6
    sw   $t1, GP0($a0)

    jr $ra
    nop

.close
