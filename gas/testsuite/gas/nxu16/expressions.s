	.text
	.global _start
_start:
	mov r0, 1 + 2 * 3
	mov r1, lo8(data_symbol + 3)
	mov r2, hi8(data_symbol + 3)
	sll r2, (1 + 2)
	l r4, data_symbol + (4 * 2)
	push elr, epsw, lr, ea
	pop psw, pc, lr, ea
	beq near_target
	nop
near_target:
	b far_target
	.space 300
far_target:
	bne _start
	bl far_target
	st r0, [er2]

	.data
	.byte lo8(data_symbol), hi8(data_symbol)
	.long data_symbol
data_symbol:
	.word 0
