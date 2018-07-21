TARGET		= exec-teocpu
OBJS_TARGET	= exec.o teocpu-vm.o

CFLAGS := -g -std=gnu99 -Wall 
LIBS := 

include Makefile.in

debug : Makefile Makefile.in
	$(MAKE) CFLAGS+=-D__DEBUG__ $(TARGET)

sample : Makefile Makefile.in
	 as/as crt0.s crt0.o -coff
	 as/as sample.s sample.o -coff
	 ld/ld sample.bin 0x00000 crt0.o sample.o

sampleC : crt0.s hello.c Makefile Makefile.in
	 as/as crt0.s crt0.o -coff
	 ucc/ucc -coff hello.c hello.o
	 ld/ld hello.bin 0x00000 crt0.o hello.o
	 
sampleXP : crt0.s pass0.c xprintf.c Makefile Makefile.in
	 as/as crt0.s crt0.o -coff
	 ucc/ucc -I golibc -coff pass0.c pass0.o
	 ucc/ucc -I golibc -coff xprintf.c xprintf.o
	 ld/ld xprintf.bin 0x00000 crt0.o pass0.o xprintf.o

sampleFibo : crt0.s fibo.c xprintf.c Makefile Makefile.in
	 as/as crt0.s crt0.o -coff
	 ucc/ucc -I golibc -coff fibo.c fibo.o
	 ucc/ucc -I golibc -coff xprintf.c xprintf.o
	 ld/ld fibo.bin 0x00000 crt0.o fibo.o xprintf.o

libc : golibc/*.c Makefile Makefile.in
	$(MAKE) -C golibc
