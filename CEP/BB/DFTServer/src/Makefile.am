INSTHDRS = \
DH_DFTRequest.h \
DH_DFTResult.h \
DFTStub.h	 \
DFTServer.h \
DFTAll.h

NOINSTHDRS = \
WH_DFTServer.h mpidft.h

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES = libdftserver.la

libdftserver_la_SOURCES = \
DH_DFTRequest.cc \
DH_DFTResult.cc \
DFTStub.cc \
DFTAll.cc \
mpidft.cc


bin_PROGRAMS = DFTServer

DFTServer_SOURCES = \
WH_DFTServer.cc \
DFTServer.cc \
DFTServer_Main.cc

DFTServer_LDADD	= libdftserver.la
DFTServer_DEPENDENCIES = libdftserver.la $(LOFAR_DEPEND)


include $(lofar_sharedir)/Makefile.common
