..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tRectMatrix


AM_CXXFLAGS = \
	-I../../../src

tRectMatrix_SOURCES =				\
tRectMatrix.cc					\
../src/RectMatrix.cc				

TESTS   = 					\
tRectMatrix.sh

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(lofar_sharedir)/Makefile.common
