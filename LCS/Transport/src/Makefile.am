if HAVE_CORBA
SUBDIRS                 = Corba .
else
SUBDIRS                 = .
endif

if HAVE_SHMEM
SHMEM_HDRS = TH_ShMem.h
else 
SHMEM_HDRS =
endif


INSTHDRS = \
BaseSim.h \
CyclicBuffer.h \
DataHolder.h \
DataBlobExtra.h \
TH_File.h \
TH_Mem.h \
TH_MPI.h \
TH_Socket.h \
Transporter.h \
TransportHolder.h \
$(SHMEM_HDRS)


NOINSTHDRS =

TCCHDRS = 

pkginclude_HEADERS =
nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libtransport.la 

libtransport_la_DEPENDENCIES	= $(LOFAR_DEPEND)

if HAVE_SHMEM
SHMEM_SOURCES = TH_ShMem.cc
else
SHMEM_SOURCES =
endif

libtransport_la_SOURCES = \
CyclicBuffer.cc \
DataHolder.cc \
DataBlobExtra.cc \
TH_File.cc \
TH_Mem.cc \
TH_MPI.cc \
TH_Socket.cc \
Transporter.cc \
TransportHolder.cc \
$(SHMEM_SOURCES)


if HAVE_CORBA
libtransport_la_LIBADD = Corba/libth_corba.la
endif

include $(lofar_sharedir)/Makefile.common

