INSTHDRS 				  =	KVpair.h \
							ParameterSet.h 

NOINSTHDRS 				  =

TCCHDRS 				  =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS			  = $(NOINSTHDRS)

DOCHDRS					  = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES           = libaps.la

libaps_la_SOURCES 		  = KVpair.cc \
							ParameterSet.cc 

include $(lofar_sharedir)/Makefile.common

