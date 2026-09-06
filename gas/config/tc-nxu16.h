/* tc-nxu16.h -- Header file for tc-nxu16.c.

   Copyright (C) 2009-2021 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with GAS; see the file COPYING.  If not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#define TC_NXU16 1
#define TARGET_BYTES_BIG_ENDIAN 0
#define WORKING_DOT_WORD

/* This macro is the BFD architecture to pass to `bfd_set_arch_mach'.  */
#define TARGET_FORMAT "elf32-nxu16"

#define TARGET_ARCH bfd_arch_nxu16

#define md_undefined_symbol(NAME)           0
extern const relax_typeS md_relax_table[];
#define TC_GENERIC_RELAX_TABLE md_relax_table
#define MD_APPLY_SYM_VALUE(FIX) 0

typedef struct
{
  const char *name;
  int nbytes;
  bfd_reloc_code_real_type reloc;
} nxu16_exp_mod_data_t;

#define TC_PARSE_CONS_RETURN_TYPE const nxu16_exp_mod_data_t *
#define TC_PARSE_CONS_RETURN_NONE nxu16_exp_mod_data
#define TC_PARSE_CONS_EXPRESSION(EXP, N) \
  nxu16_parse_cons_expression ((EXP), (N))
#define TC_CONS_FIX_NEW nxu16_cons_fix_new

extern const nxu16_exp_mod_data_t *
  nxu16_parse_cons_expression (expressionS *, int);
extern const nxu16_exp_mod_data_t nxu16_exp_mod_data[];
extern void nxu16_cons_fix_new (fragS *, int, int, expressionS *,
			       const nxu16_exp_mod_data_t *);

#define md_section_align(SEGMENT, SIZE)     (SIZE)
