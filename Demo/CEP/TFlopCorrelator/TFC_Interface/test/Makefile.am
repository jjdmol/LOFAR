..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tRectMatrix tDH_FIR


AM_CXXFLAGS = \
	-I../../../src

tRectMatrix_SOURCES =				\
tRectMatrix.cc					

tDH_FIR_SOURCES =				\
../src/DH_FIR.cc				\
tDH_FIR.cc					

TESTS   = 					\
tRectMatrix.sh					\
tDH_FIR.sh

LDADD = ../src/libtfc_interface.la
DEPENDENCIES = ../src/libtfc_interface.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(top_srcdir)/Makefile.common
