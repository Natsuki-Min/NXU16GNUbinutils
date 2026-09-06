	.text
	.global _start
_start:
	.space 254
	bne _start
	.space 260
local_data:
	.byte 0
	.byte lo8(local_data), hi8(local_data)
	mov r0, lo8(local_data)
	mov r1, hi8(local_data)
