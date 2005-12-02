check_PROGRAMS			= tConverterCommand \
				  tBlobIO \
				  tDH_Request \
				  tDH_Result

TESTS				= tConverterCommand.sh \
				  tBlobIO.sh \
				  tDH_Request.sh \
				  tDH_Result.sh

tConverterCommand_SOURCES	= tConverterCommand.cc
tConverterCommand_LDADD		= ../../src/libamcbase.la
tConverterCommand_DEPENDENCIES	= ../../src/libamcbase.la $(LOFAR_DEPEND)

tBlobIO_SOURCES			= tBlobIO.cc
tBlobIO_LDADD			= ../../src/libamcbase.la
tBlobIO_DEPENDENCIES		= ../../src/libamcbase.la $(LOFAR_DEPEND)

tDH_Request_SOURCES		= tDH_Request.cc
tDH_Request_LDADD		= ../../src/libamcbase.la
tDH_Request_DEPENDENCIES	= ../../src/libamcbase.la $(LOFAR_DEPEND)

tDH_Result_SOURCES		= tDH_Result.cc
tDH_Result_LDADD		= ../../src/libamcbase.la
tDH_Result_DEPENDENCIES		= ../../src/libamcbase.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  LOFARROOT=$(LOFARROOT)

include $(top_srcdir)/Makefile.common
