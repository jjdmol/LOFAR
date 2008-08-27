lib_LTLIBRARIES	= libtinycep.la

libtinycep_la_SOURCES = Package__Version.cc  		\
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

bin_PROGRAMS = versiontinycep

versiontinycep_SOURCES      = versiontinycep.cc
versiontinycep_LDADD        = libtinycep.la
versiontinycep_DEPENDENCIES = libtinycep.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
