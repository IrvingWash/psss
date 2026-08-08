.psx
.create "./bin/main.bin", 0x80010000

.org 0x80010000

IO_BASE_ADDR equ 0x1F80

GP0 equ 0x1810 ; 1F80_1810_h
GP1 equ 0x1814 ; 1F80_1814_h

la $sp, 0x00103CF0 ; Stack Pointer

Main:
    lui $a0, IO_BASE_ADDR ; load upper part of $a0 with IO_BASE_ADDR, so we have 1F80xxxx in $a0

    ; ==========================================================================
    ; Setup display control
    ; ==========================================================================
    ; Reset GPU
    li $t1, 0x00000000 ; Command to reset GPU is 0x00 and we have no parameters
    sw $t1, GP1($a0)   ; Store word from $t1 into GP1 offsetted by $a0
    ; Display Enable
    li $t1, 0x03000000 ; 0x03 is Display Enable command, 0 parameter is "on"
    sw $t1, GP1($a0)
    ; Display Mode 320x420, 15-bit, NTSC
    li $t1, 0x08000001 ; 0x08 is Display Mode, 1 in LSB is "horizontal res - 320"
    sw $t1, GP1($a0)
    ; Horizontal Range
    li $t1, 0x06C60260 ; 0x06 is Horizontal Range, and I copy pasted the params, so...
    sw $t1, GP1($a0)
    ; Vertical Range
    li $t1, 0x07042018 ; 0x07 is Vertical Range
    sw $t1, GP1($a0)

    ; ==========================================================================
    ; Setup VRAM Access
    ; ==========================================================================
    ; Drawing Mode Settings
    li $t1, 0xE1000400
    sw $t1, GP0($a0)
    ; Drawing Area Top-Left
    li $t1, 0xE3000000
    sw $t1, GP0($a0)
    ; Drawing Area Bottom-Right
    li $t1, 0xE403BD3F
    sw $t1, GP0($a0)
    ; Drawing Area Offset
    li $t1, 0xE5000000
    sw $t1, GP0($a0)

    ; ==========================================================================
    ; Clear the Screen
    ; ==========================================================================
    li $t1, 0x02FF0000 ; Set color to blue, 0x02 is "Fill Rectangle in VRAM"
    sw $t1, GP0($a0)
    li $t1, 0x00000000 ; Top-left corner coordinate
    sw $t1, GP0($a0)
    li $t1, 0x00EF013F ; width and height - 239 x 319
    sw $t1, GP0($a0)

    ; ==========================================================================
    ; Draw Flat-shaded Triangle
    ; ==========================================================================
    addiu $s4, -(4 * 7) ; We have 7 parameters, each a WORD
    ; Color
    li $t0, 0xFF4472
    sw $t0, 0($sp)
    ; x1
    li $t0, 200
    sw $t0, 4($sp)
    ; y1
    li $t0, 40
    sw $t0, 8($sp)
    ; x2
    li $t0, 288
    sw $t0, 12($sp)
    ; y2
    li $t0, 56
    sw $t0, 16($sp)
    ; x3
    li $t0, 224
    sw $t0, 20($sp)
    ; y3
    li $t0, 200
    sw $t0, 24($sp)
    jal DrawFlatTriangle
    nop

LoopForever:
    j LoopForever
    nop

End:

; $sp + 0  - Color
; $sp + 4  - x1
; $sp + 8  - y1
; $sp + 12 - x2
; $sp + 16 - y2
; $sp + 20 - x3
; $sp + 24 - y3
DrawFlatTriangle:
    lui $t0, 0x2000   ; 0x20 - flat-shaded triangle
    lw  $t1, 0($sp)   ; Load color from the stack
    nop
    or  $t8, $t0, $t1 ; $t8 |= $t0 | $t1, we combine because the first packet is primitive type + color
    sw  $t8, GP0($a0)

    lw   $t1, 4($sp)
    lw   $t2, 8($sp)
    nop
    sll  $t2, $t2, 16     ; Make room for the x coordinate. We hade 0xYYYY, now we have 0xYYYY0000
    andi $t1, $t1, 0xFFFF ; Erase upper bits, because we expect x1 to be 16 bit in the lower bit:
    ;   0xAB030032
    ; & 0x0000FFFF
    ;   ----------
    ;   0x00000032
    or   $t8, $t1, $t2    ; Combine XXXX and YYYY
    sw   $t8, GP0($a0)    ; Send to the GPU

    lw   $t1, 12($sp)
    lw   $t2, 16($sp)
    nop
    sll  $t2, $t2, 16
    andi $t1, $t1, 0xFFFF
    or   $t8, $t1, $t2
    sw   $t8, GP0($a0)

    lw   $t1, 20($sp)
    lw   $t2, 24($sp)
    nop
    sll  $t2, $t2, 16
    andi $t1, $t1, 0xFFFF
    or   $t8, $t1, $t2
    sw   $t8, GP0($a0)

    addiu $sp, $sp, (4 * 7)

    jr $ra
    nop

.close
