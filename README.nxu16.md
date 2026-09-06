# nX-U16 Binutils Port

This port targets the LAPIS nX-U16/100 ISA described by
`FEUL-U16-100-INST-03`.  The assembler and linker implementation is based on
normal GAS expressions, fixups, BFD relocations, and GAS relaxation rather
than target-local tokenization or link-time byte patching.

## Build

Use an out-of-tree build.  The following is the configuration used for the
current regression build:

```sh
mkdir build-nxu16
cd build-nxu16
../configure --target=nxu16 \
  --disable-gdb --disable-gdbserver --disable-sim --disable-nls \
  --enable-as --enable-ld
make -j4 all-binutils all-gas all-ld
```

The resulting programs are `gas/as-new`, `ld/ld-new`, and the utilities in
`binutils/`.

## Expressions and modifiers

Instruction operands are parsed with GAS `expression()`.  Whitespace and
parenthesized arithmetic therefore work normally, for example:

```asm
mov r0, 1 + 2 * 3
l   r4, object + (4 * 2)
mov r1, lo8(object + 3)
mov r2, hi8(object + 3)
```

The target supports `lo8()`, `hi8()`, `hlo8()` (alias `hh8()`), and `hhi8()`
as relocation-producing modifiers.  They are accepted in instruction
operands and data directives such as `.byte`.

## Branch relaxation

`b label` selects the two-byte PC-relative form by default.  Its signed
eight-bit field is a displacement in halfwords from the next instruction,
giving a range of -128 through +127 halfwords.

GAS relaxes an out-of-range unconditional branch to the four-byte `B Cadr`
form.  It relaxes an out-of-range conditional branch to:

```asm
b<inverse-condition> .+6
b target                 /* encoded as the four-byte Cadr form */
```

The linker applies `R_NXU16_FLASH_Radr` relative to the next instruction and
checks alignment and signed range.  `R_NXU16_FLASH_Cadr` writes the split
20-bit code address into the two instruction words.

## Far memory and code banks

GAS accepts native selector-address syntax such as `r2:[er0]` for a far memory
operand. Existing `DSR:[ERn]` and immediate-DSR forms remain available. The
`R_NXU16_32` relocation carries the compiler's four-byte far-pointer container
through assembly and linking.

Near indirect branches (`B ERn`/`BL ERn`) retain the current CSR segment, so
the linker treats every executable `.text.*` input section as one indivisible
function. It first moves later functions into holes at lower-bank tails, then
pads a function that would otherwise cross a 64 KiB boundary. A function over
64 KiB, or one that still crosses after placement, is a link error. This policy
requires compiler output with one function per section.

## Regression coverage

The NXU16 GAS tests cover native expressions, byte modifiers, direct-address
relocation, register lists, near/far branches, and far memory syntax. The
NXU16 LD tests cover lower-bank hole filling and bank-overflow diagnostics.
Run them through the Binutils testsuite when DejaGnu is installed:

```sh
make check-gas RUNTESTFLAGS=nxu16.exp
make check-ld RUNTESTFLAGS=nxu16.exp
```

It can also be checked without DejaGnu:

```sh
gas/as-new ../gas/testsuite/gas/nxu16/expressions.s -o expressions.o
ld/ld-new -o expressions.elf expressions.o
binutils/objdump -dr expressions.o
binutils/objdump -dr expressions.elf
```

## Current boundary

Assembler/linker expression and branch paths are usable and have been tested
end to end.  This does not by itself certify every opcode spelling or every
relocation combination in the complete ISA.  Keep adding focused `.s`/`.d`
cases as compiler-generated code exercises new instruction families.
