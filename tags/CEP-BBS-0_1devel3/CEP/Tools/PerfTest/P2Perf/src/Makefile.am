bin_PROGRAMS	= P2Perf

DOCHDRS 	=				\
DH_VarSize.h					\
P2Perf.h					\
StopWatch.h					\
WH_Src.h					\
WH_Dest.h					\
DHGrowStrategy.h

P2Perf_SOURCES  	= $(DOCHDRS)		\
DH_VarSize.cc					\
P2Perf.cc					\
Simulate.cc					\
StopWatch.cc					\
WH_Src.cc					\
WH_Dest.cc					\
DHGrowStrategy.cc                               

P2Perf_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
