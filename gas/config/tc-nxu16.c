/* tc-nxu16.c -- Assemble code for nxu16
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

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to
   the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "as.h"
#include "opcode/nxu16.h"
#include "elf/nxu16.h"
#include "safe-ctype.h"

#define WARN_IFNE0(op_end)                                                     \
  if (*op_end != 0)                                                            \
    as_warn (_ ("extra stuff on line ignored"));

const char comment_chars[] = "//";
const char line_separator_chars[] = ";";
const char line_comment_chars[] = "//";

static int pending_reloc;
static htab_t opcode_hash_control;

static inline char *
skip_space (char *s)
{
  while (is_whitespace (*s))
    ++s;
  return s;
}
static inline char *
skip_delims (char *s)
{
  while (*s != '\0' && (is_whitespace (*s) || *s == ',')
         && !is_end_of_stmt (*s))
    s++;
  return s;
}
static int popcount32(uint32_t x) {
    int count = 0;
    while (x) {
        count++;
        x &= x - 1;
    }
    return count;
}
static void
nxu16_rodata (int x)
{
  char *save_line = input_line_pointer;
  static char section[] = ".rodata\n";

  /* Just pretend this is .section .rodata */
  input_line_pointer = section;
  obj_elf_section (x);
  input_line_pointer = save_line;
}

const pseudo_typeS md_pseudo_table[]
    = { { "rodata", nxu16_rodata, 0 }, { 0, 0, 0 } };

const char FLT_CHARS[] = "rRsSfFdDxXpP";
const char EXP_CHARS[] = "eE";

// static valueT md_chars_to_number (char * buf, int n);

void
md_operand (expressionS *op __attribute__ ((unused)))
{
  /* Empty for now. */
}

/* This function is called once, at assembler startup time.  It sets
   up the hash table with all the opcodes in it, and also initializes
   some aliases for compatibility with other assemblers.  */

void
md_begin (void)
{
  size_t count;
  const nxu16_opc_info_t *opcode;
  opcode_hash_control = str_htab_create ();

  /* Insert names into hash table.  */
  for (count = 0, opcode = nxu16_opc_info;
       count++ < (sizeof (nxu16_opc_info) / sizeof (nxu16_opc_info_t));
       opcode++)
    str_hash_insert (opcode_hash_control, opcode->name, opcode, 0);

  bfd_set_arch_mach (stdoutput, TARGET_ARCH, 0);
}

/* Parse an expression and then restore the input line pointer.  */

static char *
parse_exp_save_ilp (char *s, expressionS *op)
{
  char *save = input_line_pointer;

  input_line_pointer = s;
  expression (op);
  s = input_line_pointer;
  input_line_pointer = save;
  return s;
}

static int
nxu16_check_pattern (const char *pattern, const char *str)
{
  const char *percent = strchr (pattern, '%');
  if (!percent || percent[1] != '\0')
    {
      return strcmp (pattern, str) == 0 ? 0 : -1;
    }
  size_t prefix_len = percent - pattern;
  if (strncmp (pattern, str, prefix_len) != 0)
    {
      return -1;
    }
  const char *num_str = str + prefix_len;
  if (*num_str == '\0')
    {
      return -1;
    }
  char *endptr;
  long num = strtol (num_str, &endptr, 10);
  if (*endptr != '\0' || num < 0)
    {
      return -1;
    }
  return (int)num;
}
static int
nxu16_operand_type_caculate (char *str, nxu16_ArgType type)
{
  int num = 0;
  switch (type)
    {
    case ARG_REG_Rn:
      return (num = nxu16_check_pattern ("r%", str),
              (num >= 0 && num <= 15 && num % 1 == 0) ? num : -1);
    case ARG_REG_ERn:
      return (num = (num = nxu16_check_pattern ("er%", str),
                     (num >= 0 && num <= 15 && num % 2 == 0) ? num / 2 : -1),
              num >= 0
                  ? num
                  : (num = (nxu16_check_pattern ("fp", str) == 0 ? 14 / 2 : -1),
                     num >= 0 ? num
                              : (nxu16_check_pattern ("bp", str) == 0 ? 12 / 2
                                                                      : -1)));
    case ARG_REG_XRn:
      return (num = nxu16_check_pattern ("xr%", str),
              (num >= 0 && num <= 15 && num % 4 == 0) ? num / 4 : -1);
    case ARG_REG_QRn:
      return (num = nxu16_check_pattern ("qr%", str),
              (num >= 0 && num <= 15 && num % 8 == 0) ? num / 8 : -1);
    case ARG_REG_CRn:
      return (num = nxu16_check_pattern ("cr%", str),
              (num >= 0 && num <= 15 && num % 1 == 0) ? num : -1);
    case ARG_REG_CERn:
      return (num = nxu16_check_pattern ("cer%", str),
              (num >= 0 && num <= 15 && num % 2 == 0) ? num / 2 : -1);
    case ARG_REG_CXRn:
      return (num = nxu16_check_pattern ("cxr%", str),
              (num >= 0 && num <= 15 && num % 4 == 0) ? num / 4 : -1);
    case ARG_REG_CQRn:
      return (num = nxu16_check_pattern ("cqr%", str),
              (num >= 0 && num <= 15 && num % 8 == 0) ? num / 8 : -1);
    case ARG_REG_SP:
      return nxu16_check_pattern ("sp", str);
    case ARG_REG_PSW:
      return nxu16_check_pattern ("psw", str);
    case ARG_REG_ELR:
      return nxu16_check_pattern ("elr", str);
    case ARG_REG_ECSR:
      return nxu16_check_pattern ("ecsr", str);
    case ARG_REG_EPSW:
      return nxu16_check_pattern ("epsw", str);

    case ARG_IMM3:
    case ARG_IMM6:
    case ARG_IMM7_SIGNED:
    case ARG_IMM8:
      {
        expressionS exp;
        parse_exp_save_ilp (str, &exp);
        if (exp.X_op == O_illegal || exp.X_op == O_absent)
          {
            as_bad (_("Invalid immediate expression '%.*s'"), (int)strlen (str), str);
            return -1;
          }
        if (exp.X_op != O_constant)
          {
            return -1; // Symbolic expression
          }
        long value = exp.X_add_number;

        // Validate ranges based on type
        switch (type)
          {
          case ARG_IMM3:
            if (value < 0 || value > 7)
              {
                as_bad (_("Immediate value %ld out of range [0-7]"), value);
                return -1 & 0x07;
              }
            return value;

          case ARG_IMM6:
            if (value < 0 || value > 63)
              {
                as_bad (_("Immediate value %ld out of range [0-63]"), value);
                return -1;
              }
            return value & 0x3F;

          case ARG_IMM7_SIGNED:
            if (value < -64 || value > 63)
              {
                as_bad (_("Immediate value %ld out of range [-64-63]"), value);
                return -1;
              }
            return value & 0x7F;

          case ARG_IMM8:
            if (value < -128 || value > 255)
              {
                as_bad (_("Immediate value %ld out of range [-128-255]"), value);
                return -1;
              }
            return value & 0xFF;
            default:
            return -1;
          }
        return -1;
      }

    case ARG_MEM_EA:
      return nxu16_check_pattern ("[ea]", str);
    case ARG_MEM_EAP:
      return nxu16_check_pattern ("[ea+]", str);
    case ARG_MEM_ERn:
      return (str[0] == '[' && strchr (str, ']') != NULL
                      && strchr (str, ']')[1] == '\0'
                  ? (strchr (str, ']')[0] = '\0',
                     nxu16_operand_type_caculate (str + 1, ARG_REG_ERn))
                  : -1);

    case ARG_MEM_Dadr:   // uint16_t direct address
    case ARG_FLASH_Cadr: // int8_t constant address
      {

        expressionS exp;
        parse_exp_save_ilp (str, &exp);

        if (exp.X_op == O_illegal || exp.X_op == O_absent)
          {
            as_bad (_("Invalid address expression '%.*s'"), (int)strlen (str), str);
            return -1;
          }

        if (exp.X_op != O_constant)
          {
            return -1; // Symbolic expression
          }

        long value = exp.X_add_number;

        if (type == ARG_MEM_Dadr)
          {
            // uint16_t range: 0-65535
            if (value < 0 || value > 0xFFFF)
              {
                as_bad (_("Direct address %ld out of range [0-65535]"), value);
                return -1;
              }
            return value;
          }
        else
          { // ARG_FLASH_Cadr
            // int8_t range: -128 to 127
            if (value < -128 || value > 127)
              {
                as_bad (_("Constant address %ld out of range [-128-127]"), value);
                return -1;
              }
            return value;
          }
      }

    case ARG_FLASH_Radr: // uint20_t flash address
      {
      char *s=str;
        unsigned long value = 0;
        char *colon = strchr (s, ':');

        if (colon)
          {
            // Handle X:XXXX format
            if (colon != s + 1 || *(colon + 1) == '\0')
              {
                as_bad (_(
                    "Flash address must be in X:XXXX format when using colon"));
                return -1;
              }

            // Extract high nibble (4 bits)
            char *endptr;
            value = strtoul (s, &endptr, 16);
            if (endptr != colon || value > 0xF)
              {
                as_bad (_("Invalid hex digit in flash address high part"));
                return -1;
              }
            value <<= 16;
            s = colon + 1;
          }
        else
          {
            // Handle 0x... format
            if (s[0] != '0' || (s[1] != 'x' && s[1] != 'X'))
              {
                as_bad (_("Flash address must start with 0x when not in X:XXXX format"));
                return -1;
              }
            s += 2; // skip 0x prefix
          }

        // Parse low part (16 bits if colon format, full value if 0x format)
        if (*s == '\0')
          {
            as_bad (_("Missing flash address value"));
            return -1;
          }

        char *endptr;
        unsigned long low = strtoul (s, &endptr, 16);
        if (*endptr != '\0')
          {
            as_bad (_("Invalid hex digits in flash address"));
            return -1;
          }

        if (colon)
          {
            // Colon format: low part must be exactly 4 hex digits
            if (endptr - s != 4 || low > 0xFFFF)
              {
                as_bad (_("Flash address low part must be exactly 4 hex digits"));
                return -1;
              }
            value |= low;
          }
        else
          {
            // 0x format: no digit count limit, just range check
            value = low;
          }

        // Final range check (20 bits)
        if (value > 0xFFFFF)
          {
            as_bad (_("Flash address %lx exceeds 20 bits"), value);
            return -1;
          }

        return value;
      }

    case ARG_MEM_ERnDisp16:
    case ARG_MEM_FPDisp6:
    case ARG_MEM_BPDisp6:
      {
        if (strchr (str, '[') == str || strchr (str, '[') == NULL)
          return -1;
        int ern = nxu16_operand_type_caculate (strchr (str, '['), ARG_MEM_ERn);
        if (ern < 0)
          return ern;
        strchr (str, '[')[0] = '\0';
        expressionS arg;
        parse_exp_save_ilp (str, &arg);
        if ((arg.X_op == O_illegal || arg.X_op == O_absent)
            || (type != ARG_MEM_ERnDisp16 && arg.X_op != O_constant)
            || (arg.X_op == O_constant
                && (arg.X_add_number < -32768 || arg.X_add_number > 65535))

        )
          {
            as_bad (_("Invalid Disp"));
            return -1;
          }
        else if (type == ARG_MEM_ERnDisp16 && arg.X_op != O_constant)
          {
            return -2;
          }
        // TODO:distinguish 0xffff and 65535
        if (type == ARG_MEM_ERnDisp16)
          {
            if ((ern == 14 / 2 || ern == 12 / 2)
                && (arg.X_add_number >= -32 && arg.X_add_number <= 31))
              return -1;
            else
              return ((ern & 0x07) << 16 | (arg.X_add_number & 0xffff));
          }
        else if (((ern == 14 / 2 && type == ARG_MEM_FPDisp6)
                  || (ern == 12 / 2 && type == ARG_MEM_BPDisp6))
                 && (arg.X_add_number >= -32 && arg.X_add_number <= 31))
          {
            return ((ern & 0x07) << 6 | (arg.X_add_number & 0x3f));
          }
        return -1;
      }

    case ARG_SYMBOL:
      {

        expressionS exp;
        parse_exp_save_ilp (str, &exp);
        if (exp.X_op == O_illegal || exp.X_op == O_absent)
          {
            as_bad (_("Invalid symbol expression '%.*s'"),
                    (int)strlen(str), str);
            return -1;
          }

        if (exp.X_op == O_constant)
          {
            as_bad (
                _("Internal error,constant shouldn't be treated as symbol '%.*s'"),
                (int)strlen (str), str);
            return -1;
          }
        return -2;
      }
    default:
      return -1;
    }
}
// a helper
static int
nxu16_operand_type_caculate_save (char *str, nxu16_ArgType type)
{
  char *new_str = (char *)malloc (strlen (str) + 1);
  if (new_str == NULL)
    {
      as_bad (_("Memory Panic"));
      return -1;
    }
  strcpy (new_str, str);
  int rt = nxu16_operand_type_caculate (new_str, type);
  if ((type == ARG_IMM3 || type == ARG_IMM6 
       || type == ARG_IMM8 || type == ARG_MEM_Dadr || type == ARG_FLASH_Cadr
       || type == ARG_FLASH_Radr)
      && (rt == -1))
    {
      strcpy (new_str, str);
      rt = nxu16_operand_type_caculate (new_str, ARG_SYMBOL);
    }
  free (new_str);
  return rt;
}

/* This is the guts of the machine-dependent assembler. STR points to
   a machine dependent instruction. This function is supposed to emit
   the frags/bytes it assembles to. */
void
md_assemble (char *str)
{
  char *tokens[3] = { NULL, NULL, NULL };
  int token_count = 0;
  char *p = str;
  nxu16_opc_info_t *opcode;


  p = skip_delims (p);

  while (token_count < 3 && !is_end_of_stmt (*p) && *p != '\0')
    {
      tokens[token_count] = p;
      token_count++;

    //TODO:MUST expression will have space
      while (!is_end_of_stmt (*p) && *p != '\0' && !is_whitespace (*p)
             && *p != ',')
        {
          p++;
        }

      /* Check if we're at end of statement */
      if (is_end_of_stmt (*p) || *p == '\0')
        {
          /* Terminate token and exit */
          *p = '\0';
          break;
        }
      else
        {
          /* Terminate token and move to next character */
          *p++ = '\0';
        }
      if (token_count == 2
          && (strcmp (tokens[0], "push") == 0
              || strcmp (tokens[0], "pop") == 0))
        {
          break;
        }
      /* Skip subsequent delimiters */
      p = skip_delims (p);

      /* Check for end after skipping delimiters */
      if (is_end_of_stmt (*p) || *p == '\0')
        {
          break;
        }
    }

  /* Handle extra tokens */
  if (token_count == 3)
    {
      char *temp = skip_delims (p);
      if (!is_end_of_stmt (*temp) && *temp != '\0')
        {
          as_bad (_ ("too many operands"));
          return;
        }
    }

  /* Validate we have at least one token */
  if (token_count == 0 || tokens[0][0] == '\0')
    {
      as_bad (_ ("can't find opcode"));
      return;
    }

  /* Look up opcode */
  opcode = (nxu16_opc_info_t *)str_hash_find (opcode_hash_control, tokens[0]);
  if (opcode == NULL)
    {
      as_bad (_ ("unknown opcode %s"), tokens[0]);
      return;
    }
  nxu16_OpcodeCombination *c;
  int matched = 0;
  for (size_t i = 0; i < opcode->num_combinations; i++)
    {
      c = &opcode->combinations[i];

      // Check argument count match
      if ((size_t)token_count-1 != c->num_args)
        continue;

      // Check each operand type
      int valid = 1;
      for (int j = 0; j < token_count; j++)
        {
          if (nxu16_operand_type_caculate_save (tokens[j + 1], c->args[j].type)
              == -1)
            {
              valid = 0;

              break;
            }
        }

      if (valid)
        {
          matched = 1;
          break;
        }
    }
  if (!matched)
    {
      as_bad (_("Invalid operands\n"));
      return;
    }
    uint32_t iword=(c->opcode)&(c->opcode_mask);
     p = frag_more (c->opcode_length);
    for (int j = 0; (size_t)j < c->num_args; j++) {
        int data = nxu16_operand_type_caculate_save(tokens[j + 1], c->args[j].type);
        if (data>=0){
                uint32_t arg_mask = c->args[j].mask;
        int total_bits = popcount32(arg_mask);

        // Range check for unsigned data
        if (data < 0) {
            as_bad( _("Error: negative value %d for unsigned field\n"), data);
            return;
        }
        if (total_bits < 32) {
            uint32_t max_val = (1U << total_bits) - 1;
            if ((uint32_t)data > max_val) {
                as_bad( _("Error: value %u too large for %d-bit unsigned field\n"), data, total_bits);
                return;
            }
        }

        // Clear the argument's bits in iword
        iword &= ~arg_mask;

        // Assign data bits to the positions specified by the mask
        uint32_t temp = 0;
        int bit_index = 0;
        for (int i = 0; i < 32; i++) {
            if (arg_mask & (1U << i)) {
                if (data & (1U << bit_index)) {
                    temp |= (1U << i);
                }
                bit_index++;
            }
        }

        iword |= temp;}else if(data==-2){//TODO:MUST realize lo8()
        //TODO: disp
        expressionS arg;
        parse_exp_save_ilp (tokens[j+1], &arg);
/*        #define RELOC_ENUM enum bfd_reloc_code_real
        RELOC_ENUM r_type;
        long byte=1,offset=0;
  switch (c->args[j].type)
    {
    case ARG_IMM3:
      r_type= R_NXU16_IMM3;
      byte=1;
        break;
    case ARG_IMM6:
      r_type= R_NXU16_IMM6;
      byte=1;
        break;
    case ARG_IMM8:
      r_type= R_NXU16_IMM8;
      byte=1;
        break;
    case ARG_IMM16:
      r_type= R_NXU16_IMM16;
      byte=2;
      offset=2;
        break;
    case ARG_MEM_Dadr:
      r_type= R_NXU16_MEM_Dadr;
      byte=2;
      offset=2;
        break;
    case ARG_FLASH_Cadr:
      r_type= R_NXU16_FLASH_Cadr;
            byte=4;
      
        break;
    case ARG_FLASH_Radr:
      r_type= R_NXU16_FLASH_Radr;
      byte=1;
      break;
    default:
     as_bad(_("Illegal symbol\n"));
      r_type= R_NXU16_NONE;
      break;
    }

        fix_new_exp (frag_now,
                         (p - frag_now->fr_literal +offset),
                         byte,
                         &arg,
                         (S_GET_SEGMENT(arg.X_add_symbol) == text_section) ?1:0,
                         r_type);
        */
        }else{as_bad(_("Unknow operand data\n"));}
    }
     
if(c->opcode_length==2){
md_number_to_chars(p, iword, 2);
}else if(c->opcode_length==4){
md_number_to_chars(p, iword>>16, 2);
md_number_to_chars(p+2, iword&0xffff, 2);
}else{
as_bad(_("Internal error:bad opcode length\n"));
}
  // md_number_to_chars(p, iword, 2);
  dwarf2_emit_insn (2);

  // SKIP_SPACES(op_end);
  // WARN_IFNE0(op_end);

  if (pending_reloc)
    as_bad (_ ("Something forgot to clean up\n"));
}

/* Turn a string in input_line_pointer into a floating point constant
   of type type, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP .  An error message is
   returned, or NULL on OK.  */

const char *
md_atof (int type, char *litP, int *sizeP)
{
  int prec;
  LITTLENUM_TYPE words[4];
  char *t;
  int i;

  switch (type)
    {
    case 'f':
      prec = 2;
      break;

    case 'd':
      prec = 4;
      break;

    default:
      *sizeP = 0;
      return _ ("bad call to md_atof");
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;

  *sizeP = prec * 2;

  for (i = prec - 1; i >= 0; i--)
    {
      md_number_to_chars (litP, (valueT)words[i], 2);
      litP += 2;
    }

  return NULL;
}

enum options
{
  OPTION_NONE = OPTION_MD_BASE,
};

const struct option md_longopts[] = { { NULL, no_argument, NULL, 0 } };

const size_t md_longopts_size = sizeof (md_longopts);

const char md_shortopts[] = "";

int
md_parse_option (int c ATTRIBUTE_UNUSED, const char *arg ATTRIBUTE_UNUSED)
{
  return 0;
}

void
md_show_usage (FILE *stream ATTRIBUTE_UNUSED)
{
  return;
}

/* Apply a fixup to the object file.  */
void
md_apply_fix (fixS *fixP ATTRIBUTE_UNUSED, valueT *valP ATTRIBUTE_UNUSED,
              segT seg ATTRIBUTE_UNUSED)
{
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  long val = *valP;
  // long newval;
  long max, min;

  max = min = 0;
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_32:
      buf[3] = val >> 24;
      buf[2] = val >> 16;
      buf[1] = val >> 8;
      buf[0] = val >> 0;
      buf += 4;
      break;
    case BFD_RELOC_16:
      buf[1] = val >> 8;
      buf[0] = val >> 0;
      buf += 2;
      break;
    case BFD_RELOC_8:
      *buf++ = val;
      break;
    default:
      abort ();
    }

  if (max != 0 && (val < min || val > max))
    as_bad_where (fixP->fx_file, fixP->fx_line, _ ("offset out of range"));

  if (fixP->fx_addsy == NULL && fixP->fx_pcrel == 0)
    fixP->fx_done = 1;
}

/* Put number into target byte order.  */

void
md_number_to_chars (char *ptr, valueT use, int nbytes)
{
  number_to_chars_littleendian (ptr, use, nbytes);
}

/* Convert from target byte order to host byte order.  */
/*
static valueT
md_chars_to_number (char *buf, int n)
{
  valueT result = 0;
  unsigned char *where = (unsigned char *) buf;

  while (n--)
    {
      result <<= 8;
      result |= (where[n] & 255);
    }

  return result;
}*/

/* Generate a machine-dependent relocation.  */
arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixP)
{
  arelent *relP;
  bfd_reloc_code_real_type code;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_16:
      code = fixP->fx_r_type;
      break;
    case BFD_RELOC_32:
      code = fixP->fx_r_type;
      break;
    default:
      as_bad_where (fixP->fx_file, fixP->fx_line,
                    _ ("Semantics error.  This type of operand can not be "
                       "relocated, it must be an assembly-time constant"));
      return 0;
    }

  relP = XNEW (arelent);
  relP->sym_ptr_ptr = XNEW (asymbol *);
  *relP->sym_ptr_ptr = symbol_get_bfdsym (fixP->fx_addsy);
  relP->address = fixP->fx_frag->fr_address + fixP->fx_where;

  relP->addend = fixP->fx_offset;

  /* This is the standard place for KLUDGEs to work around bugs in
     bfd_install_relocation (first such note in the documentation
     appears with binutils-2.8).

     That function bfd_install_relocation does the wrong thing with
     putting stuff into the addend of a reloc (it should stay out) for a
     weak symbol.  The really bad thing is that it adds the
     "segment-relative offset" of the symbol into the reloc.  In this
     case, the reloc should instead be relative to the symbol with no
     other offset than the assembly code shows; and since the symbol is
     weak, any local definition should be ignored until link time (or
     thereafter).
     To wit:  weaksym+42  should be weaksym+42 in the reloc,
     not weaksym+(offset_from_segment_of_local_weaksym_definition)

     To "work around" this, we subtract the segment-relative offset of
     "known" weak symbols.  This evens out the extra offset.

     That happens for a.out but not for ELF, since for ELF,
     bfd_install_relocation uses the "special function" field of the
     howto, and does not execute the code that needs to be undone.  */

  if (OUTPUT_FLAVOR == bfd_target_aout_flavour && fixP->fx_addsy
      && S_IS_WEAK (fixP->fx_addsy)
      && !bfd_is_und_section (S_GET_SEGMENT (fixP->fx_addsy)))
    {
      relP->addend -= S_GET_VALUE (fixP->fx_addsy);
    }

  relP->howto = bfd_reloc_type_lookup (stdoutput, code);
  if (!relP->howto)
    {
      const char *name;

      name = S_GET_NAME (fixP->fx_addsy);
      if (name == NULL)
        name = _ ("<unknown>");
      as_fatal (_ ("Cannot generate relocation type for symbol %s, code %s"),
                name, bfd_get_reloc_code_name (code));
    }

  return relP;
}

/* Decide from what point a pc-relative relocation is relative to,
   relative to the pc-relative fixup.  Er, relatively speaking.  */
long
md_pcrel_from (fixS *fixP)
{
  valueT addr = fixP->fx_where + fixP->fx_frag->fr_address;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_16:
      return addr + 2;
    case BFD_RELOC_32:
      return addr + 4;
    default:
      as_fatal (_ ("fx_r_type: %d"), fixP->fx_r_type);
      abort ();
      return addr;
    }
}
