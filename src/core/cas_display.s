! Assembly function to partially refresh GB-LCD
! (Credits to @DasHeiligeDoenerhuhn)
.global _gb_lcd_refresh

.align 2
_refresh_gb_lcd:
  mov.l r10, @-r15
  mov.l r9, @-r15
  mov.l r8, @-r15
	sts.l	pr, @-r15

  mov.l r7, @-r15
  mov.l r6, @-r15
  mov.l r5, @-r15
  mov.l r4, @-r15
  
  ! Call first magic function which mostly uses our parameters
  add #-1, r5
  mov.l magic_fun_1, r8
  jsr @r8
  add #-1, r7

  ! Call the second magic function
  mov.l magic_fun_2, r8
  jsr @r8
  mov #0x2c, r4

  ! Reload parameters and constants
  mov.l pixel_row_width, r8
  mov.l @r15+, r4 ! x_start
  mov.l @r15+, r5 ! x_end
  mov.l @r15+, r6 ! y_start
  mov.l @r15+, r7 ! y_end

  mov r4, r9
  mov r6, r10
  
  ! Calculate offset
  mul.l r6, r8
  sts MACL, r6
  add r6, r4 ! offset stored in r4

  ! Load serial screen data register into r6
  mov.l serial_screen_d_register, r6

  mov r10, r2 ! iteration variable y
  
  ! Calculate first pixel address
  mov.l vram, r0
  add r4, r0 ! First pixel in row

.y_loop:
  mov r0, r3 ! tmp pixel
  mov r9, r1 ! iteration variable x

.x_loop:
  ! Move current pixel into screen data register
  mov.l @r3+, r4
  add #2, r1

  ! Check if at last pixel in row and jump if not
  cmp/hs r5, r1
  bf/s .x_loop
  mov.l r4, @r6
  ! end of x_loop

  add #1, r2

  ! Check if at last row and jump if not
  cmp/hs r7, r2
  bf/s .y_loop
  add r8, r0 ! Add pixel_row_width to first pixel in row
  ! end of y_loop

  lds.l @r15+, pr
  mov.l @r15+, r8
  mov.l @r15+, r9
  mov.l @r15+, r10

  rts
  nop

.align 4
magic_fun_1:
  .long 0x80038068

magic_fun_2:
  .long 0x80038040

serial_screen_d_register:
  .long 0xb4000000

vram:
  .long 0x8c000000

pixel_row_width:
  .long 640
