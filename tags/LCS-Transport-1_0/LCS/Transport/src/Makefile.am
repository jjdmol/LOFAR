if HAVE_CORBA
SUBDIRS                 = Corba .
else
SUBDIRS                 = .
endif


INSTHDRS = \
BaseSim.h \
CyclicBuffer.h \
DataHolder.h \
DataBlobExtra.h \
TH_File.h \
TH_Mem.h \
TH_MPI.h \
TH_ShMem.h \
TH_Socket.h \
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
CyclicBuffer.cc \
DataHolder.cc \
DataBlobExtra.cc \
TH_File.cc \
TH_Mem.cc \
TH_MPI.cc \
TH_ShMem.cc \
TH_Socket.cc \
Transporter.cc \
TransportHolder.cc 


if HAVE_CORBA
libtransport_la_LIBADD = Corba/libth_corba.la
else
libtransport_la_LIBADD = 
endif

include $(lofar_sharedir)/Makefile.common

