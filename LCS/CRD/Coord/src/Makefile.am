DOCHDRS		 	= 			\
CoordClient.h \
EarthCoord.h \
Endian.h \
SkyCoord.h \
TimeCoord.h 

lib_LTLIBRARIES 	= libcoord.la

libcoord_la_SOURCES 	= $(DOCHDRS)		\
CoordClient.cc \
EarthCoord.cc \
Endian.cc \
SkyCoord.cc \
TimeCoord.cc 

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL

include $(lofar_sharedir)/Makefile.common

