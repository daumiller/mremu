  section .text

  ; at entry:
  ; A0 = address of string containing command line arguments
  ; A7 = stack pointer specifically for this process
_startup:
  ; move.l A0,-(A7)
  pea -2(PC)
  lea (main.w,pc),A0
	jsr (A0)

  ; (sp).l == return status code
  ; exit:
    ; trap #1

.loop:
  bra .loop
