if HAVE_CORBA
SUBDIRS                 = ShMem Corba .
else
SUBDIRS                 = ShMem .
endif


INSTHDRS = \
BaseDataHolder.h \
BaseSim.h \
Connection.h \
CyclicBuffer.h \
DataHolder.h \
DH_PL.h \
ParamHolder.h \
PO_DH_PL.h \
TH_File.h \
TH_Mem_Bl.h \
TH_Mem.h \
TH_MPI.h \
TH_PL.h \
Transporter.h \
TransportHolder.h 

NOINSTHDRS =

TCCHDRS = 

pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libtransport.la 


libtransport_la_SOURCES = \
BaseDataHolder.cc \
Connection.cc \
CyclicBuffer.cc \
DataHolder.cc \
DH_PL.cc \
ParamHolder.cc \
PO_DH_PL.cc \
TH_File.cc \
TH_Mem_Bl.cc \
TH_Mem.cc \
TH_MPI.cc \
TH_PL.cc \
Transporter.cc \
TransportHolder.cc 


AM_YFLAGS               = -d -p AppControllerParse
AM_LFLAGS               = -PAppControllerTokenize
LEX_OUTPUT_ROOT         = lex.AppControllerTokenize

if HAVE_CORBA
libtransport_la_LIBADD = ShMem/libth_shmem.la Corba/libth_corba.la -lpthread
else
libtransport_la_LIBADD = ShMem/libth_shmem.la -lpthread
endif

include $(lofar_sharedir)/Makefile.common

