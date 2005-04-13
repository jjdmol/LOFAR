lib_LTLIBRARIES 	= libtinycep.la

INSTHDRS = \
Profiler.h					\
SimulatorParseClass.h \
TinyDataManager.h \
TinyApplicationHolder.h \
WorkHolder.h \
Selector.h \
Sel_Linked.h \
Sel_RoundRobin.h 

NOINSTHDRS =

TCCHDRS = 

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

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

libtinycep_la_SOURCES =  \
Profiler.cc			\
SimulatorParse.yy               \
SimulatorParse.h                \
SimulatorTokenize.ll            \
SimulatorParseClass.cc          \
TinyDataManager.cc 		\
TinyApplicationHolder.cc 	\
WorkHolder.cc 			\
Selector.cc 			\
Sel_Linked.cc 			\
Sel_RoundRobin.cc 		

EXTRA_DIST = assay

AM_YFLAGS               = -d -p SimulatorParse
AM_LFLAGS               = -PSimulatorTokenize
LEX_OUTPUT_ROOT         = lex.SimulatorTokenize

include $(lofar_sharedir)/Makefile.common
