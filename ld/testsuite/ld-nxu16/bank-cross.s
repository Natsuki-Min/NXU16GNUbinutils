	.section .text.cross,"ax",@progbits
	.p2align 16
	.global crosses_bank
	.type crosses_bank,@function
crosses_bank:
	.space 0x200
	.size crosses_bank,.-crosses_bank
