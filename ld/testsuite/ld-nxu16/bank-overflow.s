	.section .text.too_large,"ax",@progbits
	.global too_large
	.type too_large,@function
too_large:
	.space 0x10001
	.size too_large,.-too_large
