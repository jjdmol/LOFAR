..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tAH_TestStorage


AM_CXXFLAGS = \
	-I../../../src

tAH_TestStorage_SOURCES =		\
tAH_TestStorage.cc			\
../src/WH_Storage.cc			\
../src/MSWriter.cc			\
../src/MSWriterImpl.cc	

TESTS   =

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(top_srcdir)/Makefile.common
