#objdump: -dr
#name: nX-U16 register-selected far memory prefixes

.*: +file format elf32-nxu16

Disassembly of section .text:

00000000 <far_memory>:
   0:	905f .*	L       R3, R5:\[ER6\]
   2:	9360 .*
   4:	90ff .*	L       ER4, R15:\[ER8\]
   6:	9482 .*
   8:	909f .*	ST      R7, R9:\[ER10\]
   a:	97a1 .*
   c:	902f .*	ST      ER12, R2:\[ER14\]
   e:	9ce3 .*
  10:	906f .*	L       R0, R6:0002\[ER4\]
  12:	9048 .*
  14:	0002 .*
