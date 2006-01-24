
check_PROGRAMS = tRectMatrix 


AM_CXXFLAGS = \
	-I../../../src

tRectMatrix_SOURCES =				\
tRectMatrix.cc					

TESTS   = 					\
tRectMatrix.sh					

LDADD = ../src/libtfc_interface.la
DEPENDENCIES = ../src/libtfc_interface.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(top_srcdir)/Makefile.common
