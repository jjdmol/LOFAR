
INSTHDRS = \
BaseSim.h \
DataHolder.h \
DataBlobExtra.h \
Connection.h \
ConnectionPool.h \
CSConnection.h \
TransportHolder.h \
TH_File.h \
TH_Mem.h \
TH_MPI.h \
TH_ShMem.h \
TH_Socket.h \
Lock.h \
CyclicBuffer.h


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
Connection.cc \
ConnectionPool.cc \
CSConnection.cc \
TransportHolder.cc \
TH_File.cc \
TH_Mem.cc \
TH_MPI.cc \
TH_ShMem.cc \
TH_Socket.cc \
CyclicBuffer.cc 

include $(lofar_sharedir)/Makefile.common

