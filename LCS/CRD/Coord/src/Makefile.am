INSTHDRS = \
CoordClient.h \
EarthCoord.h \
Endian.h \
SkyCoord.h \
TimeCoord.h 

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libcoord.la

libcoord_la_SOURCES 	= $(DOCHDRS)		\
CoordClient.cc \
EarthCoord.cc \
Endian.cc \
SkyCoord.cc \
TimeCoord.cc 


include $(lofar_sharedir)/Makefile.common

