DOCHDRS		 	= 			

lib_LTLIBRARIES 	= libtinyframe.la libtransport.la 

#bin_PROGRAMS 		= \
# 			tinyCFmemcpy \
# 			CFinput 

# tinyCFmemcpy_SOURCES	= \
# 			WB_Memcpy.cc \
# 			DH_Memcpy.cc \
# 			Memcpy.cc

# CFinput_SOURCES		= \
# 			WH_TFinput.cc \
# 			DH_Memcpy.cc \
# 			CFinput.cc

# tinyCFmemcpy_LDADD 	= \
# 			libtinyframe.la \
# 			libtransport.la 

# CFinput_LDADD		= \
#			libcepframe.la

libtinyframe_la_SOURCES = \
AppController.cc \
WorkBlock.cc

libtransport_la_SOURCES = 	\
$(DOCHDRS)			\
AppControllerParse.yy		\
AppControllerTokenize.ll 	\
AppControllerParseClass.cc 	\
BaseTransport.cc		\
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

