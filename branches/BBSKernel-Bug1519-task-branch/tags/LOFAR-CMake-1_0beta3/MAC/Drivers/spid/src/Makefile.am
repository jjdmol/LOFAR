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
#
# To enable the PPSAPI support in the spid driver add
# -DCONFIG_NTP_PPS_SPID to the spid_o_CFLAGS
#
#	-DCONFIG_NTP_PPS_SPID
