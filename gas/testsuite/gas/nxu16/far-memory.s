	.text
	.global far_memory
far_memory:
	l r3,r5:[er6]
	l er4,r15:[er8]
	st r7,r9:[er10]
	st er12,r2:[er14]
	dsr r6
	l r0,2[er4]
