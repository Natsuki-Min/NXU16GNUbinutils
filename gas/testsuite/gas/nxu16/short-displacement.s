	.text
	.global short_displacement
short_displacement:
	l er0,2[er12]
	l er2,-2[er14]
	l r4,3[er12]
	l r5,-3[er14]
	st er6,4[er12]
	st er8,-4[er14]
	st r10,5[er12]
	st r11,-5[er14]
	l er0,2[er10]
