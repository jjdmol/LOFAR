INSTHDRS = \
DH_PL.h \
PO_DH_PL.h \
TH_PL.h

NOINSTHDRS =

TCCHDRS = 

pkginclude_HEADERS =
nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libtransportpl.la 

libtransportpl_la_DEPENDENCIES	= $(LOFAR_DEPEND)

libtransportpl_la_SOURCES = \
DH_PL.cc \
PO_DH_PL.cc \
TH_PL.cc


include $(lofar_sharedir)/Makefile.common

