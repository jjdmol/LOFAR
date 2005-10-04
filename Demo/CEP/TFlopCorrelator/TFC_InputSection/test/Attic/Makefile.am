..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tWH_SBCollect CyclicBufferTest RSPBufferingTest CyclicCounterTest


AM_CXXFLAGS = \
	-I../../../src

tWH_SBCollect_SOURCES =                         \
tWH_SBCollect.cc                                \
../src/WH_SBCollect.cc

CyclicBufferTest_SOURCES =                      \
CyclicBufferTest.cc				\
../src/BufferController.cc                      \
../src/CyclicCounter.cc               

RSPBufferingTest_SOURCES =                      \
RSPBufferingTest.cc				\
../src/BufferController.cc                      \
../src/CyclicCounter.cc

CyclicCounterTest_SOURCES =                      \
CyclicCounterTest.cc				\
../src/CyclicCounter.cc


TESTS   = 	                                \
tWH_SBCollect.sh
TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(top_srcdir)/Makefile.common
