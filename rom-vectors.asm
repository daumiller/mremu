ROM_VECTOR_TABLE:
  dc.l $100000                              ; 0 @boot supervisor SP (end of RAM+1)
  dc.l ROM_ENTRY_POINT                      ; 1 @boot entry point
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 2 bus error
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 3 address error
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 4 illegal instruction
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 5 divide by 0
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 6 CHK instruction
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 7 TRAPV instruction
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 8 privilege violation
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 9 trace
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 10 line 1010 emulator
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 11 line 1111 emulator
  dc.l $000000,$000000                      ; 12-13 (reserved)
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 14 format error (68010+)
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 15 uninitialized interrupt vector
  dc.l $000000,$000000,$000000,$000000      ; 16-19 (reserved)
  dc.l $000000,$000000,$000000,$000000      ; 20-23 (reserved)
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 24 spurious interrupt
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 25 level 1 interrupt autovector
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 26 level 2 interrupt autovector
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 27 level 3 interrupt autovector
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 28 level 4 interrupt autovector
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 29 level 5 interrupt autovector
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 30 level 6 interrupt autovector
  dc.l UNIMPLEMENTED_EXCEPTION_HANDLER      ; 31 level 7 interrupt autovector
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 32 trap 0
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 33 trap 1
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 34 trap 2
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 35 trap 3
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 36 trap 4
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 37 trap 5
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 38 trap 6
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 39 trap 7
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 40 trap 8
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 41 trap 9
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 42 trap 10
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 43 trap 11
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 44 trap 12
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 45 trap 13
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 46 trap 14
  dc.l UNIMPLEMENTED_TRAP_HANDLER           ; 47 trap 15
  dc.l $000000,$000000,$000000,$000000      ; 48-51(reserved)
  dc.l $000000,$000000,$000000,$000000      ; 52-55 (reserved)
  dc.l $000000,$000000,$000000,$000000      ; 56-59 (reserved)
  dc.l $000000,$000000,$000000,$000000      ; 60-63 (reserved)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ;  64- 71 user interrupt vectors (( 1/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ;  72- 79 user interrupt vectors (( 2/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ;  80- 87 user interrupt vectors (( 3/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ;  88- 95 user interrupt vectors (( 4/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ;  96-103 user interrupt vectors (( 5/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 104-111 user interrupt vectors (( 6/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 112-119 user interrupt vectors (( 7/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 120-127 user interrupt vectors (( 8/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 128-135 user interrupt vectors (( 9/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 136-143 user interrupt vectors ((10/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 144-151 user interrupt vectors ((11/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 152-159 user interrupt vectors ((12/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 160-167 user interrupt vectors ((13/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 168-175 user interrupt vectors ((14/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 176-183 user interrupt vectors ((15/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 184-191 user interrupt vectors ((16/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 192-199 user interrupt vectors ((17/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 200-207 user interrupt vectors ((18/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 208-215 user interrupt vectors ((19/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 216-223 user interrupt vectors ((20/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 224-231 user interrupt vectors ((21/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 232-239 user interrupt vectors ((22/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 240-247 user interrupt vectors ((23/24)x8)
  dc.l $000000,$000000,$000000,$000000,$000000,$000000,$000000,$000000 ; 248-255 user interrupt vectors ((24/24)x8)
