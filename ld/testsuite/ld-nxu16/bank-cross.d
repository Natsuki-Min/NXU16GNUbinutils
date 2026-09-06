#name: Reject a function forced across a code segment boundary
#source: bank-cross.s
#ld: -T bank-cross.ld
#error: .*function section \.text\.cross crosses a 64 KiB code segment boundary.*
#target: nxu16-*-*
