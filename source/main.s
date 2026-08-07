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
    li $t1, 0x2000FFFF ; 2 - triangle, 0 - flat-shaded, 00FFFF - color
    sw $t1, GP0($t0)
    li $t1, 0x00320032 ; First vertex coordinate (50, 50)
    sw $t1, GP0($t0)
    li $t1, 0x001E0064 ; Second vertex (100, 30)
    sw $t1, GP0($t0)
    li $t1, 0x0064006E ; Third vertex (110, 100)
    sw $t1, GP0($t0)

    ; ==========================================================================
    ; Draw Flat-shaded Quad
    ; ==========================================================================
    li $t1, 0x28FF00FF
    sw $t1, GP0($t0)
    li $t1, 0x00960096
    sw $t1, GP0($t0)
    li $t1, 0x006400BE
    sw $t1, GP0($t0)
    li $t1, 0x00DC00A0
    sw $t1, GP0($t0)
    li $t1, 0x00CB0104
    sw $t1, GP0($t0)

    ; ==========================================================================
    ; Draw Gouraud-shaded Triangle
    ; ==========================================================================
    li $t1, 0x30FF31FF ; Gouraud-shaded triangle, Vertex1 color FF31FF
    sw $t1, GP0($t0)
    li $t1, 0x00B40014 ; Vertex1 coords
    sw $t1, GP0($t0)
    li $t1, 0x00A88332 ; Vertex2 color
    sw $t1, GP0($t0)
    li $t1, 0x006400A0 ; Vertex2 coords
    sw $t1, GP0($t0)
    li $t1, 0x0000FF00 ; Vertex3 color
    sw $t1, GP0($t0)
    li $t1, 0x00E6004B ; Vertex3 coords
    sw $t1, GP0($t0)

LoopForever:
    j LoopForever
    nop

End:

.close
