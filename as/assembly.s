	li .text
	sr %0
.loop
	li #0
	lr %0
	ldb
	sr %1
	lr %1
	tst
	ce
	li .fin
	bc
	lr %1
	li #0
	li #0xff000000
	std
	li #1
	lr %0
	add
	sr %0
	li .loop
	b
.fin
	li .fin
	b
.text
	ascii 'hello,world'
	db #0
