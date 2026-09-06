/* nX-U16-specific support for 32-bit ELF.
   Copyright (C) 2009-2025 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/nxu16.h"

/* A 32-bit instruction is stored as two little-endian 16-bit words.
   Cadr is consequently not contiguous in the byte representation: bits
   19:16 occupy bits 11:8 of the first word and bits 15:0 occupy the second
   word.  Radr is a signed halfword displacement from the next instruction.
   These two relocations are installed explicitly below.  */

static reloc_howto_type nxu16_elf_howto_table[] =
{
  HOWTO (R_NXU16_NONE, 0, 0, 0, false, 0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "R_NXU16_NONE", false, 0, 0, false),
  HOWTO (R_NXU16_IMM3, 0, 1, 3, false, 4, complain_overflow_unsigned,
	 bfd_elf_generic_reloc, "R_NXU16_IMM3", false, 0, 0x70, false),
  HOWTO (R_NXU16_IMM6, 0, 1, 6, false, 0, complain_overflow_unsigned,
	 bfd_elf_generic_reloc, "R_NXU16_IMM6", false, 0, 0x3f, false),
  HOWTO (R_NXU16_IMM8, 0, 1, 8, false, 0, complain_overflow_bitfield,
	 bfd_elf_generic_reloc, "R_NXU16_IMM8", false, 0, 0xff, false),
  HOWTO (R_NXU16_IMM16, 0, 2, 16, false, 0, complain_overflow_bitfield,
	 bfd_elf_generic_reloc, "R_NXU16_IMM16", false, 0, 0xffff, false),
  HOWTO (R_NXU16_MEM_Dadr, 0, 2, 16, false, 0,
	 complain_overflow_unsigned, bfd_elf_generic_reloc,
	 "R_NXU16_MEM_Dadr", false, 0, 0xffff, false),
  HOWTO (R_NXU16_FLASH_Cadr, 0, 4, 20, false, 0,
	 complain_overflow_unsigned, bfd_elf_generic_reloc,
	 "R_NXU16_FLASH_Cadr", false, 0, 0, false),
  HOWTO (R_NXU16_FLASH_Radr, 1, 1, 8, true, 0,
	 complain_overflow_signed, bfd_elf_generic_reloc,
	 "R_NXU16_FLASH_Radr", false, 0, 0xff, true),
  HOWTO (R_NXU16_LO8, 0, 1, 8, false, 0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "R_NXU16_LO8", false, 0, 0xff, false),
  HOWTO (R_NXU16_HI8, 8, 1, 8, false, 0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "R_NXU16_HI8", false, 0, 0xff, false),
  HOWTO (R_NXU16_HLO8, 16, 1, 8, false, 0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "R_NXU16_HLO8", false, 0, 0xff, false),
  HOWTO (R_NXU16_HHI8, 24, 1, 8, false, 0, complain_overflow_dont,
	 bfd_elf_generic_reloc, "R_NXU16_HHI8", false, 0, 0xff, false),
  HOWTO (R_NXU16_32, 0, 4, 32, false, 0, complain_overflow_bitfield,
	 bfd_elf_generic_reloc, "R_NXU16_32", false, 0, 0xffffffff, false)
};

struct nxu16_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc;
  unsigned int elf_reloc;
};

static const struct nxu16_reloc_map nxu16_reloc_map[] =
{
  { BFD_RELOC_NONE, R_NXU16_NONE },
  { BFD_RELOC_NXU16_IMM3, R_NXU16_IMM3 },
  { BFD_RELOC_NXU16_IMM6, R_NXU16_IMM6 },
  { BFD_RELOC_NXU16_IMM8, R_NXU16_IMM8 },
  { BFD_RELOC_NXU16_IMM16, R_NXU16_IMM16 },
  { BFD_RELOC_16, R_NXU16_IMM16 },
  { BFD_RELOC_NXU16_MEM_Dadr, R_NXU16_MEM_Dadr },
  { BFD_RELOC_NXU16_FLASH_Cadr, R_NXU16_FLASH_Cadr },
  { BFD_RELOC_NXU16_FLASH_Radr, R_NXU16_FLASH_Radr },
  { BFD_RELOC_NXU16_LO8, R_NXU16_LO8 },
  { BFD_RELOC_NXU16_HI8, R_NXU16_HI8 },
  { BFD_RELOC_NXU16_HLO8, R_NXU16_HLO8 },
  { BFD_RELOC_NXU16_HHI8, R_NXU16_HHI8 },
  { BFD_RELOC_32, R_NXU16_32 }
};

static reloc_howto_type *
nxu16_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			 bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < sizeof (nxu16_reloc_map) / sizeof (nxu16_reloc_map[0]);
       ++i)
    if (nxu16_reloc_map[i].bfd_reloc == code)
      return &nxu16_elf_howto_table[nxu16_reloc_map[i].elf_reloc];
  return NULL;
}

static reloc_howto_type *
nxu16_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (nxu16_elf_howto_table) / sizeof (nxu16_elf_howto_table[0]);
       ++i)
    if (nxu16_elf_howto_table[i].name != NULL
	&& strcasecmp (nxu16_elf_howto_table[i].name, name) == 0)
      return &nxu16_elf_howto_table[i];
  return NULL;
}

static bool
nxu16_info_to_howto_rela (bfd *abfd, arelent *cache_ptr,
			  Elf_Internal_Rela *dst)
{
  unsigned int type = ELF32_R_TYPE (dst->r_info);

  if (type >= R_NXU16_max)
    {
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  cache_ptr->howto = &nxu16_elf_howto_table[type];
  return true;
}

static bfd_reloc_status_type
nxu16_final_link_relocate (reloc_howto_type *howto, bfd *input_bfd,
			   asection *input_section, bfd_byte *contents,
			   Elf_Internal_Rela *rel, bfd_vma relocation)
{
  bfd_vma value = relocation + rel->r_addend;
  bfd_byte *where = contents + rel->r_offset;

  switch (howto->type)
    {
    case R_NXU16_FLASH_Cadr:
      if (value > 0xfffff)
	return bfd_reloc_overflow;
      bfd_putl16 ((bfd_getl16 (where) & 0xf0ff)
		  | ((value >> 8) & 0x0f00), where);
      bfd_putl16 (value & 0xffff, where + 2);
      return bfd_reloc_ok;

    case R_NXU16_FLASH_Radr:
      value -= input_section->output_section->vma;
      value -= input_section->output_offset;
      value -= rel->r_offset + 2;
      if ((value & 1) != 0)
	return bfd_reloc_dangerous;
      {
	bfd_signed_vma displacement = (bfd_signed_vma) value / 2;
	if (displacement < -128 || displacement > 127)
	  return bfd_reloc_overflow;
	where[0] = displacement & 0xff;
	return bfd_reloc_ok;
      }

    default:
      return _bfd_final_link_relocate (howto, input_bfd, input_section,
				       contents, rel->r_offset,
				       relocation, rel->r_addend);
    }
}

static int
nxu16_elf_relocate_section (bfd *output_bfd, struct bfd_link_info *info,
			    bfd *input_bfd, asection *input_section,
			    bfd_byte *contents, Elf_Internal_Rela *relocs,
			    Elf_Internal_Sym *local_syms,
			    asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (input_bfd);
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; ++rel)
    {
      unsigned int type = ELF32_R_TYPE (rel->r_info);
      unsigned long symndx = ELF32_R_SYM (rel->r_info);
      reloc_howto_type *howto;
      struct elf_link_hash_entry *h = NULL;
      Elf_Internal_Sym *sym = NULL;
      asection *sec = NULL;
      bfd_vma relocation;
      bfd_reloc_status_type status;
      const char *name;

      if (type >= R_NXU16_max)
	{
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      input_bfd, type);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      howto = &nxu16_elf_howto_table[type];

      if (symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + symndx;
	  sec = local_sections[symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  if (name == NULL || *name == '\0')
	    name = bfd_section_name (sec);
	}
      else
	{
	  bool unresolved_reloc, warned, ignored;
	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   symndx, symtab_hdr, sym_hashes, h, sec,
				   relocation, unresolved_reloc, warned, ignored);
	  name = h->root.root.string;
	}

      if (sec != NULL && discarded_section (sec))
	{
	  RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					   rel, 1, relend, howto, 0, contents);
	  continue;
	}
      if (bfd_link_relocatable (info))
	continue;

      status = nxu16_final_link_relocate (howto, input_bfd, input_section,
					  contents, rel, relocation);
      if (status != bfd_reloc_ok)
	{
	  const char *message = NULL;
	  switch (status)
	    {
	    case bfd_reloc_overflow:
	      info->callbacks->reloc_overflow
		(info, h ? &h->root : NULL, name, howto->name, 0,
		 input_bfd, input_section, rel->r_offset);
	      break;
	    case bfd_reloc_undefined:
	      info->callbacks->undefined_symbol
		(info, name, input_bfd, input_section, rel->r_offset, true);
	      break;
	    case bfd_reloc_dangerous:
	      message = _("odd address for nX-U16 relative branch");
	      break;
	    case bfd_reloc_outofrange:
	      message = _("internal error: relocation out of range");
	      break;
	    case bfd_reloc_notsupported:
	      message = _("internal error: unsupported relocation");
	      break;
	    default:
	      message = _("internal error: unknown relocation error");
	      break;
	    }
	  if (message != NULL)
	    info->callbacks->warning (info, message, name, input_bfd,
				input_section, rel->r_offset);
	}
    }
  return true;
}

#define ELF_ARCH		bfd_arch_nxu16
#define ELF_MACHINE_CODE	EM_NXU16
#define ELF_MAXPAGESIZE		1

#define TARGET_LITTLE_SYM	nxu16_elf32_vec
#define TARGET_LITTLE_NAME	"elf32-nxu16"

#define elf_info_to_howto_rel			NULL
#define elf_info_to_howto			nxu16_info_to_howto_rela
#define elf_backend_relocate_section		nxu16_elf_relocate_section
#define elf_backend_can_gc_sections		1
#define elf_backend_rela_normal			1

#define bfd_elf32_bfd_reloc_type_lookup		nxu16_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		nxu16_reloc_name_lookup

#include "elf32-target.h"
