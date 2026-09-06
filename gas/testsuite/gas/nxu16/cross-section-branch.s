	.section .text.first,"ax",@progbits
	.global cross_section_start
cross_section_start:
	b cross_section_target
	bne cross_section_target

	.section .text.second,"ax",@progbits
cross_section_target:
	nop
