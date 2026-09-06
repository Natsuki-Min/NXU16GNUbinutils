#objdump: -dr
#name: nX-U16 backward branch and local modifier boundaries

.*: +file format elf32-nxu16

Disassembly of section .text:

00000000 <_start>:
.*
 +fe:	c880 .*	BNE     0 <_start>
.*
00000204 <local_data>:
.*
	+205: R_NXU16_LO8	.text\+0x204
	+206: R_NXU16_HI8	.text\+0x204
.*
	+207: R_NXU16_LO8	.text\+0x204
.*
	+209: R_NXU16_HI8	.text\+0x204
.*
