DOCHDRS		 	= 			

lib_LTLIBRARIES 	= libtransport.la 

libtransport_la_SOURCES = 	\
$(DOCHDRS)			\
AppControllerParse.yy		\
AppControllerTokenize.ll 	\
AppControllerParseClass.cc 	\
BaseTransport.cc		\
BaseDataHolder.cc		\
ParamHolder.cc			\
DataHolder.cc			\
Transport.cc			\
Transportable.cc		\
TransportHolder.cc		\
TH_File.cc			\
TH_Mem.cc			\
TH_Database.cc			\
TH_MPI.cc			\
TH_Mem_Bl.cc			\
CyclicBuffer.cc			

EXTRA_DIST = assay

AM_YFLAGS               = -d -p AppControllerParse
AM_LFLAGS               = -PAppControllerTokenize
LEX_OUTPUT_ROOT         = lex.AppControllerTokenize

include $(lofar_sharedir)/Makefile.common

