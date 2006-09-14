check_PROGRAMS			= tCoord3D \
				  tDirection \
				  tPosition \
				  tEpoch \
				  tConverterCommand \
				  tConverterStatus \
				  tBlobIO \
				  tDH_Request \
				  tDH_Result

CHECKTOOLPROGS			= tCoord3D \
				  tDirection \
				  tPosition \
				  tEpoch \
				  tConverterCommand \
				  tConverterStatus \
				  tBlobIO \
				  tDH_Request \
				  tDH_Result
#ENDCHECKTOOLPROGS

tCoord3D_SOURCES		= tCoord3D.cc
tCoord3D_LDADD			= ../src/libamcbase.la
tCoord3D_DEPENDENCIES		= ../src/libamcbase.la $(LOFAR_DEPEND)

tDirection_SOURCES		= tDirection.cc
tDirection_LDADD		= ../src/libamcbase.la
tDirection_DEPENDENCIES		= ../src/libamcbase.la $(LOFAR_DEPEND)

tPosition_SOURCES		= tPosition.cc
tPosition_LDADD			= ../src/libamcbase.la
tPosition_DEPENDENCIES		= ../src/libamcbase.la $(LOFAR_DEPEND)

tEpoch_SOURCES			= tEpoch.cc
tEpoch_LDADD			= ../src/libamcbase.la
tEpoch_DEPENDENCIES		= ../src/libamcbase.la $(LOFAR_DEPEND)

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

TESTS				= tCoord3D.sh \
				  tDirection.sh \
				  tPosition.sh \
				  tEpoch.sh \
				  tConverterCommand.sh \
				  tConverterStatus.sh \
				  tBlobIO.sh \
				  tDH_Request.sh \
				  tDH_Result.sh


EXTRA_DIST			= $(TESTS) \
				  tDirection.stdout \
				  tPosition.stdout \
				  tConverterCommand.stdout \
				  tConverterStatus.stdout

include $(top_srcdir)/Makefile.common
