INSTHDRS = \
BaseSim.h \
DataHolder.h \
DataBlobExtra.h \
TH_File.h \
TH_Mem.h \
TH_MPI.h \
TH_ShMem.h \
TH_Socket.h \
TH_Ethernet.h \
Transporter.h \
TransportHolder.h 

NOINSTHDRS =

TCCHDRS = 

pkginclude_HEADERS =
nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libtransport.la 

libtransport_la_DEPENDENCIES	= $(LOFAR_DEPEND)

libtransport_la_SOURCES = \
DataHolder.cc \
DataBlobExtra.cc \
TH_File.cc \
TH_Mem.cc \
TH_MPI.cc \
TH_ShMem.cc \
TH_Socket.cc \
TH_Ethernet.cc \
Transporter.cc \
TransportHolder.cc 

include $(lofar_sharedir)/Makefile.common

