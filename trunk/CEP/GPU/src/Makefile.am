INSTHDRS = \
	GPUEnvironment.h \
	PixelBuffer.h \
	Stream.h

NOINSTHDRS =  \
	nv30glext.h

TCCHDRS = 

pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES = libGPU.la

libGPU_la_SOURCES = \
	GPUEnvironment.cc \
	PixelBuffer.cc \
	Stream.cc

include $(lofar_sharedir)/Makefile.common
