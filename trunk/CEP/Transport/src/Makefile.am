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
DH_PL.h \
PO_DH_PL.h \
TH_File.h \
TH_Mem.h \
TH_MPI.h \
TH_PL.h \
TH_ShMem.h \
TH_Socket.h \
Transporter.h \
TransportHolder.h 

NOINSTHDRS =

TCCHDRS = 

pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libtransport.la 


libtransport_la_SOURCES = \
CyclicBuffer.cc \
DataHolder.cc \
DataBlobExtra.cc \
DH_PL.cc \
PO_DH_PL.cc \
TH_File.cc \
TH_Mem.cc \
TH_MPI.cc \
TH_PL.cc \
TH_ShMem.cc \
TH_Socket.cc \
Transporter.cc \
TransportHolder.cc 


AM_YFLAGS               = -d -p AppControllerParse
AM_LFLAGS               = -PAppControllerTokenize
LEX_OUTPUT_ROOT         = lex.AppControllerTokenize

if HAVE_CORBA
libtransport_la_LIBADD = Corba/libth_corba.la -lpthread
else
libtransport_la_LIBADD = -lpthread
endif

include $(lofar_sharedir)/Makefile.common

