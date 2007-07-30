
check_PROGRAMS = tWH_SBCollect CyclicBufferTest  CyclicCounterTest


AM_CXXFLAGS = \
	-I../../../src

tWH_SBCollect_SOURCES =                         \
tWH_SBCollect.cc                                \
../src/WH_SBCollect.cc

CyclicBufferTest_SOURCES =                      \
CyclicBufferTest.cc				\
../src/BeamletBuffer.cc                      \
../src/CyclicCounter.cc               

CyclicCounterTest_SOURCES =                      \
CyclicCounterTest.cc				\
../src/CyclicCounter.cc


TESTS   = 	                                \
tWH_SBCollect.sh

include $(top_srcdir)/Makefile.common
