/* Definitions for nxu16 opcodes.
   Copyright (C) 2009-2021 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "opcode/nxu16.h"

/* Standard instructions */
const nxu16_opc_info_t nxu16_opc_info[62] =//IMM MUST AFTER REG,otherwise it will be recognized as symbol
  {
    /* === Arithmetic Instructions === */
     INSTR("add",
        COMB(0x8001, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        COMB(0x1000, 0xF000, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM8, 0x00FF)
        ),
        COMB(0xF006, 0xF11F, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_REG_ERn, 0x00E0)
        ),
        COMB(0xE080, 0xF180, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_IMM7_SIGNED, 0x007F)
        ),
        /* == Special Instructions == */
        //add sp,#signed8
        COMB(0xE100, 0xFF00, 2, NULL,
            ARG(ARG_REG_SP, 0x0000),
            ARG(ARG_IMM8, 0x00FF)
        )
    ),

    // ADDC instructions
    INSTR("addc",
        COMB(0x8006, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        COMB(0x6000, 0xF000, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM8, 0x00FF)
        )
    ),
    INSTR("sub",
        COMB(0x8008, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        )
    ),

    // SUBC instructions
    INSTR("subc",
        COMB(0x8009, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        )
    ),
    /* === Logical Instructions === */
    INSTR("and",
        // AND Rn,Rn
        COMB(0x8002, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // AND Rn,#imm8
        COMB(0x2000, 0xF000, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM8, 0x00FF)
        ),
        // AND psw,#imm8
        COMB(0xEB00,0xFF00,2,NULL,
            ARG(ARG_REG_PSW,0x0000),
            ARG(ARG_IMM8,0x00FF)
        )
    ),

    INSTR("or",
        // OR Rn,Rn
        COMB(0x8003, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // OR Rn,#imm8
        COMB(0x3000, 0xF000, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM8, 0x00FF)
        ),
        // OR psw,#imm8
        COMB(0xED00,0xFF00,2,NULL,
            ARG(ARG_REG_PSW,0x0000),
            ARG(ARG_IMM8,0x00FF)
        )
    ),

    INSTR("xor",
        // XOR Rn,Rn
        COMB(0x8004, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // XOR Rn,#imm8
        COMB(0x4000, 0xF000, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM8, 0x00FF)
        )
    ),

    /* === Compare Instructions === */
    INSTR("cmp",
        // CMP Rn,Rn
        COMB(0x8007, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // CMP Rn,#imm8
        COMB(0x7000, 0xF000, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM8, 0x00FF)
        ),
        // CMP ERn,ERn
        COMB(0xF007, 0xF11F, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_REG_ERn, 0x00E0)
        )
    ),

    INSTR("cmpc",
        // CMPC Rn,Rn
        COMB(0x8005, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // CMPC Rn,#imm8
        COMB(0x5000, 0xF000, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM8, 0x00FF)
        )
    ),

    /* === Move Instructions === */
    INSTR("mov",
        // MOV Rn,Rn
        COMB(0x8000, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // MOV Rn,#imm8
        COMB(0x0000, 0xF000, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM8, 0x00FF)
        ),
        // MOV ERn,ERn
        COMB(0xF005, 0xF11F, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_REG_ERn, 0x00E0)
        ),
        // MOV ERn,#imm7
        COMB(0xE000, 0xF180, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_IMM7_SIGNED, 0x007F)
        ),
        // MOV ERn,SP
        COMB(0xA01A, 0xF1FF, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_REG_SP,0x0000)
        ),
        // MOV SP,ERn
        COMB(0xA10A, 0xFF1F, 2, NULL,
            ARG(ARG_REG_ERn, 0x00E0),
            ARG(ARG_REG_SP, 0x0000)
        ),
        // MOV PSW,Rn
        COMB(0xA00B, 0xFF0F, 2, NULL,
            ARG(ARG_REG_PSW, 0x0000),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // MOV PSW,#imm8
        COMB(0xE900, 0xFF00, 2, NULL,
            ARG(ARG_REG_PSW, 0x0000),
            ARG(ARG_IMM8, 0x00FF)
        ),
        // MOV Rn,PSW
        COMB(0xA003, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_PSW, 0x0000)
        ),
        // MOV Rn,ECSR
        COMB(0xA007, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_ECSR, 0x0000),
        ),
        // MOV ECSR,Rn
        COMB(0xA00F, 0xFF0F, 2, NULL,
            ARG(ARG_REG_ECSR,0x0000),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // MOV ELR,ERn
        COMB(0xA00D, 0xF1FF, 2, NULL,
            ARG(ARG_REG_ELR,0x0000),
            ARG(ARG_REG_ERn, 0x0E00)
        ),
        // MOV ERn,ELR
        COMB(0xA005, 0xF1FF, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_REG_ELR,0x0000)
        ),
        // MOV Rn,EPSW
        COMB(0xA004, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_EPSW, 0x0000)
        ),
        // MOV EPSW,Rn
        COMB(0xA00C, 0xFF0F, 2, NULL,
            ARG(ARG_REG_EPSW, 0x0000),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // MOV CRn,Rn
        COMB(0xA00E, 0xF00F, 2, NULL,
            ARG(ARG_REG_CRn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // MOV Rn,CRn
        COMB(0xA006, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_CRn, 0x00F0)
        )
    ),

    /* === Load/Store Instructions === */
    // L instructions (memory to register)
    INSTR("l",
        // L ERn,[EA]
        COMB(0x9032, 0xF1FF, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // L ERn,[EA+]
        COMB(0x9052, 0xF1FF, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // L ERn,[ERn]
        COMB(0x9002, 0xF11F, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_MEM_ERn, 0x00E0)
        ),
        // L ERn,Disp16[ERn]
        COMB(0xA0080000, 0xF11F0000, 4, NULL,
            ARG(ARG_REG_ERn, 0x0E000000),
            ARG(ARG_MEM_ERnDisp16, 0x00E0FFFF)
        ),
        // L ERn,Disp6[BP]
        COMB(0xB000, 0xF1C0, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_MEM_BPDisp6, 0x003F)
        ),
        // L ERn,Disp6[FP]
        COMB(0xB040, 0xF1C0, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_MEM_FPDisp6, 0x003F)
        ),
        // L ERn,Dadr
        COMB(0x90120000, 0xF1FF0000, 4, NULL,
            ARG(ARG_REG_ERn, 0x0E000000),
            ARG(ARG_MEM_Dadr, 0x0000FFFF)
        ),
        // L Rn,[EA]
        COMB(0x9030, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // L Rn,[EA+]
        COMB(0x9050, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // L Rn,[ERn]
        COMB(0x9000, 0xF01F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_ERn, 0x00E0)
        ),
        // L Rn,Disp16[ERn]
        COMB(0x90080000, 0xF01F0000, 4, NULL,
            ARG(ARG_REG_Rn, 0x0F000000),
            ARG(ARG_MEM_ERnDisp16, 0x00E0FFFF)
        ),
        // L Rn,Disp6[BP]
        COMB(0xD000, 0xF0C0, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_MEM_BPDisp6, 0x003F)
        ),
        // L Rn,Disp6[FP]
        COMB(0xD040, 0xF0C0, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_MEM_FPDisp6, 0x003F)
        ),
        // L Rn,Dadr
        COMB(0x90100000, 0xF0FF0000, 4, NULL,
            ARG(ARG_REG_Rn, 0x0F000000),
            ARG(ARG_MEM_Dadr, 0x0000FFFF)
        ),
        // L XRn,[EA]
        COMB(0x9034, 0xF3FF, 2, NULL,
            ARG(ARG_REG_XRn, 0x0C00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // L XRn,[EA+]
        COMB(0x9054, 0xF3FF, 2, NULL,
            ARG(ARG_REG_XRn, 0x0C00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // L QRn,[EA]
        COMB(0x9036, 0xF7FF, 2, NULL,
            ARG(ARG_REG_QRn, 0x0800),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // L QRn,[EA+]
        COMB(0x9056, 0xF7FF, 2, NULL,
            ARG(ARG_REG_QRn, 0x0800),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // L CERn,[EA]
        COMB(0xF02D, 0xF1FF, 2, NULL,
            ARG(ARG_REG_CERn, 0x0E00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // L CERn,[EA+]
        COMB(0xF03D, 0xF1FF, 2, NULL,
            ARG(ARG_REG_CERn, 0x0E00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // L CRn,[EA]
        COMB(0xF00D, 0xF0FF, 2, NULL,
            ARG(ARG_REG_CRn, 0x0F00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // L CRn,[EA+]
        COMB(0xF01D, 0xF0FF, 2, NULL,
            ARG(ARG_REG_CRn, 0x0F00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // L CXRn,[EA]
        COMB(0xF04D, 0xF3FF, 2, NULL,
            ARG(ARG_REG_CXRn, 0x0C00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // L CXRn,[EA+]
        COMB(0xF05D, 0xF3FF, 2, NULL,
            ARG(ARG_REG_CXRn, 0x0C00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // L CQRn,[EA]
        COMB(0xF06D, 0xF7FF, 2, NULL,
            ARG(ARG_REG_CQRn, 0x0800),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // L CQRn,[EA+]
        COMB(0xF07D, 0xF7FF, 2, NULL,
            ARG(ARG_REG_CQRn, 0x0800),
            ARG(ARG_MEM_EAP, 0x0000)
        )
    ),

    // ST instructions (register to memory)
    INSTR("st",
        // ST ERn,[EA]
        COMB(0x9033, 0xF1FF, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // ST ERn,[EA+]
        COMB(0x9053, 0xF1FF, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // ST ERn,[ERn]
        COMB(0x9003, 0xF11F, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_MEM_ERn, 0x00E0)
        ),
        // ST ERn,Disp16[ERn]
        COMB(0xA0090000, 0xF11F0000, 4, NULL,
            ARG(ARG_REG_ERn, 0x0E000000),
            ARG(ARG_MEM_ERnDisp16, 0x00E0FFFF)
        ),
        // ST ERn,Disp6[BP]
        COMB(0xB080, 0xF1C0, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_MEM_BPDisp6, 0x003F)
        ),
        // ST ERn,Disp6[FP]
        COMB(0xB0C0, 0xF1C0, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_MEM_FPDisp6, 0x003F)
        ),
        // ST ERn,Dadr
        COMB(0x90130000, 0xF1FF0000, 4, NULL,
            ARG(ARG_REG_ERn, 0x0E000000),
            ARG(ARG_MEM_Dadr, 0x0000FFFF)
        ),
        // ST Rn,[EA]
        COMB(0x9031, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // ST Rn,[EA+]
        COMB(0x9051, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // ST Rn,[ERn]
        COMB(0x9001, 0xF01F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_ERn, 0x00E0)
        ),
        // ST Rn,Disp16[ERn]
        COMB(0x90090000, 0xF01F0000, 4, NULL,
            ARG(ARG_REG_Rn, 0x0F000000),
            ARG(ARG_MEM_ERnDisp16, 0x00E0FFFF)
        ),
        // ST Rn,Disp6[BP]
        COMB(0xD080, 0xF0C0, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_MEM_BPDisp6, 0x003F)
        ),
        // ST Rn,Disp6[FP]
        COMB(0xD0C0, 0xF0C0, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_MEM_FPDisp6, 0x003F)
        ),
        // ST Rn,Dadr
        COMB(0x90110000, 0xF0FF0000, 4, NULL,
            ARG(ARG_REG_Rn, 0x0F000000),
            ARG(ARG_MEM_Dadr, 0x0000FFFF)
        ),
        // ST XRn,[EA]
        COMB(0x9035, 0xF3FF, 2, NULL,
            ARG(ARG_REG_XRn, 0x0C00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // ST XRn,[EA+]
        COMB(0x9055, 0xF3FF, 2, NULL,
            ARG(ARG_REG_XRn, 0x0C00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // ST QRn,[EA]
        COMB(0x9037, 0xF7FF, 2, NULL,
            ARG(ARG_REG_QRn, 0x0800),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // ST QRn,[EA+]
        COMB(0x9057, 0xF7FF, 2, NULL,
            ARG(ARG_REG_QRn, 0x0800),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // ST CERn,[EA]
        COMB(0xF0AD, 0xF1FF, 2, NULL,
            ARG(ARG_REG_CERn, 0x0E00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // ST CERn,[EA+]
        COMB(0xF0BD, 0xF1FF, 2, NULL,
            ARG(ARG_REG_CERn, 0x0E00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // ST CRn,[EA]
        COMB(0xF08D, 0xF0FF, 2, NULL,
            ARG(ARG_REG_CRn, 0x0F00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // ST CRn,[EA+]
        COMB(0xF09D, 0xF0FF, 2, NULL,
            ARG(ARG_REG_CRn, 0x0F00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // ST CXRn,[EA]
        COMB(0xF0CD, 0xF3FF, 2, NULL,
            ARG(ARG_REG_CXRn, 0x0C00),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // ST CXRn,[EA+]
        COMB(0xF0DD, 0xF3FF, 2, NULL,
            ARG(ARG_REG_CXRn, 0x0C00),
            ARG(ARG_MEM_EAP, 0x0000)
        ),
        // ST CQRn,[EA]
        COMB(0xF0ED, 0xF7FF, 2, NULL,
            ARG(ARG_REG_CQRn, 0x0800),
            ARG(ARG_MEM_EA, 0x0000)
        ),
        // ST CQRn,[EA+]
        COMB(0xF0FD, 0xF7FF, 2, NULL,
            ARG(ARG_REG_CQRn, 0x0800),
            ARG(ARG_MEM_EAP, 0x0000)
        )
    ),

    /* === Stack Operations === */
    INSTR("push",
        // PUSH ERn
        COMB(0xF05E, 0xF1FF, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00)
        ),
        // PUSH QRn
        COMB(0xF07E, 0xF7FF, 2, NULL,
            ARG(ARG_REG_QRn, 0x0800)
        ),
        // PUSH Rn
        COMB(0xF04E, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00)
        ),
        // PUSH XRn
        COMB(0xF06E, 0xF3FF, 2, NULL,
            ARG(ARG_REG_XRn, 0x0C00)
        ),
        // PUSH register_list
        COMB(0xF0CE, 0xF0FF, 2, NULL,
            ARG(ARG_REG_LIST, 0x0F00)
        )
    ),

    INSTR("pop",
        // POP ERn
        COMB(0xF01E, 0xF1FF, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00)
        ),
        // POP QRn
        COMB(0xF03E, 0xF7FF, 2, NULL,
            ARG(ARG_REG_QRn, 0x0800)
        ),
        // POP Rn
        COMB(0xF00E, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00)
        ),
        // POP XRn
        COMB(0xF02E, 0xF3FF, 2, NULL,
            ARG(ARG_REG_XRn, 0x0C00)
        ),
        // POP register_list
        COMB(0xF08E, 0xF0FF, 2, NULL,
            ARG(ARG_REG_LIST, 0x0F00)
        )
    ),

    /* === Branch/Jump Instructions === */
    INSTR("b",
            // B ERn
        COMB(0xF002, 0xFF1F, 2, NULL,
            ARG(ARG_REG_ERn, 0x00E0)
        ),
        // B Cadr
        COMB(0xF0000000, 0xF0FF0000, 4, NULL,
            ARG(ARG_FLASH_Cadr,0x0F00FFFF)
        ),
        COMB(0xCE00, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))

    ),

    INSTR("bl",
            // BL ERn
        COMB(0xF003, 0xFF1F, 2, NULL,
            ARG(ARG_REG_ERn, 0x00E0)
        ),
        // BL Cadr
        COMB(0xF0010000, 0xF0FF0000, 4, NULL,
            ARG(ARG_FLASH_Cadr,0x0F00FFFF)
        )

    ),

    // Conditional branches
    INSTR("bge", COMB(0xC000, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("blt", COMB(0xC100, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("bgt", COMB(0xC200, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("ble", COMB(0xC300, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("bges", COMB(0xC400, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("blts", COMB(0xC500, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("bgts", COMB(0xC600, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("bles", COMB(0xC700, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("bne", COMB(0xC800, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("beq", COMB(0xC900, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("bnv", COMB(0xCA00, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("bov", COMB(0xCB00, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("bps", COMB(0xCC00, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("bns", COMB(0xCD00, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),
    INSTR("bal", COMB(0xCE00, 0xFF00, 2, NULL, ARG(ARG_FLASH_Radr, 0x00FF))),

    /* === Shift Instructions === */
    INSTR("sll",
        // SLL Rn,Rn
        COMB(0x800A, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // SLL Rn,#width
        COMB(0x900A, 0xF08F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM3, 0x0070)
        )
    ),

    INSTR("sllc",
        // SLLC Rn,Rn
        COMB(0x800B, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // SLLC Rn,#width
        COMB(0x900B, 0xF08F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM3, 0x0070)
        )
    ),

    INSTR("sra",
        // SRA Rn,Rn
        COMB(0x800E, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // SRA Rn,#width
        COMB(0x900E, 0xF08F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM3, 0x0070)
        )
    ),

    INSTR("srl",
        // SRL Rn,Rn
        COMB(0x800C, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // SRL Rn,#width
        COMB(0x900C, 0xF08F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM3, 0x0070)
        )
    ),

    INSTR("srlc",
        // SRLC Rn,Rn
        COMB(0x800D, 0xF00F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_REG_Rn, 0x00F0)
        ),
        // SRLC Rn,#width
        COMB(0x900D, 0xF08F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM3, 0x0070)
        )
    ),

    /* === Miscellaneous Instructions === */
    INSTR("nop", COMB(0xFE8F, 0xFFFF, 2, NULL)),
    INSTR("rt", COMB(0xFE1F, 0xFFFF, 2, NULL)),
    INSTR("rti", COMB(0xFE0F, 0xFFFF, 2, NULL)),
    INSTR("iceswi", COMB(0xFEFF, 0xFFFF, 2, NULL)),
    INSTR("rtice", COMB(0xFE6F, 0xFFFF, 2, NULL)),
    INSTR("rticepsw", COMB(0xFE7F, 0xFFFF, 2, NULL)),
    INSTR("brk", COMB(0xFFFF, 0xFFFF, 2, NULL)),
    INSTR("swi",
        COMB(0xE500, 0xFFC0, 2, NULL,//TODO:need special treat
            ARG(ARG_IMM6, 0x003F)
        )
    ),
    INSTR("extbw",
        COMB(0x810F, 0xF11F, 2, NULL,//TODO:need special treat
            ARG(ARG_REG_ERn, 0x0EE0)
        )
    ),
    INSTR("daa",
        COMB(0x801F, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00)
        )
    ),
    INSTR("das",
        COMB(0x803F, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00)
        )
    ),
    INSTR("neg",
        COMB(0x805F, 0xF0FF, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00)
        )
    ),
    INSTR("cplc", COMB(0xFECF, 0xFFFF, 2, NULL)),
    INSTR("ei", COMB(0xED08, 0xFFFF, 2, NULL)),
    INSTR("di", COMB(0xEBF7, 0xFFFF, 2, NULL)),
    INSTR("sc", COMB(0xED80, 0xFFFF, 2, NULL)),
    INSTR("rc", COMB(0xEB7F, 0xFFFF, 2, NULL)),

    /* === Bit Manipulation === */
    INSTR("sb",
        // SB Rn.bit_offset
        COMB(0xA000, 0xF08F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM3, 0x0070)
        ),
        // SB Dbitadr
        COMB(0xA0800000, 0xFF8F0000, 4, NULL,
            ARG(ARG_MEM_Dadr, 0x0000FFFF),
            ARG(ARG_IMM3, 0x00700000)
        )
    ),

    INSTR("rb",
        // RB Rn.bit_offset
        COMB(0xA002, 0xF08F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM3, 0x0070)
        ),
        // RB Dbitadr
        COMB(0xA0820000, 0xFF8F0000, 4, NULL,
            ARG(ARG_MEM_Dadr, 0x0000FFFF),
            ARG(ARG_IMM3, 0x00700000)
        )
    ),

    INSTR("tb",
        // TB Rn.bit_offset
        COMB(0xA001, 0xF08F, 2, NULL,
            ARG(ARG_REG_Rn, 0x0F00),
            ARG(ARG_IMM3, 0x0070)
        ),
        // TB Dbitadr
        COMB(0xA0810000, 0xFF8F0000, 4, NULL,
            ARG(ARG_MEM_Dadr, 0x0000FFFF),
            ARG(ARG_IMM3, 0x00700000)
        )
    ),

    /* === Address Calculation === */
    INSTR("lea",
        // LEA [ERn]
        COMB(0xF00A, 0xFF1F, 2, NULL,
            ARG(ARG_MEM_ERn, 0x00E0)
        ),
        // LEA Disp16[ERn]
        COMB(0xF00B0000, 0xFF1F0000, 4, NULL,
            ARG(ARG_MEM_ERnDisp16, 0x00E0FFFF)
        ),
        // LEA Dadr
        COMB(0xF00C0000, 0xFFFF0000, 4, NULL,
            ARG(ARG_MEM_Dadr, 0x0000FFFF)
        )
    ),

    /* === Math Operations === */
    INSTR("mul",
        COMB(0xF004, 0xF10F, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_REG_Rn, 0x00F0)
        )
    ),

    INSTR("div",
        COMB(0xF009, 0xF10F, 2, NULL,
            ARG(ARG_REG_ERn, 0x0E00),
            ARG(ARG_REG_Rn, 0x00F0)
        )
    ),

    /* === Memory Operations === */
    INSTR("inc",
        COMB(0xFE2F, 0xFFFF, 2, NULL,
            ARG(ARG_MEM_EA, 0x0000)
        )
    ),

    INSTR("dec",
        COMB(0xFE3F, 0xFFFF, 2, NULL,
            ARG(ARG_MEM_EA, 0x0000)
        )
    ),
    /* === DSR Prefix === */
    INSTR("dsr",

        COMB(0x900F,0xFF0F,2,NULL,ARG(ARG_REG_Rn,0x00F0)),
                COMB(0xE300,0xFF00,2,NULL,ARG(ARG_IMM8,0x00FF)),
        COMB(0xFE9F,0xFFFF,2,NULL)
    )

};
