..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tWH_SBCollect CyclicBufferTest


AM_CXXFLAGS = \
	-I../../../src

tWH_SBCollect_SOURCES =				\
tWH_SBCollect.cc				\
../src/WH_SBCollect.cc				

CyclicBufferTest_SOURCES =                      \
CyclicBufferTest.cc				\
../src/BufferController.cc

TESTS   = 					\
tWH_SBCollect.sh				

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(lofar_sharedir)/Makefile.common
