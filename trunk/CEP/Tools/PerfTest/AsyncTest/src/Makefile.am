bin_PROGRAMS	= AsyncTest
AsyncTest_CXXFLAGS	=

DOCHDRS 	=				\
DH_GrowSize.h					\
AsyncTest.h					\
StopWatch.h					\
WH_Source.h					\
WH_Sink.h

AsyncTest_SOURCES  	= $(DOCHDRS)		\
DH_GrowSize.cc					\
AsyncTest.cc					\
Simulate.cc					\
StopWatch.cc					\
WH_Source.cc					\
WH_Sink.cc

AsyncTest_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
