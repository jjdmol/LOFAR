INSTHDRS = \
ParameterSet.h 

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES                 = libacc.la

libacc_la_SOURCES = \
					ParameterSet.cc 

#					ApplControl.cc DH_ApplControl.cc \
#					DH_AC_Connect.cc \
#					ApplControlClient.cc \
#					ApplControlServer.cc 

include $(lofar_sharedir)/Makefile.common

#AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
