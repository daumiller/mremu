#NO_APP
	.file	"boot.c"
	.text
	.align	2
	.globl	startup
	.type	startup, @function
startup:
	move.l #15728647,%a0
	move.b #104,(%a0)
	move.l #15728647,%a0
	move.b #101,(%a0)
	move.l #15728647,%a0
	move.b #108,(%a0)
	move.l #15728647,%a0
	move.b #108,(%a0)
	move.l #15728647,%a0
	move.b #111,(%a0)
	move.l #15728647,%a0
	move.b #32,(%a0)
	move.l #15728647,%a0
	move.b #99,(%a0)
	move.l #15728647,%a0
	move.b #33,(%a0)
	move.l #15728647,%a0
	move.b #13,(%a0)
	move.l #15728647,%a0
	move.b #10,(%a0)
	nop
	rts
	.size	startup, .-startup
	.ident	"GCC: (GNU) 10.2.0"
