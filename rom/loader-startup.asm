  include "loader-definitions.asm"
  section .text
  org .ROM_BEGIN
  include "loader-vectors.asm"

; =================================================================
.UNIMPLEMENTED_EXCEPTION_HANDLER:
.UNIMPLEMENTED_TRAP_HANDLER:
  rte

; =================================================================
.TRAP0:
  ; trap 0 -- "reset"
  ; trashes everything
  move.l #.RAM_SIZE,A7
  jmp .ROM_ENTRY_POINT

.TRAP1:
  ; trap 1 -- read D0 bytes from DUART.A, into (A0)
  ; input   D0 : count of bytes to read
  ; input   A0 : address to read bytes into
  ; trashes D1
.trap1_read_byte:
  tst.l D0
  beq.s .trap1_read_complete
.trap1_read_byte_wait:
  btst.b #0,.DUART_REG_SRA
  beq.s .trap1_read_byte_wait
  move.b .DUART_REG_RHRA,D1
  move.b D1,(A0)+
  subq.l #1,D0
  bra.s .trap1_read_byte
.trap1_read_complete:
  rte

; =================================================================
.ROM_ENTRY_POINT:
  ; disable interrupts
  or.w #$0700,SR

  ; copy vector table to RAM
  lea (.ROM_VECTOR_TABLE),A0
  lea (.VECTOR_TABLE_BEGIN),A1
  move.l #.VECTOR_TABLE_COUNT,D0
  bra .vector_copy_loop
.vector_copy_loop:
  move.l (A0)+,(A1)+
  dbf D0,.vector_copy_loop

  ; initialize 68681
  move.b #$A0,.DUART_REG_CRA       ; enable extended TX rates
  move.b #$80,.DUART_REG_CRA       ; enable extended RX rates
  move.b #$80,.DUART_REG_ACR       ; extended bit rate select
  move.b #$88,.DUART_REG_CSRA      ; 115.2 Kbps
  move.b #%00000000,.DUART_REG_IMR ; disable interrupts from 68681
  move.b #%00010011,.DUART_REG_MRA ; MR1A - 8 bit mode, no parity
  move.b #%00000111,.DUART_REG_MRA ; MR2A - bit length 1.0; normal mode
  move.b #%11110000,.DUART_REG_ACR ; ACR - no interrupt on input change; timer = xtal/16; extended bit rate set
  move.b #%00000101,.DUART_REG_CRA ; CRA - enable transmitter & receiver
  ;move.b #64,.DUART_REG_IVR        ; IVR - use user interrupt 64 for DUART
  move.b #$04,.DUART_REG_CTU       ; CTU - counter timer = 1152
  move.b #$80,.DUART_REG_CTL       ; CTL - which makes ~100 Hz ticks
  move.b .DUART_REG_SCC,D0         ; start counter/timer (interrupt is masked, so we won't actually see these)

.jump_to_c
  ; jump to C code
  jsr loader_main
  jmp .jump_to_c
