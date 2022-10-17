RAM_BEGIN equ $000000
RAM_END   equ $0FFFFF
RAM_SIZE  equ $100000 

ROM_BEGIN equ $E00000
ROM_END   equ $EFFFFF
ROM_SIZE  equ $100000

IO_BEGIN  equ $F00000
IO_END    equ $FFFFFF
IO_SIZE   equ $100000

DUART_BEGIN equ $F00000
DUART_END   equ $F0001F
DUART_SIZE  equ $000020

VECTOR_TABLE_BEGIN equ $000000
VECTOR_TABLE_END   equ $0003FF
VECTOR_TABLE_SIZE  equ $000400
VECTOR_TABLE_COUNT equ 256
VECTOR_USER_BASE   equ 64
VECTOR_USER_MAX    equ 255

; bus address = DUART_BEGIN + (relative_address * 2) + 1
DUART_REG_MRA   equ $F00001 ; chA mode (MR1A/MR2A)      | read/write
DUART_REG_SRA   equ $F00003 ; chA status                | read
DUART_REG_CSRA  equ $F00003 ; chA clock select          | write
DUART_REG_MISR  equ $F00005 ; masked interrupt status   | read
DUART_REG_CRA   equ $F00005 ; chA command               | write
DUART_REG_RHRA  equ $F00007 ; chA rx holding            | read
DUART_REG_THRA  equ $F00007 ; chA tx holding            | write
DUART_REG_IPCR  equ $F00009 ; input port change         | read
DUART_REG_ACR   equ $F00009 ; auxiliary control         | write
DUART_REG_ISR   equ $F0000B ; interrupt status          | read
DUART_REG_IMR   equ $F0000B ; interrupt mask            | write
DUART_REG_CTU   equ $F0000D ; counter/timer upper byte  | read/write
DUART_REG_CTL   equ $F0000F ; counter/timer lower byte  | read/write
DUART_REG_MRB   equ $F00011 ; chB mode (MR1B/MR2B)      | read/write
DUART_REG_SRB   equ $F00013 ; chB status                | read
DUART_REG_CSRB  equ $F00013 ; chB clock select          | write
DUART_REG_CRB   equ $F00015 ; chB command               | write
DUART_REG_RHBB  equ $F00017 ; chB rx holding            | read
DUART_REG_THRB  equ $F00017 ; chB tx holding            | write
DUART_REG_IVR   equ $F00019 ; interrupt vector          | read/write
DUART_REG_IP    equ $F0001B ; input port                | read
DUART_REG_OPCR  equ $F0001B ; output port configuration | write
DUART_REG_SCC   equ $F0001D ; start counter/timer       | "read" (address triggered)
DUART_REG_SOPBC equ $F0001D ; set output port bits      | write
DUART_REG_STC   equ $F0001F ; stop counter/timer        | "read" (address triggered)
DUART_REG_COPBC equ $F0001F ; clear output port bits    | write
