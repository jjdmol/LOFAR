if HAVE_CORBA
SUBDIRS                 = ShMem Corba .
else
SUBDIRS                 = ShMem .
endif

DOCHDRS		 	= 			

lib_LTLIBRARIES 	= libtransport.la 

libtransport_la_SOURCES = 	\
$(DOCHDRS)			\
Transporter.cc			\
BaseDataHolder.cc		\
ParamHolder.cc			\
DataHolder.cc			\
TransportHolder.cc		\
Connection.cc			\
TH_File.cc			\
TH_Mem.cc			\
TH_MPI.cc			\
TH_Mem_Bl.cc			\
CyclicBuffer.cc			

#TH_PL.cc			\
#DH_PL.cc			

EXTRA_DIST = assay

AM_YFLAGS               = -d -p AppControllerParse
AM_LFLAGS               = -PAppControllerTokenize
LEX_OUTPUT_ROOT         = lex.AppControllerTokenize

if HAVE_CORBA
libtransport_la_LIBADD = ShMem/libth_shmem.la Corba/libth_corba.la -lpthread
else
libtransport_la_LIBADD = ShMem/libth_shmem.la -lpthread
endif

include $(lofar_sharedir)/Makefile.common

