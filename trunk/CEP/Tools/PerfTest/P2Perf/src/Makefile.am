bin_PROGRAMS	= P2Perf
P2Perf_CXXFLAGS	=

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

include $(lofar_sharedir)/Makefile.common
