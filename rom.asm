; motorola syntax
  include "rom-defs.asm"
  org ROM_BEGIN
  include "rom-vectors.asm"

UNIMPLEMENTED_EXCEPTION_HANDLER:
UNIMPLEMENTED_TRAP_HANDLER:
  rte

ROM_ENTRY_POINT:
  ; disable interrupts
  or.w #$0700,SR

  ; copy vector table to RAM
  lea (ROM_VECTOR_TABLE),A0
  lea (VECTOR_TABLE_BEGIN),A1
  move.l #VECTOR_TABLE_COUNT,D0
  bra vector_copy_loop
vector_copy_move:
  move.l (A0)+,(A1)+
vector_copy_loop:
  dbf D0,vector_copy_move

duart_initialize:
  ; initialize 68681
  move.b #%00000000,DUART_REG_IMR ; disable interrupts from 68681
  move.b #%00010011,DUART_REG_MRA ; MR1A - 8 bit mode, no parity
  move.b #%00000111,DUART_REG_MRA ; MR2A - bit length 1.0; normal mode
  move.b #%11110000,DUART_REG_ACR ; ACR - no interrupt on input change; timer = xtal/16; extended bit rate set
  move.b #%00000101,DUART_REG_CRA ; CRA - enable transmitter & receiver
  ;move.b #64,DUART_REG_IVR        ; IVR - use user interrupt 64 for DUART
  move.b #$04,DUART_REG_CTU       ; CTU - counter timer = 1152
  move.b #$80,DUART_REG_CTL       ; CTL - which makes ~100 Hz ticks
  move.b DUART_REG_SCC,D0         ; start counter/timer (interrupt is masked, so we won't actually see these)

  ; print welcome message
  lea (welcome_string),A0
  jsr DUART_PRINT_STRING

  ; ask for input
  lea (prompt_string),A0
  jsr DUART_PRINT_STRING
  move.l #$0F0000,A0
  move.l A0,A1 ; store our command pointer for later
  move.l #$FF,D0
  move.l #$01,D1
  jsr DUART_READ_LINE

  ; ack that we read this command
  lea (ack_string),A0
  jsr DUART_PRINT_STRING
  move.b #$22,D0
  jsr DUART_PRINT_CHARACTER
  move.l A1,A0 ; our read command  
  jsr DUART_PRINT_STRING
  move.b #$22,D0
  jsr DUART_PRINT_CHARACTER
  lea (newline_string),A0
  jsr DUART_PRINT_STRING

  ; runloop
  clr.l D0
entry_loop:
  addq.l #$1,D0
  jmp entry_loop

; input D0.b -- character to print
DUART_PRINT_CHARACTER:
  btst.b #2,DUART_REG_SRA     ; check bit 2 (%0100) in status register, if 1 tx is ready
  beq.s DUART_PRINT_CHARACTER ; if 0, wait for tx to be ready
  move.b D0,DUART_REG_THRA    ; print character
  rts

; input   A0 -- address of null-terminated string
; trashes A0,D0
DUART_PRINT_STRING:
  move.b (A0)+,D0               ; read byte from address into D0; increment address
  beq.s duart_print_string_done ; if read byte is zero, we're done
duart_print_string_wait:
  btst.b #2,DUART_REG_SRA       ; check bit 2 (%0100) in status register, if 1 tx is ready
  beq.s duart_print_string_wait ; if 0, wait for tx to be ready
  move.b D0,DUART_REG_THRA      ; write byte to tx register
  bra DUART_PRINT_STRING        ; move on to next byte
duart_print_string_done:
  rts

; input   A0 -- address where string can be read to
; input   D0 -- maximum length of string to read (not including null byte)
; input   D1 -- whether to echo output while reading
; output  D2 -- length of read string
; trashes D3,A0
DUART_READ_LINE:
  btst.b #0,DUART_REG_SRA     ; check bit 0 (%0001) in status register, if 1 rx has a byte ready
  beq.s DUART_READ_LINE       ; if not, wait
  move.b DUART_REG_RHRA,D3    ; read byte from receiver
  tst.b D1                    ; if(echo_output)
  beq.s duart_read_line_skip_echo
duart_read_line_echo:
  btst.b #2,DUART_REG_SRA     ; see if ready for transmit
  beq.s duart_read_line_echo  ; wait for it to be ready
  move.b D3,DUART_REG_THRA    ; echo character
duart_read_line_skip_echo:
  cmp.b #$D,D3                ; if read == '\r'
  beq.s DUART_READ_LINE       ; move on to next character
  cmp.b #$A,D3                ; if read == '\n'
  beq.s duart_read_line_zero_and_return
  move.b D3,(A0)+             ; store this character, and increment pointer
  addq.l #1,D2                ; ++read_length
  cmp.l D0,D2                 ; if(read_length == maximum_length)
  beq.s duart_read_line_zero_and_return
  bra.s DUART_READ_LINE       ; else keep reading
duart_read_line_zero_and_return:
  move.b #0,(A0)+ ; terminate string
  rts

newline_string:
  dc.b $0D, $0A, $00
welcome_string:
  dc.b "hello 68k world!", $0D, $0A, $00
prompt_string:
  dc.b "command > ", $00
ack_string:
  dc.b "processing ", $00
