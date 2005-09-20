KERNELDIR=/lib/modules/`uname -r`/build

moduledir = $(libdir)/modules/`uname -r`/kernel/drivers/char

module_PROGRAMS = \
	spid.o

spid_o_SOURCES = \
	spid.c

spid_o_CFLAGS = \
	-Wno-unused \
	-Wall \
	-D__KERNEL__ \
	-DMODULE \
	-I$(KERNELDIR)/include \
	-O2
