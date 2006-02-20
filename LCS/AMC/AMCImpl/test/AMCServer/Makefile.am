check_PROGRAMS			= tProcess

TESTS				= tProcess.sh 

tProcess_SOURCES		= tProcess.cc
tProcess_LDADD			= ../../src/libamcimpl.la
tProcess_DEPENDENCIES		= ../../src/libamcimpl.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT               = lofar_sharedir=$(lofar_sharedir) \
				  LOFARROOT=$(LOFARROOT) \
				  AIPSPP=$(AIPSPP)

include $(top_srcdir)/Makefile.common
