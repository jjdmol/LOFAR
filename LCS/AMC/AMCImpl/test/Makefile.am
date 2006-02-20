check_PROGRAMS			= tConverter

tConverter_SOURCES		= tConverter.cc
tConverter_LDADD		= ../src/libamcimpl.la
tConverter_DEPENDENCIES		= ../src/libamcimpl.la $(LOFAR_DEPEND)

TESTS				= tConverter.sh

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  LOFARROOT=$(LOFARROOT) \
				  AIPSPP=$(AIPSPP)

EXTRA_DIST			= $(TESTS) \
				  tConverter.run \
				  tConverter.stdout

include $(top_srcdir)/Makefile.common
