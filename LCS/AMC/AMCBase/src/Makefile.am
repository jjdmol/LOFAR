SUBDIRS				= . AMCClient

INSTHDRS			= Converter.h \
				  EarthCoord.h \
				  Exceptions.h
				  SkyCoord.h \
				  TimeCoord.h 

NOINSTHDRS			=

TCCHDRS				=

nobase_pkginclude_HEADERS	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			= $(NOINSTHDRS)

DOCHDRS				= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES			= libamcbase.la

libamcbase_la_SOURCES 		= $(DOCHDRS) \
				  EarthCoord.cc \
				  SkyCoord.cc \
				  TimeCoord.cc 

include $(lofar_sharedir)/Makefile.common
