/* Definitions for decoding the nxu16 opcode table.
   Copyright (C) 2009-2021 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */
#include <stdint.h>
#include <stddef.h>
// nxu16_Argument types
typedef enum {
    ARG_NONE = 0,
    ARG_REG_Rn,
    ARG_REG_ERn,
    ARG_REG_QRn,
    ARG_REG_XRn,
    ARG_REG_CRn,
    ARG_REG_CERn,
    ARG_REG_CXRn,
    ARG_REG_CQRn,
    ARG_REG_SP,
    ARG_REG_PSW,
    ARG_REG_ELR,
    ARG_REG_ECSR,
    ARG_REG_EPSW,
    ARG_REG_LIST,
    
    ARG_MEM_EA,
    ARG_MEM_EAP,
    ARG_MEM_ERn,
    //-----CAN ACCEPT SYMBOL-----
    ARG_IMM3,
    ARG_IMM6,
    ARG_IMM7_SIGNED,
    ARG_IMM8,

    ARG_MEM_ERnDisp16,
    ARG_MEM_FPDisp6,
    ARG_MEM_BPDisp6,
    ARG_MEM_Dadr,
    ARG_FLASH_Cadr,
    ARG_FLASH_Radr,
    ARG_SYMBOL,
    ARG_SPECIAL
} nxu16_ArgType;

// nxu16_Argument structure
typedef struct {
    nxu16_ArgType type;   // nxu16_Argument type
    uint32_t mask;  // Bitmask for argument position
} nxu16_Arg;

// Handler function type
typedef void (*InstrHandler)(void);

// Opcode combination structure
typedef struct {
    uint32_t opcode;          // Base opcode value
    uint32_t opcode_mask;     // Mask for relevant opcode bits
    uint8_t opcode_length;    // Length in bits (up to 32)
    nxu16_Arg *args;                // nxu16_Argument combinations
    size_t num_args;          // Number of arguments
    InstrHandler handler;     // Custom handler (default NULL)
} nxu16_OpcodeCombination;

// nxu16_opc_info_t structure
typedef struct {
    const char *name;               // nxu16_opc_info_t name
    nxu16_OpcodeCombination *combinations; // Valid opcode combinations
    size_t num_combinations;        // Number of combinations
} nxu16_opc_info_t;

// Macros for clean initialization
#define INSTR(name, ...) \
    { name, (nxu16_OpcodeCombination[]){__VA_ARGS__}, sizeof((nxu16_OpcodeCombination[]){__VA_ARGS__}) / sizeof(nxu16_OpcodeCombination) }

#define COMB(opcode, mask, len, handler, ...) \
    { opcode, mask, len, (nxu16_Arg[]){__VA_ARGS__}, sizeof((nxu16_Arg[]){__VA_ARGS__}) / sizeof(nxu16_Arg), handler }

#define ARG(type, mask) \
    { type, mask }

#define HANDLER(func) \
    (InstrHandler)func


extern const nxu16_opc_info_t nxu16_opc_info[62];
