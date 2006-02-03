lib_LTLIBRARIES	= libtinycep.la

pkginclude_HEADERS = 			\
	ApplicationHolderController.h 	\
	Profiler.h			\
	Selector.h			\
	Sel_Linked.h 			\
	Sel_RoundRobin.h		\
	SimulatorParseClass.h		\
	TinyApplicationHolder.h		\
	TinyDataManager.h		\
	WorkHolder.h

libtinycep_la_SOURCES = 		\
	ApplicationHolderController.cc	\
	Profiler.cc			\
	Selector.cc 			\
	Sel_Linked.cc 			\
	Sel_RoundRobin.cc 		\
	SimulatorParseClass.cc          \
	SimulatorParse.h                \
	SimulatorParse.yy               \
	SimulatorTokenize.ll            \
	TinyApplicationHolder.cc 	\
	TinyDataManager.cc 		\
	WorkHolder.cc 

AM_YFLAGS	= -d -p SimulatorParse
AM_LFLAGS       = -PSimulatorTokenize
LEX_OUTPUT_ROOT = lex.SimulatorTokenize

include $(top_srcdir)/Makefile.common
