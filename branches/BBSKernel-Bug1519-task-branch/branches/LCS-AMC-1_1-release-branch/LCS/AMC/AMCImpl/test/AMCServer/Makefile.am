check_PROGRAMS			= tProcess \
				  tConverterClient \
				  tConverterServer

TESTS				= tProcess.sh \
				  tClientServer.sh

tProcess_SOURCES		= tProcess.cc
tProcess_LDADD			= ../../src/libamcimpl.la
tProcess_DEPENDENCIES		= ../../src/libamcimpl.la $(LOFAR_DEPEND)

tConverterClient_SOURCES	= tConverterClient.cc

tConverterServer_SOURCES	= tConverterServer.cc
tConverterServer_LDADD		= ../../src/libamcimpl.la
tConverterServer_DEPENDENCIES	= ../../src/libamcimpl.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT               = lofar_sharedir=$(lofar_sharedir) \
				  LOFARROOT=$(LOFARROOT) \
				  AIPSPP=$(AIPSPP)

include $(top_srcdir)/Makefile.common
