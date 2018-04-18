TARGET		= exec-teocpu
OBJS_TARGET	= exec.o teocpu-vm.o

CFLAGS := -g -std=gnu99 -Wall 
LIBS := 

include Makefile.in
