..common_top_builddir=@common_top_builddir@

check_PROGRAMS = tRectMatrix tDH_SubBand


AM_CXXFLAGS = \
	-I../../../src

tRectMatrix_SOURCES =				\
tRectMatrix.cc					

tDH_SubBand_SOURCES =				\
tDH_SubBand.cc					

TESTS   = 					\
tRectMatrix.sh					\
tDH_SubBand.sh

LDADD = ../src/libtfc_interface.la
DEPENDENCIES = ../src/libtfc_interface.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

include $(lofar_sharedir)/Makefile.common
