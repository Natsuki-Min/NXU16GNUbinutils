/* tc-nxu16.c -- Assemble code for the nX-U16 architecture.
   Copyright (C) 2009-2025 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   GAS is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.  */

#include "as.h"
#include "opcode/nxu16.h"
#include "safe-ctype.h"

const char comment_chars[] = "//";
const char line_separator_chars[] = ";";
const char line_comment_chars[] = "//";
const char FLT_CHARS[] = "rRsSfFdDxXpP";
const char EXP_CHARS[] = "eE";

static htab_t opcode_hash_control;

enum nxu16_match_kind
{
  NXU16_NO_MATCH,
  NXU16_CONSTANT,
  NXU16_RELOCATABLE
};

struct nxu16_operand
{
  enum nxu16_match_kind kind;
  uint32_t value;
  expressionS exp;
  bfd_reloc_code_real_type modifier_reloc;
};

/* GAS relaxation states.  A conditional long branch is an inverse
   condition over a four-byte B Cadr.  BAL (and the `b' alias) only needs
   the B Cadr.  */
enum
{
  RELAX_COND_SHORT,
  RELAX_COND_LONG,
  RELAX_BAL_SHORT,
  RELAX_BAL_LONG
};

const relax_typeS md_relax_table[] =
{
  /* GAS measures a backward target from the start of the frag while the
     encoded displacement is relative to the following instruction.  Keep
     two bytes of margin at the negative boundary so -129 words expands.  */
  { 254, -254, 2, RELAX_COND_LONG },
  { 0, 0, 6, 0 },
  { 254, -254, 2, RELAX_BAL_LONG },
  { 0, 0, 4, 0 }
};

static char *
nxu16_skip_space (char *p)
{
  while (ISSPACE ((unsigned char) *p))
    ++p;
  return p;
}

static char *
nxu16_trim (char *p)
{
  char *end;

  p = nxu16_skip_space (p);
  end = p + strlen (p);
  while (end > p && ISSPACE ((unsigned char) end[-1]))
    --end;
  *end = '\0';
  return p;
}

static void
nxu16_compact_lower (const char *input, char *output, size_t size)
{
  size_t used = 0;

  while (*input != '\0' && used + 1 < size)
    {
      if (!ISSPACE ((unsigned char) *input))
	output[used++] = TOLOWER ((unsigned char) *input);
      ++input;
    }
  output[used] = '\0';
}

static int
nxu16_register_number (const char *text, const char *prefix, int alignment)
{
  size_t len = strlen (prefix);
  char *end;
  long number;

  if (strncasecmp (text, prefix, len) != 0 || !ISDIGIT (text[len]))
    return -1;
  number = strtol (text + len, &end, 10);
  if (*end != '\0' || number < 0 || number > 15
      || number % alignment != 0)
    return -1;
  return number / alignment;
}

static bfd_reloc_code_real_type
nxu16_modifier_reloc (const char *name, size_t length)
{
  if (length == 3 && strncasecmp (name, "lo8", 3) == 0)
    return BFD_RELOC_NXU16_LO8;
  if (length == 3 && strncasecmp (name, "hi8", 3) == 0)
    return BFD_RELOC_NXU16_HI8;
  if ((length == 4 && strncasecmp (name, "hlo8", 4) == 0)
      || (length == 3 && strncasecmp (name, "hh8", 3) == 0))
    return BFD_RELOC_NXU16_HLO8;
  if (length == 4 && strncasecmp (name, "hhi8", 4) == 0)
    return BFD_RELOC_NXU16_HHI8;
  return BFD_RELOC_NONE;
}

static unsigned int
nxu16_modifier_shift (bfd_reloc_code_real_type reloc)
{
  switch (reloc)
    {
    case BFD_RELOC_NXU16_HI8:
      return 8;
    case BFD_RELOC_NXU16_HLO8:
      return 16;
    case BFD_RELOC_NXU16_HHI8:
      return 24;
    default:
      return 0;
    }
}

/* Parse an entire operand through GAS's expression parser.  The only
   target syntax handled here is the AVR-compatible byte selector wrapped
   around an otherwise ordinary GAS expression.  */
static bool
nxu16_parse_expression (char *text, expressionS *exp,
			bfd_reloc_code_real_type *modifier)
{
  char *save = input_line_pointer;
  char *start = nxu16_skip_space (text);
  char *name_end = start;
  char *after;

  *modifier = BFD_RELOC_NONE;
  if (*start == '#')
    start = nxu16_skip_space (start + 1);

  while (ISALNUM ((unsigned char) *name_end) || *name_end == '_')
    ++name_end;
  after = nxu16_skip_space (name_end);
  if (*after == '(')
    {
      bfd_reloc_code_real_type reloc
	= nxu16_modifier_reloc (start, name_end - start);
      if (reloc != BFD_RELOC_NONE)
	{
	  input_line_pointer = nxu16_skip_space (after + 1);
	  expression (exp);
	  input_line_pointer = nxu16_skip_space (input_line_pointer);
	  if (*input_line_pointer != ')')
	    {
	      input_line_pointer = save;
	      return false;
	    }
	  input_line_pointer = nxu16_skip_space (input_line_pointer + 1);
	  if (*input_line_pointer != '\0')
	    {
	      input_line_pointer = save;
	      return false;
	    }
	  *modifier = reloc;
	  if (exp->X_op == O_constant)
	    {
	      exp->X_add_number
		= ((valueT) exp->X_add_number >> nxu16_modifier_shift (reloc))
		  & 0xff;
	      *modifier = BFD_RELOC_NONE;
	    }
	  input_line_pointer = save;
	  return exp->X_op != O_absent && exp->X_op != O_illegal;
	}
    }

  input_line_pointer = start;
  expression (exp);
  input_line_pointer = nxu16_skip_space (input_line_pointer);
  if (*input_line_pointer != '\0')
    {
      input_line_pointer = save;
      return false;
    }
  input_line_pointer = save;
  return exp->X_op != O_absent && exp->X_op != O_illegal;
}

static struct nxu16_operand
nxu16_no_match (void)
{
  struct nxu16_operand result;
  memset (&result, 0, sizeof (result));
  result.kind = NXU16_NO_MATCH;
  return result;
}

static struct nxu16_operand
nxu16_constant (uint32_t value)
{
  struct nxu16_operand result = nxu16_no_match ();
  result.kind = NXU16_CONSTANT;
  result.value = value;
  return result;
}

static struct nxu16_operand
nxu16_expression_operand (char *text, offsetT minimum, offsetT maximum,
			  bool allow_symbol, bool allow_modifier)
{
  struct nxu16_operand result = nxu16_no_match ();
  bfd_reloc_code_real_type modifier;

  if (!nxu16_parse_expression (text, &result.exp, &modifier))
    return result;
  if (modifier != BFD_RELOC_NONE && !allow_modifier)
    return result;
  if (result.exp.X_op == O_constant)
    {
      if (result.exp.X_add_number < minimum
	  || result.exp.X_add_number > maximum)
	return result;
      result.kind = NXU16_CONSTANT;
      result.value = result.exp.X_add_number;
      return result;
    }
  if (!allow_symbol)
    return result;
  result.kind = NXU16_RELOCATABLE;
  result.modifier_reloc = modifier;
  return result;
}

static int
nxu16_memory_register (const char *text)
{
  char compact[64];
  size_t len;

  nxu16_compact_lower (text, compact, sizeof (compact));
  len = strlen (compact);
  if (len < 3 || compact[0] != '[' || compact[len - 1] != ']')
    return -1;
  compact[len - 1] = '\0';
  if (strcmp (compact + 1, "fp") == 0)
    return 7;
  if (strcmp (compact + 1, "bp") == 0)
    return 6;
  return nxu16_register_number (compact + 1, "er", 2);
}

static struct nxu16_operand
nxu16_far_memory_register (const char *text)
{
  char compact[64];
  char *colon;
  int segment;
  int base;

  nxu16_compact_lower (text, compact, sizeof (compact));
  colon = strchr (compact, ':');
  if (colon == NULL || colon == compact)
    return nxu16_no_match ();
  *colon = '\0';
  segment = nxu16_register_number (compact, "r", 1);
  base = nxu16_memory_register (colon + 1);
  if (segment < 0 || base < 0)
    return nxu16_no_match ();
  return nxu16_constant (base | (segment << 3));
}

static struct nxu16_operand
nxu16_displacement_operand (const char *text, nxu16_ArgType type)
{
  struct nxu16_operand result = nxu16_no_match ();
  char buffer[256];
  char *bracket;
  int base;

  nxu16_compact_lower (text, buffer, sizeof (buffer));
  bracket = strchr (buffer, '[');
  if (bracket == NULL || bracket == buffer)
    return result;
  base = nxu16_memory_register (bracket);
  if (base < 0)
    return result;
  *bracket = '\0';

  if (type == ARG_MEM_FPDisp6 || type == ARG_MEM_BPDisp6)
    {
      if ((type == ARG_MEM_FPDisp6 && base != 7)
	  || (type == ARG_MEM_BPDisp6 && base != 6))
	return result;
      result = nxu16_expression_operand (buffer, -32, 31, false, false);
      if (result.kind == NXU16_CONSTANT)
	result.value = (base << 6) | (result.value & 0x3f);
      return result;
    }

  result = nxu16_expression_operand (buffer, -32768, 65535, true, false);
  if (result.kind != NXU16_NO_MATCH)
    result.value = (base << 16) | (result.value & 0xffff);
  return result;
}

static struct nxu16_operand
nxu16_register_list (const char *text, const char *mnemonic)
{
  char buffer[128];
  char *saveptr = NULL;
  char *item;
  unsigned int mask = 0;

  nxu16_compact_lower (text, buffer, sizeof (buffer));
  for (item = strtok_r (buffer, ",", &saveptr); item != NULL;
       item = strtok_r (NULL, ",", &saveptr))
    {
      unsigned int bit;
      if (strcmp (item, "ea") == 0)
	bit = 1;
      else if (strcmp (mnemonic, "push") == 0 && strcmp (item, "elr") == 0)
	bit = 2;
      else if (strcmp (mnemonic, "pop") == 0 && strcmp (item, "pc") == 0)
	bit = 2;
      else if (strcmp (mnemonic, "push") == 0 && strcmp (item, "epsw") == 0)
	bit = 4;
      else if (strcmp (mnemonic, "pop") == 0 && strcmp (item, "psw") == 0)
	bit = 4;
      else if (strcmp (item, "lr") == 0)
	bit = 8;
      else
	return nxu16_no_match ();
      if ((mask & bit) != 0)
	return nxu16_no_match ();
      mask |= bit;
    }
  return mask == 0 ? nxu16_no_match () : nxu16_constant (mask);
}

static struct nxu16_operand
nxu16_match_operand (char *text, nxu16_ArgType type, const char *mnemonic)
{
  char compact[128];
  int reg;

  nxu16_compact_lower (text, compact, sizeof (compact));
  switch (type)
    {
    case ARG_REG_Rn:
      reg = nxu16_register_number (compact, "r", 1);
      break;
    case ARG_REG_ERn:
      if (strcmp (compact, "fp") == 0)
	reg = 7;
      else if (strcmp (compact, "bp") == 0)
	reg = 6;
      else
	reg = nxu16_register_number (compact, "er", 2);
      break;
    case ARG_REG_XRn:
      reg = nxu16_register_number (compact, "xr", 4);
      break;
    case ARG_REG_QRn:
      reg = nxu16_register_number (compact, "qr", 8);
      break;
    case ARG_REG_CRn:
      reg = nxu16_register_number (compact, "cr", 1);
      break;
    case ARG_REG_CERn:
      reg = nxu16_register_number (compact, "cer", 2);
      break;
    case ARG_REG_CXRn:
      reg = nxu16_register_number (compact, "cxr", 4);
      break;
    case ARG_REG_CQRn:
      reg = nxu16_register_number (compact, "cqr", 8);
      break;
    case ARG_REG_SP:
      reg = strcasecmp (compact, "sp") == 0 ? 0 : -1;
      break;
    case ARG_REG_PSW:
      reg = strcasecmp (compact, "psw") == 0 ? 0 : -1;
      break;
    case ARG_REG_ELR:
      reg = strcasecmp (compact, "elr") == 0 ? 0 : -1;
      break;
    case ARG_REG_ECSR:
      reg = strcasecmp (compact, "ecsr") == 0 ? 0 : -1;
      break;
    case ARG_REG_EPSW:
      reg = strcasecmp (compact, "epsw") == 0 ? 0 : -1;
      break;
    case ARG_REG_LIST:
      return nxu16_register_list (text, mnemonic);

    case ARG_MEM_EA:
      return strcmp (compact, "[ea]") == 0
	? nxu16_constant (0) : nxu16_no_match ();
    case ARG_MEM_EAP:
      return strcmp (compact, "[ea+]") == 0
	? nxu16_constant (0) : nxu16_no_match ();
    case ARG_MEM_ERn:
      reg = nxu16_memory_register (compact);
      break;
    case ARG_MEM_RnERn:
      return nxu16_far_memory_register (text);

    case ARG_IMM3:
      return nxu16_expression_operand (text, 0, 7, true, false);
    case ARG_IMM6:
      return nxu16_expression_operand (text, 0, 63, true, false);
    case ARG_IMM7_SIGNED:
      return nxu16_expression_operand (text, -64, 63, false, false);
    case ARG_IMM8:
      return nxu16_expression_operand (text, -128, 255, true, true);
    case ARG_MEM_Dadr:
      return nxu16_expression_operand (text, 0, 0xffff, true, false);
    case ARG_FLASH_Cadr:
      return nxu16_expression_operand (text, 0, 0xfffff, true, false);
    case ARG_FLASH_Radr:
      return nxu16_expression_operand (text, -128, 127, true, false);
    case ARG_MEM_ERnDisp16:
    case ARG_MEM_FPDisp6:
    case ARG_MEM_BPDisp6:
      return nxu16_displacement_operand (text, type);
    case ARG_SYMBOL:
      return nxu16_expression_operand (text, 0, 0, true, false);
    default:
      return nxu16_no_match ();
    }
  return reg < 0 ? nxu16_no_match () : nxu16_constant (reg);
}

static unsigned int
nxu16_scatter (uint32_t value, uint32_t mask)
{
  unsigned int result = 0;
  unsigned int source_bit = 0;
  unsigned int bit;

  for (bit = 0; bit < 32; ++bit)
    if ((mask & (1u << bit)) != 0)
      {
	if ((value & (1u << source_bit)) != 0)
	  result |= 1u << bit;
	++source_bit;
      }
  return result;
}

static int
nxu16_branch_condition (uint32_t opcode)
{
  return (opcode >> 8) & 0x0f;
}

static void
nxu16_emit_relax_branch (const struct nxu16_operand *operand, int condition)
{
  int state = condition == 14 ? RELAX_BAL_SHORT : RELAX_COND_SHORT;
  int maximum = condition == 14 ? 4 : 6;
  char *where;

  where = frag_var (rs_machine_dependent, maximum, 2, state,
		    operand->exp.X_add_symbol, operand->exp.X_add_number,
		    (char *) (uintptr_t) condition);
  bfd_putl16 (0xc000 | (condition << 8), (bfd_byte *) where);
  dwarf2_emit_insn (0);
}

static bfd_reloc_code_real_type
nxu16_operand_reloc (nxu16_ArgType type,
		     const struct nxu16_operand *operand,
		     int *offset, int *size, bool *pcrel)
{
  *offset = 0;
  *size = 1;
  *pcrel = false;
  if (operand->modifier_reloc != BFD_RELOC_NONE)
    return operand->modifier_reloc;

  switch (type)
    {
    case ARG_IMM3:
      return BFD_RELOC_NXU16_IMM3;
    case ARG_IMM6:
      return BFD_RELOC_NXU16_IMM6;
    case ARG_IMM8:
      return BFD_RELOC_NXU16_IMM8;
    case ARG_MEM_ERnDisp16:
    case ARG_MEM_Dadr:
      *offset = 2;
      *size = 2;
      return BFD_RELOC_NXU16_MEM_Dadr;
    case ARG_FLASH_Cadr:
      *size = 4;
      return BFD_RELOC_NXU16_FLASH_Cadr;
    case ARG_FLASH_Radr:
      *pcrel = true;
      return BFD_RELOC_NXU16_FLASH_Radr;
    default:
      return BFD_RELOC_NONE;
    }
}

static int
nxu16_split_operands (char *text, char **operands, const char *mnemonic)
{
  int count = 0;
  int parens = 0;
  int brackets = 0;
  char *p;

  text = nxu16_trim (text);
  if (*text == '\0')
    return 0;
  if (strcmp (mnemonic, "push") == 0 || strcmp (mnemonic, "pop") == 0)
    {
      operands[0] = text;
      return 1;
    }

  operands[count++] = text;
  for (p = text; *p != '\0'; ++p)
    {
      if (*p == '(')
	++parens;
      else if (*p == ')' && parens > 0)
	--parens;
      else if (*p == '[')
	++brackets;
      else if (*p == ']' && brackets > 0)
	--brackets;
      else if (*p == ',' && parens == 0 && brackets == 0)
	{
	  if (count == 2)
	    return -1;
	  *p = '\0';
	  operands[count++] = nxu16_trim (p + 1);
	}
    }
  for (int i = 0; i < count; ++i)
    operands[i] = nxu16_trim (operands[i]);
  return count;
}

static void
nxu16_rodata (int ignored)
{
  char *save = input_line_pointer;
  static char section[] = ".rodata\n";
  input_line_pointer = section;
  obj_elf_section (ignored);
  input_line_pointer = save;
}

const pseudo_typeS md_pseudo_table[] =
{
  { "rodata", nxu16_rodata, 0 },
  { NULL, NULL, 0 }
};

void
md_begin (void)
{
  const nxu16_opc_info_t *opcode;
  size_t count;

  opcode_hash_control = str_htab_create ();
  for (count = 0, opcode = nxu16_opc_info;
       count < ARRAY_SIZE (nxu16_opc_info); ++count, ++opcode)
    str_hash_insert (opcode_hash_control, opcode->name, opcode, 0);
  bfd_set_arch_mach (stdoutput, TARGET_ARCH, 0);
}

void
md_operand (expressionS *operand ATTRIBUTE_UNUSED)
{
}

void
md_assemble (char *line)
{
  char *mnemonic;
  char *tail;
  char *operands[2] = { NULL, NULL };
  struct nxu16_operand matches[2];
  const nxu16_opc_info_t *opcode;
  const nxu16_OpcodeCombination *combination = NULL;
  int operand_count;
  size_t i;

  line = nxu16_skip_space (line);
  mnemonic = line;
  while (*line != '\0' && !ISSPACE ((unsigned char) *line))
    {
      *line = TOLOWER ((unsigned char) *line);
      ++line;
    }
  if (*line != '\0')
    *line++ = '\0';
  tail = line;

  opcode = str_hash_find (opcode_hash_control, mnemonic);
  if (opcode == NULL)
    {
      as_bad (_("unknown opcode `%s'"), mnemonic);
      return;
    }
  operand_count = nxu16_split_operands (tail, operands, mnemonic);
  if (operand_count < 0)
    {
      as_bad (_("too many operands"));
      return;
    }

  /* The architectural spelling is Rn.bit or Dadr.bit.  Keep the opcode
     table's two fields but split this spelling before matching.  */
  if (operand_count == 1
      && (strcmp (mnemonic, "sb") == 0 || strcmp (mnemonic, "rb") == 0
	  || strcmp (mnemonic, "tb") == 0))
    {
      char *dot = strrchr (operands[0], '.');
      if (dot != NULL)
	{
	  *dot = '\0';
	  operands[1] = nxu16_trim (dot + 1);
	  operands[0] = nxu16_trim (operands[0]);
	  operand_count = 2;
	}
    }

  for (i = 0; i < opcode->num_combinations; ++i)
    {
      const nxu16_OpcodeCombination *candidate = &opcode->combinations[i];
      size_t j;
      if ((size_t) operand_count != candidate->num_args)
	continue;
      for (j = 0; j < candidate->num_args; ++j)
	{
	  matches[j] = nxu16_match_operand
	    (operands[j], candidate->args[j].type, mnemonic);
	  if (matches[j].kind == NXU16_NO_MATCH)
	    break;
	}
      if (j == candidate->num_args)
	{
	  combination = candidate;
	  break;
	}
    }
  if (combination == NULL)
    {
      as_bad (_("invalid operands for `%s'"), mnemonic);
      return;
    }

  if (combination->num_args == 1
      && combination->args[0].type == ARG_FLASH_Radr
      && matches[0].kind == NXU16_RELOCATABLE)
    {
      nxu16_emit_relax_branch (&matches[0],
			       nxu16_branch_condition (combination->opcode));
      return;
    }

  {
    uint32_t instruction = combination->opcode & combination->opcode_mask;
    char *where;
    size_t j;

    /* EXTBW repeats the same ER number in two fields.  */
    if (strcmp (mnemonic, "extbw") == 0 && combination->num_args == 1)
      instruction = 0x810f | (matches[0].value << 9)
		    | (matches[0].value << 5);
    else
      for (j = 0; j < combination->num_args; ++j)
	{
	  instruction &= ~combination->args[j].mask;
	  instruction |= nxu16_scatter (matches[j].value,
					  combination->args[j].mask);
	}

    where = frag_more (combination->opcode_length);
    if (combination->opcode_length == 2)
      bfd_putl16 (instruction, (bfd_byte *) where);
    else if (combination->opcode_length == 4)
      {
	bfd_putl16 (instruction >> 16, (bfd_byte *) where);
	bfd_putl16 (instruction, (bfd_byte *) where + 2);
      }
    else
      as_fatal (_("internal error: invalid nX-U16 instruction length"));

    for (j = 0; j < combination->num_args; ++j)
      if (matches[j].kind == NXU16_RELOCATABLE)
	{
	  int offset, size;
	  bool pcrel;
	  bfd_reloc_code_real_type reloc = nxu16_operand_reloc
	    (combination->args[j].type, &matches[j], &offset, &size, &pcrel);
	  if (reloc == BFD_RELOC_NONE)
	    as_bad (_("relocation is not supported for this operand"));
	  else
	    {
	      fixS *fix = fix_new_exp
		(frag_now, where - frag_now->fr_literal + offset,
		 size, &matches[j].exp, pcrel, reloc);
	      /* A local symbol may be resolved before md_apply_fix runs.  These
		 target fields perform their own range/scaling checks and must not
		 be rejected by GAS's generic byte-width check first.  */
	      if (reloc >= BFD_RELOC_NXU16_IMM3
		  && reloc <= BFD_RELOC_NXU16_HHI8)
		fix->fx_no_overflow = true;
	    }
	}
    dwarf2_emit_insn (combination->opcode_length);
  }
}

int
md_estimate_size_before_relax (fragS *frag, asection *section)
{
  if (!S_IS_DEFINED (frag->fr_symbol)
      || S_GET_SEGMENT (frag->fr_symbol) != section)
    frag->fr_subtype = (frag->fr_subtype == RELAX_BAL_SHORT
			|| frag->fr_subtype == RELAX_BAL_LONG
			? RELAX_BAL_LONG : RELAX_COND_LONG);
  return md_relax_table[frag->fr_subtype].rlx_length;
}

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED,
		 asection *section ATTRIBUTE_UNUSED, fragS *frag)
{
  bfd_byte *where = (bfd_byte *) frag->fr_literal + frag->fr_fix;
  int condition = (int) (uintptr_t) frag->fr_opcode;
  int length = md_relax_table[frag->fr_subtype].rlx_length;

  switch (frag->fr_subtype)
    {
    case RELAX_COND_SHORT:
    case RELAX_BAL_SHORT:
      bfd_putl16 (0xc000 | (condition << 8), where);
      fix_new (frag, frag->fr_fix, 1, frag->fr_symbol, frag->fr_offset,
	       true, BFD_RELOC_NXU16_FLASH_Radr);
      break;
    case RELAX_COND_LONG:
      bfd_putl16 (0xc000 | ((condition ^ 1) << 8) | 2, where);
      bfd_putl16 (0xf000, where + 2);
      bfd_putl16 (0, where + 4);
      fix_new (frag, frag->fr_fix + 2, 4, frag->fr_symbol, frag->fr_offset,
	       false, BFD_RELOC_NXU16_FLASH_Cadr);
      break;
    case RELAX_BAL_LONG:
      bfd_putl16 (0xf000, where);
      bfd_putl16 (0, where + 2);
      fix_new (frag, frag->fr_fix, 4, frag->fr_symbol, frag->fr_offset,
	       false, BFD_RELOC_NXU16_FLASH_Cadr);
      break;
    default:
      abort ();
    }
  frag->fr_var = 0;
  frag->fr_fix += length;
}

long
md_pcrel_from (fixS *fix)
{
  return fix->fx_frag->fr_address + fix->fx_where + 2;
}

void
md_apply_fix (fixS *fix, valueT *valuep, segT section)
{
  valueT value = *valuep;
  bfd_byte *where = (bfd_byte *) fix->fx_frag->fr_literal + fix->fx_where;

  if (fix->fx_addsy == NULL)
    fix->fx_done = true;
  else if (fix->fx_pcrel)
    {
      segT symbol_section = S_GET_SEGMENT (fix->fx_addsy);
      if (symbol_section == section || symbol_section == absolute_section)
	{
	  value += S_GET_VALUE (fix->fx_addsy);
	  fix->fx_done = true;
	}
    }
  if (fix->fx_subsy != NULL)
    as_bad_subtract (fix);
  if (!fix->fx_done)
    return;

  /* Target relocations below validate their logical field after any
     scaling.  GAS's byte-width check sees the unscaled value.  */
  if (fix->fx_r_type >= BFD_RELOC_NXU16_IMM3
      && fix->fx_r_type <= BFD_RELOC_NXU16_HHI8)
    fix->fx_no_overflow = true;

  switch (fix->fx_r_type)
    {
    case BFD_RELOC_8:
    case BFD_RELOC_NXU16_IMM8:
    case BFD_RELOC_NXU16_LO8:
      where[0] = value & 0xff;
      break;
    case BFD_RELOC_NXU16_HI8:
      where[0] = (value >> 8) & 0xff;
      break;
    case BFD_RELOC_NXU16_HLO8:
      where[0] = (value >> 16) & 0xff;
      break;
    case BFD_RELOC_NXU16_HHI8:
      where[0] = (value >> 24) & 0xff;
      break;
    case BFD_RELOC_NXU16_IMM3:
      if (value > 7)
	as_bad_where (fix->fx_file, fix->fx_line,
		      _("shift count out of range: %ld"), (long) value);
      where[0] = (where[0] & ~0x70) | ((value & 7) << 4);
      break;
    case BFD_RELOC_NXU16_IMM6:
      if (value > 63)
	as_bad_where (fix->fx_file, fix->fx_line,
		      _("immediate out of range: %ld"), (long) value);
      where[0] = (where[0] & ~0x3f) | (value & 0x3f);
      break;
    case BFD_RELOC_16:
    case BFD_RELOC_NXU16_IMM16:
    case BFD_RELOC_NXU16_MEM_Dadr:
      bfd_putl16 (value, where);
      break;
    case BFD_RELOC_32:
      bfd_putl32 (value, where);
      break;
    case BFD_RELOC_NXU16_FLASH_Cadr:
      if (value > 0xfffff)
	as_bad_where (fix->fx_file, fix->fx_line,
		      _("code address out of range: %#lx"), (unsigned long) value);
      bfd_putl16 ((bfd_getl16 (where) & 0xf0ff)
		  | ((value >> 8) & 0x0f00), where);
      bfd_putl16 (value & 0xffff, where + 2);
      break;
    case BFD_RELOC_NXU16_FLASH_Radr:
      if ((value & 1) != 0)
	as_bad_where (fix->fx_file, fix->fx_line,
		      _("odd address for relative branch"));
      {
	offsetT displacement = (offsetT) value / 2;
	if (displacement < -128 || displacement > 127)
	  as_bad_where (fix->fx_file, fix->fx_line,
			_("relative branch out of range: %ld"),
			(long) displacement);
	where[0] = displacement & 0xff;
      }
      break;
    default:
      as_bad_where (fix->fx_file, fix->fx_line,
		    _("unsupported relocation %s"),
		    bfd_get_reloc_code_name (fix->fx_r_type));
      break;
    }
}

void
md_number_to_chars (char *buffer, valueT value, int bytes)
{
  number_to_chars_littleendian (buffer, value, bytes);
}

arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fix)
{
  arelent *reloc;

  if (fix->fx_addsy == NULL)
    return NULL;
  if (fix->fx_subsy != NULL)
    {
      as_bad_subtract (fix);
      return NULL;
    }
  reloc = notes_alloc (sizeof (*reloc));
  reloc->sym_ptr_ptr = notes_alloc (sizeof (*reloc->sym_ptr_ptr));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fix->fx_addsy);
  reloc->address = fix->fx_frag->fr_address + fix->fx_where;
  reloc->addend = fix->fx_offset;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fix->fx_r_type);
  if (reloc->howto == NULL)
    {
      as_bad_where (fix->fx_file, fix->fx_line,
		    _("cannot represent %s relocation in object file"),
		    bfd_get_reloc_code_name (fix->fx_r_type));
      return NULL;
    }
  return reloc;
}

const char *
md_atof (int type, char *literal, int *size)
{
  return ieee_md_atof (type, literal, size, false);
}

enum options
{
  OPTION_NONE = OPTION_MD_BASE
};

const struct option md_longopts[] =
{
  { NULL, no_argument, NULL, 0 }
};
const size_t md_longopts_size = sizeof (md_longopts);
const char md_shortopts[] = "";

int
md_parse_option (int option ATTRIBUTE_UNUSED,
		 const char *argument ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_show_usage (FILE *stream ATTRIBUTE_UNUSED)
{
}

const nxu16_exp_mod_data_t nxu16_exp_mod_data[] =
{
  { "", 0, BFD_RELOC_NONE },
  { "lo8", 1, BFD_RELOC_NXU16_LO8 },
  { "hi8", 1, BFD_RELOC_NXU16_HI8 },
  { "hlo8", 1, BFD_RELOC_NXU16_HLO8 },
  { "hh8", 1, BFD_RELOC_NXU16_HLO8 },
  { "hhi8", 1, BFD_RELOC_NXU16_HHI8 }
};

const nxu16_exp_mod_data_t *
nxu16_parse_cons_expression (expressionS *exp, int nbytes)
{
  char *start = nxu16_skip_space (input_line_pointer);
  unsigned int i;

  for (i = 1; i < ARRAY_SIZE (nxu16_exp_mod_data); ++i)
    {
      const nxu16_exp_mod_data_t *modifier = &nxu16_exp_mod_data[i];
      size_t length = strlen (modifier->name);
      char *p;

      if (nbytes != modifier->nbytes
	  || strncasecmp (start, modifier->name, length) != 0)
	continue;
      p = nxu16_skip_space (start + length);
      if (*p != '(')
	continue;
      input_line_pointer = nxu16_skip_space (p + 1);
      expression (exp);
      input_line_pointer = nxu16_skip_space (input_line_pointer);
      if (*input_line_pointer != ')')
	{
	  as_bad (_("`)' required"));
	  return &nxu16_exp_mod_data[0];
	}
      ++input_line_pointer;
      if (exp->X_op == O_constant)
	{
	  exp->X_add_number
	    = ((valueT) exp->X_add_number
	       >> nxu16_modifier_shift (modifier->reloc)) & 0xff;
	  return &nxu16_exp_mod_data[0];
	}
      return modifier;
    }
  input_line_pointer = start;
  expression (exp);
  return &nxu16_exp_mod_data[0];
}

void
nxu16_cons_fix_new (fragS *frag, int where, int nbytes, expressionS *exp,
		    const nxu16_exp_mod_data_t *modifier)
{
  bfd_reloc_code_real_type reloc = modifier->reloc;
  fixS *fix;

  if (reloc == BFD_RELOC_NONE)
    {
      if (nbytes == 1)
	reloc = BFD_RELOC_8;
      else if (nbytes == 2)
	reloc = BFD_RELOC_16;
      else if (nbytes == 4)
	reloc = BFD_RELOC_32;
      else
	{
	  as_bad (_("unsupported data relocation size: %d"), nbytes);
	  return;
	}
    }
  fix = fix_new_exp (frag, where, nbytes, exp, false, reloc);

  /* The value represented by a byte-selector relocation is the selected
     byte, not the complete symbol address.  Mark this at fix creation time:
     GAS performs its generic n-byte overflow check before md_apply_fix, so
     doing it only in md_apply_fix is too late for locally resolved symbols.  */
  if (reloc >= BFD_RELOC_NXU16_LO8
      && reloc <= BFD_RELOC_NXU16_HHI8)
    fix->fx_no_overflow = true;
}
