# nX-U16 code-bank placement and validation.  B/BL ERn retain CSR, and PC
# overflow wraps inside the current 64 KiB segment.  Therefore every input
# function section must fit wholly within one segment.

fragment <<EOF

static bool
nxu16_code_section_crosses_segment (asection *section)
{
  bfd_vma start = section->output_section->vma + section->output_offset;
  bfd_size_type size = bfd_section_size (section);

  return size > 0x10000 || ((start & 0xffff) + size > 0x10000);
}

/* Move an input-section statement and keep the BFD input-section map in the
   same order.  This is the same pair of lists maintained by ldlang.c when a
   wildcard statement is expanded.  */
static void
nxu16_move_input_section (lang_statement_list_type *list,
			  lang_statement_union_type **from,
			  lang_statement_union_type **to)
{
  lang_statement_union_type *statement = *from;
  asection *section = statement->input_section.section;
  asection *previous;
  asection *next;

  *from = statement->header.next;
  if (*from == NULL)
    list->tail = from;

  statement->header.next = *to;
  *to = statement;
  if (list->tail == to)
    list->tail = &statement->header.next;

  next = section->map_head.s;
  previous = section->map_tail.s;
  (previous != NULL ? previous : section->output_section)->map_head.s = next;
  (next != NULL ? next : section->output_section)->map_tail.s = previous;

  next = statement->header.next->input_section.section;
  previous = next->map_tail.s;
  section->map_head.s = next;
  section->map_tail.s = previous;
  (previous != NULL ? previous : section->output_section)->map_head.s = section;
  next->map_tail.s = section;
}

/* If SECTION is the first function which does not fit at the end of a code
   segment, pull the first later function which does fit in front of it.  A
   new layout pass then repeats this first-fit operation, so useful tail space
   is consumed before a new segment is opened.  */
static bool
nxu16_fill_code_hole_in_list (lang_statement_list_type *list)
{
  lang_statement_union_type *statement;
  lang_statement_union_type **cursor;

  for (cursor = &list->head; (statement = *cursor) != NULL;
       cursor = &statement->header.next)
    {
      if (statement->header.type == lang_wild_statement_enum)
	{
	  if (nxu16_fill_code_hole_in_list
	      (&statement->wild_statement.children))
	    return true;
	  continue;
	}
      if (statement->header.type == lang_group_statement_enum)
	{
	  if (nxu16_fill_code_hole_in_list
	      (&statement->group_statement.children))
	    return true;
	  continue;
	}
      if (statement->header.type != lang_input_section_enum)
	continue;

      asection *section = statement->input_section.section;
      if ((section->flags & (SEC_ALLOC | SEC_CODE))
	  != (SEC_ALLOC | SEC_CODE)
	  || bfd_section_size (section) > 0x10000
	  || !nxu16_code_section_crosses_segment (section))
	continue;

      bfd_vma start = (section->output_section->vma
		       + section->output_offset);
      bfd_vma bank_end = (start | 0xffff) + 1;
      lang_statement_union_type **candidate_cursor;
      lang_statement_union_type *candidate;

      for (candidate_cursor = &statement->header.next;
	   (candidate = *candidate_cursor) != NULL;
	   candidate_cursor = &candidate->header.next)
	if (candidate->header.type == lang_input_section_enum)
	  {
	    asection *candidate_section = candidate->input_section.section;
	    unsigned int power = candidate_section->alignment_power;
	    bfd_vma mask = power >= sizeof (bfd_vma) * CHAR_BIT
	      ? (bfd_vma) -1 : (((bfd_vma) 1 << power) - 1);
	    bfd_vma candidate_start = (start + mask) & ~mask;

	    if ((candidate_section->flags & (SEC_ALLOC | SEC_CODE))
		== (SEC_ALLOC | SEC_CODE)
		&& bfd_section_size (candidate_section) <= 0x10000
		&& candidate_start + bfd_section_size (candidate_section)
		   <= bank_end)
	      {
		nxu16_move_input_section (list, candidate_cursor, cursor);
		return true;
	      }
	  }
    }
  return false;
}

static bool
nxu16_fill_code_holes (void)
{
  asection *output;

  for (output = link_info.output_bfd->sections;
       output != NULL;
       output = output->next)
    if ((output->flags & (SEC_ALLOC | SEC_CODE)) == (SEC_ALLOC | SEC_CODE))
      {
	lang_output_section_statement_type *output_statement
	  = lang_output_section_find (output->name);
	if (output_statement != NULL
	    && nxu16_fill_code_hole_in_list (&output_statement->children))
	  return true;
      }
  return false;
}

static bool
nxu16_pad_crossing_code_sections (void)
{
  asection *output;
  bool changed = false;

  for (output = link_info.output_bfd->sections;
       output != NULL;
       output = output->next)
    {
      asection *input;
      for (input = output->map_head.s;
           input != NULL;
           input = input->map_head.s)
	if ((input->flags & (SEC_ALLOC | SEC_CODE)) == (SEC_ALLOC | SEC_CODE)
	    && bfd_section_size (input) != 0)
	  {
	    if (bfd_section_size (input) > 0x10000)
	      einfo (_("%X%P: function section %pA is larger than one "
		       "64 KiB code segment\n"), input);
	    else if (nxu16_code_section_crosses_segment (input)
		     && input->alignment_power < 16)
	      {
		input->alignment_power = 16;
		changed = true;
	      }
	  }
    }
  return changed;
}

static void
nxu16_validate_code_sections (void)
{
  asection *output;

  for (output = link_info.output_bfd->sections;
       output != NULL;
       output = output->next)
    {
      asection *input;
      for (input = output->map_head.s;
           input != NULL;
           input = input->map_head.s)
	if ((input->flags & (SEC_ALLOC | SEC_CODE)) == (SEC_ALLOC | SEC_CODE)
	    && bfd_section_size (input) != 0
	    && nxu16_code_section_crosses_segment (input))
	  einfo (_("%X%P: function section %pA crosses a 64 KiB code "
		   "segment boundary\n"), input);
    }
}

static void
nxu16elf_after_allocation (void)
{
  unsigned int passes = 0;

  while (++passes < 128)
    {
      if (nxu16_fill_code_holes ())
	{
	  lang_relax_sections (true);
	  continue;
	}
      if (nxu16_pad_crossing_code_sections ())
	{
	  lang_relax_sections (true);
	  continue;
	}
      break;
    }

  nxu16_validate_code_sections ();
  gld${EMULATION_NAME}_after_allocation ();
}

EOF

LDEMUL_AFTER_ALLOCATION=nxu16elf_after_allocation
