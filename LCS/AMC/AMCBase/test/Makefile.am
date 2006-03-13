check_PROGRAMS			= tSkyCoord \
				  tEarthCoord \
				  tTimeCoord \
				  tConverterCommand \
				  tConverterStatus \
				  tBlobIO \
				  tDH_Request \
				  tDH_Result

tSkyCoord_SOURCES		= tSkyCoord.cc
tSkyCoord_LDADD			= ../src/libamcbase.la
tSkyCoord_DEPENDENCIES		= ../src/libamcbase.la $(LOFAR_DEPEND)

tEarthCoord_SOURCES		= tEarthCoord.cc
tEarthCoord_LDADD		= ../src/libamcbase.la
tEarthCoord_DEPENDENCIES	= ../src/libamcbase.la $(LOFAR_DEPEND)

tTimeCoord_SOURCES		= tTimeCoord.cc
tTimeCoord_LDADD		= ../src/libamcbase.la
tTimeCoord_DEPENDENCIES		= ../src/libamcbase.la $(LOFAR_DEPEND)

tConverterCommand_SOURCES	= tConverterCommand.cc
tConverterCommand_LDADD		= ../src/libamcbase.la
tConverterCommand_DEPENDENCIES	= ../src/libamcbase.la $(LOFAR_DEPEND)

tConverterStatus_SOURCES	= tConverterStatus.cc
tConverterStatus_LDADD		= ../src/libamcbase.la
tConverterStatus_DEPENDENCIES	= ../src/libamcbase.la $(LOFAR_DEPEND)

tBlobIO_SOURCES			= tBlobIO.cc
tBlobIO_LDADD			= ../src/libamcbase.la
tBlobIO_DEPENDENCIES		= ../src/libamcbase.la $(LOFAR_DEPEND)

tDH_Request_SOURCES		= tDH_Request.cc
tDH_Request_LDADD		= ../src/libamcbase.la
tDH_Request_DEPENDENCIES	= ../src/libamcbase.la $(LOFAR_DEPEND)

tDH_Result_SOURCES		= tDH_Result.cc
tDH_Result_LDADD		= ../src/libamcbase.la
tDH_Result_DEPENDENCIES		= ../src/libamcbase.la $(LOFAR_DEPEND)

TESTS				= tSkyCoord.sh \
				  tEarthCoord.sh \
				  tTimeCoord.sh \
				  tConverterCommand.sh \
				  tConverterStatus.sh \
				  tBlobIO.sh \
				  tDH_Request.sh \
				  tDH_Result.sh

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  LOFARROOT=$(LOFARROOT)

EXTRA_DIST			= $(TESTS) \
				  tSkyCoord.stdout \
				  tEarthCoord.stdout \
				  tConverterCommand.stdout \
				  tConverterStatus.stdout

include $(top_srcdir)/Makefile.common
