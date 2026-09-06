#name: Reject a function larger than one code segment
#source: bank-overflow.s
#ld: -T bank-pack.ld
#error: .*function section \.text\.too_large is larger than one 64 KiB code segment.*
#target: nxu16-*-*
