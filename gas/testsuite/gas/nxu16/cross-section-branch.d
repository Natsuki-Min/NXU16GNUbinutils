#objdump: -dr
#name: nX-U16 cross-section branch relaxation

.*: +file format elf32-nxu16

Disassembly of section .text.first:

00000000 <cross_section_start>:
   0:	f000 .*\tB       0 <cross_section_start>
   2:	0000 .*
			0: R_NXU16_FLASH_Cadr	.text.second
   4:	c902 .*\tBEQ     a <cross_section_start\+0xa>
   6:	f000 .*\tB       0 <cross_section_start>
   8:	0000 .*
			6: R_NXU16_FLASH_Cadr	.text.second

Disassembly of section .text.second:

00000000 <cross_section_target>:
   0:	fe8f .*\tNOP
