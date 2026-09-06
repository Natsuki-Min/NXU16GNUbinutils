#objdump: -dr
#name: nX-U16 native expressions, modifiers, and branch relaxation

.*: +file format elf32-nxu16

Disassembly of section .text:

00000000 <_start>:
   0:	0007 .*	MOV     R0, 0x07
   2:	0100 .*	MOV     R1, 0x00
			2: R_NXU16_LO8	.data\+0x9
   4:	0200 .*	MOV     R2, 0x00
			4: R_NXU16_HI8	.data\+0x9
   6:	923a .*	SLL     R2, 3
   8:	9410 .*	L       R4, 0x0000
   a:	0000 .*
			a: R_NXU16_MEM_Dadr	.data\+0xe
   c:	ffce .*	PUSH    ELR EPSW LR EA *
   e:	ff8e .*	POP     PC PSW LR EA *
  10:	c901 .*	BEQ     14 <near_target>
  12:	fe8f .*	NOP

00000014 <near_target>:
  14:	f000 .*	B       0 <_start>
  16:	0000 .*
			14: R_NXU16_FLASH_Cadr	.text\+0x144
.*
00000144 <far_target>:
 144:	c902 .*	BEQ     14a <far_target\+0x6>
 146:	f000 .*	B       0 <_start>
 148:	0000 .*
			146: R_NXU16_FLASH_Cadr	_start
 14a:	f001 .*	BL      0 <_start>
 14c:	0000 .*
			14a: R_NXU16_FLASH_Cadr	.text\+0x144
  14e:	9021 .*\tST      R0, 0x\[ER2\]
