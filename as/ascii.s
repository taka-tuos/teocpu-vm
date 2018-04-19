	li #0x30
	sr %0
.loop
	lr %0
	li #0
	li #0xff000000
	std
	lr %0
	li #127
	cmp
	ce
	li .fin
	bc
	li #1
	lr %0
	add
	sr %0
	li .loop
	b
.fin
	li .fin
	b
