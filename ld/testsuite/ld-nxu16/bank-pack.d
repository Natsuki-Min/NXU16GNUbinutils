#name: Pack functions into the lower segment tail first
#source: bank-pack.s
#ld: -T bank-pack.ld
#objdump: -t
#target: nxu16-*-*

#...
00000000 g     F \.text[ \t]+0000ff00 large
0000ff00 g     F \.text[ \t]+00000100 small
00010000 g     F \.text[ \t]+00000200 medium
#...
