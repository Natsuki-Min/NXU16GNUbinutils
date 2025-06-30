/* Disassemble nxu16 instructions.
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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <stdio.h>
#include<stdint.h>
#include <string.h>
#include "opintl.h"
#include "libiberty.h"

#define STATIC_TABLE
#define DEFINE_TABLE

#include "opcode/nxu16.h"
#include "disassemble.h"


static fprintf_ftype fpr;
static void* stream;
unsigned int baddr=0;
/* Macros to extract operands from the instruction word.  */
#define OP_A(i) (i & 0x7)
#define OP_B(i) ((i >> 3) & 0x7)
#define OP_C(i) ((i >> 6) & 0x7)

/* Have zero padding at first if necessary.
   The number with maximum length is (1 << (binlen - 1)), equal to the smallest
   number. It is a power of 2, thus first digit cannot be hexadecimal.
   The number has binlen digits. So, hexlen = ceil(binlen/4).
*/

static const char *signedtohex(int n, int binlen) {
static char hex_buffer[100];
    // Check for zero case
    if (n == 0) {
        strcpy(hex_buffer, "0");
        return hex_buffer;
    }

    int ispositive = (n >> (binlen - 1)) == 0;

    // If negative, convert to two's complement
    if (!ispositive) {
        n = (1 << binlen) - (-n);
    }

    int i = 0;
    while (n && i < 100 - 1) {
        int digit = n & 0xF;
        hex_buffer[i++] = "0123456789ABCDEF"[digit];
        n >>= 4;
    }

    // Reverse the string
    hex_buffer[i--] = '\0';
    for (int j = 0; j < i; j++, i--) {
        char temp = hex_buffer[j];
        hex_buffer[j] = hex_buffer[i];
        hex_buffer[i] = temp;
    }

    // If original number was negative, prepend a '-'
    if (!ispositive) {
        memmove(hex_buffer + 1, hex_buffer, strlen(hex_buffer) + 1);
        hex_buffer[0] = '-';
    }

    return hex_buffer;
}



static uint8_t* decode(char* output, uint8_t* buf, uint32_t pc) {
	static const char* cond[16] = {"GE ", "LT ", "GT ", "LE ", "GES", "LTS", "GTS", "LES",
		"NE ", "EQ ", "NV ", "OV ", "PS ", "NS ", "   ", "NOP"};


	char dsr_prefix[30] = "0x";
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11111111) == 0b11100011) {
		int i = buf[0] >> 0 & 0b11111111;
		sprintf(dsr_prefix ,"%01X:" ,i) ;
		buf += 2;
	}
	if ((buf[0] & 0b00001111) == 0b00001111 && (buf[1] & 0b11111111) == 0b10010000) {
		int d = buf[0] >> 4 & 0b1111;
		sprintf(dsr_prefix ,"R%d:" ,d) ;
		buf += 2;
	}
	if ((buf[0] & 0b11111111) == 0b10011111 && (buf[1] & 0b11111111) == 0b11111110) {
		sprintf(dsr_prefix,"DSR:");
		buf += 2;
	}

	if ((buf[0] & 0b00001111) == 0b00000001 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
		buf += 2;
    sprintf(output, "ADD     R%d, R%d", n, m);
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b00010000) {
		int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
		buf += 2;
    sprintf(output, "ADD     R%d, %d", n, i);
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00000110 && (buf[1] & 0b11110001) == 0b11110000) {
		int m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "ADD     ER%d, ER%d", n * 2, m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b10000000) == 0b10000000 && (buf[1] & 0b11110001) == 0b11100000) {
		int i = buf[0] >> 0 & 0b1111111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "ADD     ER%d,%d", n * 2, (i << 25 >> 25));
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00000110 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "ADDC    R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b01100000) {
		int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "ADDC    R%d, %d", n, i);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00000010 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "AND     R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b00100000) {
		int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "AND     R%d, 0x%02X", n, i);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00000111 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
		buf += 2;
    sprintf(output, "CMP     R%d, R%d", n, m);
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b01110000) {
		int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
		buf += 2;
    sprintf(output, "CMP     R%d, 0x%02X", n, i);
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00000101 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "CMPC    R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b01010000) {
		int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "CMPC    R%d, 0x%02X", n, i);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00000101 && (buf[1] & 0b11110001) == 0b11110000) {
		int m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "MOV     ER%d, ER%d", n * 2, m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b10000000) == 0b00000000 && (buf[1] & 0b11110001) == 0b11100000) {
		int i = buf[0] >> 0 & 0b1111111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "MOV     ER%d, %d", n * 2, (i<<25)>>25);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00000000 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b00000000) {
		int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     R%d, 0x%02X", n, i);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00000011 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "OR      R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b00110000) {
		int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "OR      R%d, 0x%02X", n, i);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00000100 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "XOR     R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b01000000) {
		int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "XOR     R%d, 0x%02X", n, i);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00000111 && (buf[1] & 0b11110001) == 0b11110000) {
		int m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "CMP     ER%d, ER%d", n * 2, m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001000 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
		buf += 2;
    sprintf(output, "SUB     R%d, R%d", n, m);
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001001 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "SUBC    R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001010 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
		buf += 2;
    sprintf(output, "SLL     R%d, R%d", n, m);
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b00001010 && (buf[1] & 0b11110000) == 0b10010000) {
		int n = buf[1] >> 0 & 0b1111, w = buf[0] >> 4 & 0b111;
    sprintf(output, "SLL     R%d, %d", n, w);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001011 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "SLLC    R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b00001011 && (buf[1] & 0b11110000) == 0b10010000) {
		int n = buf[1] >> 0 & 0b1111, w = buf[0] >> 4 & 0b111;
    sprintf(output, "SLLC    R%d, %d", n, w);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001110 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "SRA     R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b00001110 && (buf[1] & 0b11110000) == 0b10010000) {
		int n = buf[1] >> 0 & 0b1111, w = buf[0] >> 4 & 0b111;
    sprintf(output, "SRA     R%d, %d", n, w);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001100 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "SRL     R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b00001100 && (buf[1] & 0b11110000) == 0b10010000) {
		int n = buf[1] >> 0 & 0b1111, w = buf[0] >> 4 & 0b111;
    sprintf(output, "SRL     R%d, %d", n, w);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001101 && (buf[1] & 0b11110000) == 0b10000000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "SRLC    R%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b00001101 && (buf[1] & 0b11110000) == 0b10010000) {
		int n = buf[1] >> 0 & 0b1111, w = buf[0] >> 4 & 0b111;
    sprintf(output, "SRLC    R%d, %d", n, w);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00110010 && (buf[1] & 0b11110001) == 0b10010000) {
		int n = buf[1] >> 1 & 0b111;
    sprintf(output, "L       ER%d, %s[EA]", n * 2, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01010010 && (buf[1] & 0b11110001) == 0b10010000) {
		int n = buf[1] >> 1 & 0b111;
    sprintf(output, "L       ER%d, %s[EA+]", n * 2, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00000010 && (buf[1] & 0b11110001) == 0b10010000) {
		int m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "L       ER%d, %s[ER%d]", n * 2, dsr_prefix, m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11000000) == 0b00000000 && (buf[1] & 0b11110001) == 0b10110000) {
		int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "L       ER%d, %sER12[%s]", n * 2, dsr_prefix, signedtohex(D, 6));
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11000000) == 0b01000000 && (buf[1] & 0b11110001) == 0b10110000) {
		int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "L       ER%d, %sER14[%s]", n * 2, dsr_prefix, signedtohex(D, 6));
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00110000 && (buf[1] & 0b11110000) == 0b10010000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "L       R%d, %s[EA]", n, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01010000 && (buf[1] & 0b11110000) == 0b10010000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "L       R%d, %s[EA+]", n, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00000000 && (buf[1] & 0b11110000) == 0b10010000) {
		int m = buf[0] >> 5 & 0b111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "L       R%d, %s[ER%d]", n, dsr_prefix, m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b11010000) {
		int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "L       R%d, %sER12[%s]", n, dsr_prefix, signedtohex(D, 6));
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11000000) == 0b01000000 && (buf[1] & 0b11110000) == 0b11010000) {
		int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "L       R%d, %sER14[%s]", n, dsr_prefix, signedtohex(D, 6));
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00110100 && (buf[1] & 0b11110011) == 0b10010000) {
		int n = buf[1] >> 2 & 0b11;
    sprintf(output, "L       XR%d, %s[EA]", n * 4, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01010100 && (buf[1] & 0b11110011) == 0b10010000) {
		int n = buf[1] >> 2 & 0b11;
    sprintf(output, "L       XR%d, %s[EA+]", n * 4, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00110110 && (buf[1] & 0b11110111) == 0b10010000) {
		int n = buf[1] >> 3 & 0b1;
    sprintf(output, "L       QR%d, %s[EA]", n * 8, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01010110 && (buf[1] & 0b11110111) == 0b10010000) {
		int n = buf[1] >> 3 & 0b1;
    sprintf(output, "L       QR%d, %s[EA+]", n * 8, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00110011 && (buf[1] & 0b11110001) == 0b10010000) {
		int n = buf[1] >> 1 & 0b111;
    sprintf(output, "ST      ER%d, %s[EA]", n * 2, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01010011 && (buf[1] & 0b11110001) == 0b10010000) {
		int n = buf[1] >> 1 & 0b111;
    sprintf(output, "ST      ER%d, %s[EA+]", n * 2, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00000011 && (buf[1] & 0b11110001) == 0b10010000) {
		int m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "ST      ER%d, %s[ER%d]", n * 2, dsr_prefix, m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11000000) == 0b10000000 && (buf[1] & 0b11110001) == 0b10110000) {
		int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "ST      ER%d, %sER12[%s]", n * 2, dsr_prefix, signedtohex(D, 6));
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11000000) == 0b11000000 && (buf[1] & 0b11110001) == 0b10110000) {
		int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "ST      ER%d, %sER14[%s]", n * 2, dsr_prefix, signedtohex(D, 6));
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00110001 && (buf[1] & 0b11110000) == 0b10010000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "ST      R%d, %s[EA]", n, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01010001 && (buf[1] & 0b11110000) == 0b10010000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "ST      R%d, %s[EA+]", n, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00000001 && (buf[1] & 0b11110000) == 0b10010000) {
		int m = buf[0] >> 5 & 0b111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "ST      R%d, %s[ER%d]", n, dsr_prefix, m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11000000) == 0b10000000 && (buf[1] & 0b11110000) == 0b11010000) {
		int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "ST      R%d, %sER12[%s]", n, dsr_prefix, signedtohex(D, 6));
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11000000) == 0b11000000 && (buf[1] & 0b11110000) == 0b11010000) {
		int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "ST      R%d, %sER14[%s]", n, dsr_prefix, signedtohex(D, 6));
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00110101 && (buf[1] & 0b11110011) == 0b10010000) {
		int n = buf[1] >> 2 & 0b11;
    sprintf(output, "ST      XR%d, %s[EA]", n * 4, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01010101 && (buf[1] & 0b11110011) == 0b10010000) {
		int n = buf[1] >> 2 & 0b11;
    sprintf(output, "ST      XR%d, %s[EA+]", n * 4, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00110111 && (buf[1] & 0b11110111) == 0b10010000) {
		int n = buf[1] >> 3 & 0b1;
    sprintf(output, "ST      QR%d, %s[EA]", n * 8, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01010111 && (buf[1] & 0b11110111) == 0b10010000) {
		int n = buf[1] >> 3 & 0b1;
    sprintf(output, "ST      QR%d, %s[EA+]", n * 8, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11111111) == 0b11100001) {
		int i = buf[0] >> 0 & 0b11111111;
    sprintf(output, "ADD     SP, %d", (i<<24)>>24);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001111 && (buf[1] & 0b11111111) == 0b10100000) {
		int m = buf[0] >> 4 & 0b1111;
    sprintf(output, "MOV     ECSR, R%d", m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00001101 && (buf[1] & 0b11110001) == 0b10100000) {
		int m = buf[1] >> 1 & 0b111;
    sprintf(output, "MOV     ELR, ER%d", m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001100 && (buf[1] & 0b11111111) == 0b10100000) {
		int m = buf[0] >> 4 & 0b1111;
    sprintf(output, "MOV     EPSW, R%d", m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00000101 && (buf[1] & 0b11110001) == 0b10100000) {
		int n = buf[1] >> 1 & 0b111;
    sprintf(output, "MOV     ER%d, ELR", n * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00011010 && (buf[1] & 0b11110001) == 0b10100000) {
		int n = buf[1] >> 1 & 0b111;
    sprintf(output, "MOV     ER%d, SP", n * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001011 && (buf[1] & 0b11111111) == 0b10100000) {
		int m = buf[0] >> 4 & 0b1111;
    sprintf(output, "MOV     PSW, R%d", m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11111111) == 0b11101001) {
		int i = buf[0] >> 0 & 0b11111111;
    sprintf(output, "MOV     PSW, %d", i);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00000111 && (buf[1] & 0b11110000) == 0b10100000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     R%d, ECSR", n);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00000100 && (buf[1] & 0b11110000) == 0b10100000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     R%d, EPSW", n);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00000011 && (buf[1] & 0b11110000) == 0b10100000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     R%d, PSW", n);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00001010 && (buf[1] & 0b11111111) == 0b10100001) {
		int m = buf[0] >> 5 & 0b111;
    sprintf(output, "MOV     SP, ER%d", m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01011110 && (buf[1] & 0b11110001) == 0b11110000) {
		int n = buf[1] >> 1 & 0b111;
    sprintf(output, "PUSH    ER%d", n * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01111110 && (buf[1] & 0b11110111) == 0b11110000) {
		int n = buf[1] >> 3 & 0b1;
    sprintf(output, "PUSH    QR%d", n * 8);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01001110 && (buf[1] & 0b11110000) == 0b11110000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "PUSH    R%d", n);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01101110 && (buf[1] & 0b11110011) == 0b11110000) {
		int n = buf[1] >> 2 & 0b11;
    sprintf(output, "PUSH    XR%d", n * 4);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b11001110 && (buf[1] & 0b11110000) == 0b11110000) {
		int l = buf[1] >> 3 & 0b1, e = buf[1] >> 2 & 0b1, p = buf[1] >> 1 & 0b1, a = buf[1] & 0b1;
		sprintf(output, "PUSH    %s%s%s%s",
			 (p ? "ELR " : ""),
			 (e ? "EPSW " : ""),
			 (l ? "LR " : ""),
			 (a ? "EA " : ""));
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00011110 && (buf[1] & 0b11110001) == 0b11110000) {
		int n = buf[1] >> 1 & 0b111;
    sprintf(output, "POP     ER%d", n * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00111110 && (buf[1] & 0b11110111) == 0b11110000) {
		int n = buf[1] >> 3 & 0b1;
    sprintf(output, "POP     QR%d", n * 8);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00001110 && (buf[1] & 0b11110000) == 0b11110000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "POP     R%d", n);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00101110 && (buf[1] & 0b11110011) == 0b11110000) {
		int n = buf[1] >> 2 & 0b11;
    sprintf(output, "POP     XR%d", n * 4);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b10001110 && (buf[1] & 0b11110000) == 0b11110000) {
		int l = buf[1] >> 3 & 0b1, e = buf[1] >> 2 & 0b1, p = buf[1] >> 1 & 0b1, a = buf[1] & 0b1;
		sprintf(output, "POP    %s%s%s%s",
			 (p ? "ELR " : ""),
			 (e ? "EPSW " : ""),
			 (l ? "LR " : ""),
			 (a ? "EA " : ""));
		if (p) {
			// in this case, we dont hope the controlflow gets extended, lets just define a funciton at next instruction
			//LABEL_FUNCTION(pc + 2);
		}
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001110 && (buf[1] & 0b11110000) == 0b10100000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     CR%d, R%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00101101 && (buf[1] & 0b11110001) == 0b11110000) {
		int n = buf[1] >> 1 & 0b111;
    sprintf(output, "MOV     CER%d, %s[EA]", n * 2, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00111101 && (buf[1] & 0b11110001) == 0b11110000) {
		int n = buf[1] >> 1 & 0b111;
    sprintf(output, "MOV     CER%d, %s[EA+]", n * 2, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00001101 && (buf[1] & 0b11110000) == 0b11110000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     CR%d, %s[EA]", n, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00011101 && (buf[1] & 0b11110000) == 0b11110000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     CR%d, %s[EA+]", n, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01001101 && (buf[1] & 0b11110011) == 0b11110000) {
		int n = buf[1] >> 2 & 0b11;
    sprintf(output, "MOV     CXR%d, %s[EA]", n * 4, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01011101 && (buf[1] & 0b11110011) == 0b11110000) {
		int n = buf[1] >> 2 & 0b11;
    sprintf(output, "MOV     CXR%d, %s[EA+]", n * 4, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01101101 && (buf[1] & 0b11110111) == 0b11110000) {
		int n = buf[1] >> 3 & 0b1;
    sprintf(output, "MOV     CQR%d, %s[EA]", n * 8, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01111101 && (buf[1] & 0b11110111) == 0b11110000) {
		int n = buf[1] >> 3 & 0b1;
    sprintf(output, "MOV     CQR%d, %s[EA+]", n * 8, dsr_prefix);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00000110 && (buf[1] & 0b11110000) == 0b10100000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     R%d, CR%d", n, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b10101101 && (buf[1] & 0b11110001) == 0b11110000) {
		int m = buf[1] >> 1 & 0b111;
    sprintf(output, "MOV     %s[EA], CER%d", dsr_prefix, m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b10111101 && (buf[1] & 0b11110001) == 0b11110000) {
		int m = buf[1] >> 1 & 0b111;
    sprintf(output, "MOV     %s[EA+], CER%d", dsr_prefix, m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b10001101 && (buf[1] & 0b11110000) == 0b11110000) {
		int m = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     %s[EA], CR%d", dsr_prefix, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b10011101 && (buf[1] & 0b11110000) == 0b11110000) {
		int m = buf[1] >> 0 & 0b1111;
    sprintf(output, "MOV     %s[EA+], CR%d", dsr_prefix, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b11001101 && (buf[1] & 0b11110011) == 0b11110000) {
		int m = buf[1] >> 2 & 0b11;
    sprintf(output, "MOV     %s[EA], CXR%d", dsr_prefix, m * 4);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b11011101 && (buf[1] & 0b11110011) == 0b11110000) {
		int m = buf[1] >> 2 & 0b11;
    sprintf(output, "MOV     %s[EA+], CXR%d", dsr_prefix, m * 4);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b11101101 && (buf[1] & 0b11110111) == 0b11110000) {
		int m = buf[1] >> 3 & 0b1;
    sprintf(output, "MOV     %s[EA], CQR%d", dsr_prefix, m * 8);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b11111101 && (buf[1] & 0b11110111) == 0b11110000) {
		int m = buf[1] >> 3 & 0b1;
    sprintf(output, "MOV     %s[EA+], CQR%d", dsr_prefix, m * 8);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00001010 && (buf[1] & 0b11111111) == 0b11110000) {
		int m = buf[0] >> 5 & 0b111;
    sprintf(output, "LEA     [%sER%d]", dsr_prefix, m * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00011111 && (buf[1] & 0b11110000) == 0b10000000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "DAA     R%d", n);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00111111 && (buf[1] & 0b11110000) == 0b10000000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "DAS     R%d", n);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01011111 && (buf[1] & 0b11110000) == 0b10000000) {
		int n = buf[1] >> 0 & 0b1111;
    sprintf(output, "NEG     R%d", n);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b00000000 && (buf[1] & 0b11110000) == 0b10100000) {
		int b = buf[0] >> 4 & 0b111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "SB      R%d.%d", n, b);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b00000010 && (buf[1] & 0b11110000) == 0b10100000) {
		int b = buf[0] >> 4 & 0b111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "RB      R%d.%d", n, b);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b00000001 && (buf[1] & 0b11110000) == 0b10100000) {
		int b = buf[0] >> 4 & 0b111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "TB      R%d.%d", n, b);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00001000 && (buf[1] & 0b11111111) == 0b11101101) {
    sprintf(output, "EI");
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b11110111 && (buf[1] & 0b11111111) == 0b11101011) {
    sprintf(output, "DI");
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b10000000 && (buf[1] & 0b11111111) == 0b11101101) {
    sprintf(output, "SC");
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b01111111 && (buf[1] & 0b11111111) == 0b11101011) {
    sprintf(output, "RC");
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b11001111 && (buf[1] & 0b11111111) == 0b11111110) {
    sprintf(output, "CPLC");
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b11000000) {
		int c = buf[1] >> 0 & 0b1111, r = buf[0] >> 0 & 0b11111111;
		unsigned int addr = (pc & 0x0f0000) | (uint16_t)(pc + 2 + ((int)(signed char)r << 1));
		if (c == 15) {
    sprintf(output, "NOP");
		}
		else {
			//LABEL_LABEL(addr);
			if (c == 14) { // BAL
				// in this case, we dont hope the controlflow gets extended, lets just define a funciton at next instruction
				//LABEL_FUNCTION(pc + 2);
			}
			baddr=addr;
    sprintf(output, "B%s    ", cond[c]);
    buf+=6;
		}
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00001111 && (buf[1] & 0b11110001) == 0b10000001) {
		int m = buf[1] >> 1 & 0b111, n = buf[0] >> 5 & 0b111;
    sprintf(output, "EXTBW   ER%d", n +m);
		//  << " ; ERn" << (n * 2) << " = R" << (m * 2)
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11000000) == 0b00000000 && (buf[1] & 0b11111111) == 0b11100101) {
		int i = buf[0] >> 0 & 0b111111;
    sprintf(output, "SWI      %d", i);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b11111111 && (buf[1] & 0b11111111) == 0b11111111) {
    sprintf(output, "BRK");
		//LABEL_FUNCTION(pc + 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00000010 && (buf[1] & 0b11111111) == 0b11110000) {
		int n = buf[0] >> 5 & 0b111;
    sprintf(output, "B       ER%d", n * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00000011 && (buf[1] & 0b11111111) == 0b11110000) {
		int n = buf[0] >> 5 & 0b111;
    sprintf(output, "BL      ER%d", n * 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00000100 && (buf[1] & 0b11110001) == 0b11110000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "MUL     ER%d, R%d", n * 2, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00001111) == 0b00001001 && (buf[1] & 0b11110001) == 0b11110000) {
		int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "DIV     ER%d, R%d", n * 2, m);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00101111 && (buf[1] & 0b11111111) == 0b11111110) {
    sprintf(output, "INC     [EA]");
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00111111 && (buf[1] & 0b11111111) == 0b11111110) {
    sprintf(output, "DEC     [EA]");
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00011111 && (buf[1] & 0b11111111) == 0b11111110) {
    sprintf(output, "RT");
		//LABEL_FUNCTION(pc + 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00001111 && (buf[1] & 0b11111111) == 0b11111110) {
    sprintf(output, "RTI");
		//LABEL_FUNCTION(pc + 2);
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b10001111 && (buf[1] & 0b11111111) == 0b11111110) {
    sprintf(output, "NOP");
		buf += 2;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00001000 && (buf[1] & 0b11110001) == 0b10100000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "L       ER%d, %sER%d[%s]", n * 2, dsr_prefix, m * 2, signedtohex(E * 256 + D, 16));
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00010010 && (buf[1] & 0b11110001) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "L       ER%d, %s%04X", n * 2, dsr_prefix, E * 256 + D);
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00001000 && (buf[1] & 0b11110000) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, m = buf[0] >> 5 & 0b111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "L       R%d, %sER%d[%s]", n, dsr_prefix, m * 2, signedtohex(E * 256 + D, 16));
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00010000 && (buf[1] & 0b11110000) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "L       R%d, %s%04X", n, dsr_prefix, E * 256 + D);
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00001001 && (buf[1] & 0b11110001) == 0b10100000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "ST      ER%d, %sER%d[%s]", n * 2, dsr_prefix, m * 2, signedtohex(E * 256 + D, 16));
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00010011 && (buf[1] & 0b11110001) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, n = buf[1] >> 1 & 0b111;
    sprintf(output, "ST      ER%d, %s%04X", n * 2, dsr_prefix, E * 256 + D);
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00001001 && (buf[1] & 0b11110000) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, m = buf[0] >> 5 & 0b111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "ST      R%d, %sER%d[%s]", n, dsr_prefix, m * 2, signedtohex(E * 256 + D, 16));
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00010001 && (buf[1] & 0b11110000) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
    sprintf(output, "ST      R%d, %s%04X", n, dsr_prefix, E * 256 + D);
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b00011111) == 0b00001011 && (buf[1] & 0b11111111) == 0b11110000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, m = buf[0] >> 5 & 0b111;
    sprintf(output, "LEA     ER%d[%s]", m * 2, signedtohex(E * 256 + D, 16));
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00001100 && (buf[1] & 0b11111111) == 0b11110000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111;
    sprintf(output, "LEA     %04X", E * 256 + D);
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b10000000 && (buf[1] & 0b11111111) == 0b10100000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, b = buf[0] >> 4 & 0b111;
    sprintf(output, "SB      %04X.%d", E * 256 + D, b);
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b10000010 && (buf[1] & 0b11111111) == 0b10100000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, b = buf[0] >> 4 & 0b111;
    sprintf(output, "RB      %04X.%d", E * 256 + D, b);
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b10001111) == 0b10000001 && (buf[1] & 0b11111111) == 0b10100000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, b = buf[0] >> 4 & 0b111;
    sprintf(output, "TB      %04X.%d", E * 256 + D, b);
		buf += 4;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00000000 && (buf[1] & 0b11110000) == 0b11110000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int C = buf[2] >> 0 & 0b11111111, D = buf[3] >> 0 & 0b11111111, g = buf[1] >> 0 & 0b1111;
		unsigned int  addr = (g << 16) | (D << 8) | (C);
		//LABEL_FUNCTION(addr);
		baddr=addr;
    sprintf(output, "B       ");
		// in this case, we dont hope the controlflow gets extended, lets just define a funciton at next instruction
		//LABEL_FUNCTION(pc + 4);
		buf += 10;
		return buf;
	}
	if ((buf[0] & 0b11111111) == 0b00000001 && (buf[1] & 0b11110000) == 0b11110000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
		int C = buf[2] >> 0 & 0b11111111, D = buf[3] >> 0 & 0b11111111, g = buf[1] >> 0 & 0b1111;
		unsigned int addr = (g << 16) | (D << 8) | (C);
		//LABEL_FUNCTION(addr);
		baddr=addr;
    sprintf(output, "BL      ");
		buf += 10;
		return buf;
	}
    sprintf(output, "dw      %04X\n", buf[0] | (buf[1] << 8));
	buf += 2;
	return buf;
}



int
print_insn_nxu16(bfd_vma memaddr, struct disassemble_info *info)
{
  bfd_byte buffer[2];
  int status;
char ptrstr[100];
uint8_t * str;
uint16_t iword;
uint16_t insn[5]={0};
fpr = info->fprintf_func;
stream = info->stream;
  info->bytes_per_chunk = 2;
  info->bytes_per_line = 2;
  info->display_endian = BFD_ENDIAN_LITTLE;
  info->insn_info_valid = 1;
  info->target = memaddr;

  if ((status = info->read_memory_func (memaddr, buffer,
					2, info)) != 0)
    {
      info->memory_error_func (status, memaddr, info);
      return -1; /* loongarch_insn_length (0); */
    }

iword = bfd_getl16 (buffer);
insn[0]=iword;
str=decode(ptrstr,(uint8_t*)insn,(uint32_t)memaddr);
if(str==(uint8_t*)insn+2){fpr (stream, "%s",ptrstr);return 2;}
  if ((status = info->read_memory_func (memaddr, buffer,
					2, info)) != 0)
    {
      info->memory_error_func (status, memaddr, info);
      return -1; /* loongarch_insn_length (0); */
    }

iword = bfd_getl16 (buffer);
insn[1]=iword;
str=decode(ptrstr,(uint8_t*)insn,(uint32_t)memaddr);
if(str==(uint8_t*)insn+4){fpr (stream, "%s",ptrstr);return 4;}
  if ((status = info->read_memory_func (memaddr, buffer,
					2, info)) != 0)
    {
      info->memory_error_func (status, memaddr, info);
      return -1; /* loongarch_insn_length (0); */
    }

iword = bfd_getl16 (buffer);
insn[2]=iword;
str=decode(ptrstr,(uint8_t*)insn,(uint32_t)memaddr);
if(str==(uint8_t*)insn+6){fpr (stream, "%s",ptrstr);return 6;}
if(str==(uint8_t*)insn+8){fpr (stream, "%s",ptrstr);info->print_address_func ((bfd_vma) baddr, info);return 2;}
if(str==(uint8_t*)insn+10){fpr (stream, "%s",ptrstr);info->print_address_func ((bfd_vma) baddr, info);return 4;}
  return -1;
}
