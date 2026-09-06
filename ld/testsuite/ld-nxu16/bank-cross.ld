SECTIONS
{
  /* SUBALIGN deliberately defeats the input section's 64 KiB alignment so
     validation, rather than automatic padding, must reject the crossing.  */
  .text 0xff00 : SUBALIGN(1) { *(.text.*) }
}
