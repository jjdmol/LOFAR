#***************************************************************************
# ZEPTOOS:zepto-info
#      This file is part of ZeptoOS: The Small Linux for Big Computers.
#      See www.mcs.anl.gov/zeptoos for more information.
# ZEPTOOS:zepto-info
#
# ZEPTOOS:zepto-fillin
#      $Id$
#      ZeptoOS_Version: 1.2
#      ZeptoOS_Heredity: FOSS_ORIG
#      ZeptoOS_License: GPL
# ZEPTOOS:zepto-fillin
#
# ZEPTOOS:zepto-gpl
#       Copyright: Argonne National Laboratory, Department of Energy,
#                  and UChicago Argonne, LLC.  2004, 2005, 2006, 2007
#       ZeptoOS License: GPL
#  
#       This software is free.  See the file ZeptoOS/misc/license.GPL
#       for complete details on your rights to copy, modify, and use this
#       software.
# ZEPTOOS:zepto-gpl
#***************************************************************************

CC = /usr/bin/gcc
CFLAGS = -pthread -Wall -g -O2 -I/bgl/BlueLight/ppcfloor/bglsys/include -I../include

ifdef ZEPTO_VERSION
CFLAGS += -DZEPTO_VERSION=$(ZEPTO_VERSION)
endif
LDFLAGS = -L/bgl/BlueLight/ppcfloor/bglsys/lib -Wl,--export-dynamic,--version-script=zoid.map
LOADLIBES = -ldevices.rts -ldl

all: zoid ciod_preload.so

zoid: init.o handle_traffic.o buffers.o zoid_mapping.o cache.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES)

ciod_preload.so: ciod_preload.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^

install:
	cp zoid $(ZEPTO_INSTALLDIR)/sbin
	cp ciod_preload.so $(ZEPTO_INSTALLDIR)/lib

clean:
	rm -f zoid ciod_preload.so *.o *~
