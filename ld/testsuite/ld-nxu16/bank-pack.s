	.section .text.large,"ax",@progbits
	.global large
	.type large,@function
large:
	.space 0xff00
	.size large,.-large

	.section .text.medium,"ax",@progbits
	.global medium
	.type medium,@function
medium:
	.space 0x200
	.size medium,.-medium

	.section .text.small,"ax",@progbits
	.global small
	.type small,@function
small:
	.space 0x100
	.size small,.-small
