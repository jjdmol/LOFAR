
check_PROGRAMS = tRectMatrix 


AM_CXXFLAGS = \
	-I../../../src

tRectMatrix_SOURCES =				\
tRectMatrix.cc					

TESTS   = 					\
tRectMatrix.sh					

LDADD = ../src/libtfc_interface.la
DEPENDENCIES = ../src/libtfc_interface.la $(LOFAR_DEPEND)

EXTRA_DIST = $(TESTS)

include $(top_srcdir)/Makefile.common
