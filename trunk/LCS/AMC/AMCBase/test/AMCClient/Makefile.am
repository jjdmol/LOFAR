check_PROGRAMS			= tConverterCommand \
				  tBlobIO

TESTS				= tConverterCommand.sh \
				  tBlobIO.sh

tConverterCommand_SOURCES	= tConverterCommand.cc
tConverterCommand_LDADD		= ../../src/libamcbase.la
tConverterCommand_DEPENDENCIES	= ../../src/libamcbase.la $(LOFAR_DEPEND)

tBlobIO_SOURCES			= tBlobIO.cc
tBlobIO_LDADD			= ../../src/libamcbase.la
tBlobIO_DEPENDENCIES		= ../../src/libamcbase.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir)

include $(lofar_sharedir)/Makefile.common
