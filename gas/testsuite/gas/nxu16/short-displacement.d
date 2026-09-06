#objdump: -dr
#name: nX-U16 compact ER12/ER14 signed-displacement selection

.*: +file format elf32-nxu16

Disassembly of section .text:

00000000 <short_displacement>:
   0:	b002 .*	L       ER0, .*
   2:	b27e .*	L       ER2, .*
   4:	d403 .*	L       R4, .*
   6:	d57d .*	L       R5, .*
   8:	b684 .*	ST      ER6, .*
   a:	b8fc .*	ST      ER8, .*
   c:	da85 .*	ST      R10, .*
   e:	dbfb .*	ST      R11, .*
  10:	a0a8 .*	L       ER0, 0x0002\[ER10\]
  12:	0002 .*
