check_PROGRAMS	      =	tDebug testLogger tBlobString \
			tBlobStream tBlobArray tBlobField \
			tDataConvert tKeyValueMap tStringUtil tHexdump \
			tLorrays testSocket tComplex \
			tRunOnNode tTimer tNumeric

# programs to run through supplied checktools
CHECKTOOLPROGS 	      =	tDebug testLogger tBlobString \
			tBlobStream tBlobArray tBlobField \
			tDataConvert tKeyValueMap tStringUtil tHexdump \
			tLorrays tTimer tNumeric
#ENDCHECKTOOLPROGS

TESTSCRIPTS = \
	  tBlobArray_test.sh \
	  tBlobField_test.sh \
          tBlobStream_test.sh \
          tBlobString_test.sh \
	  tComplex.sh \
	  tDataConvert_test.sh \
	  tDebug_test.sh \
	  testLogger_test.sh \
	  tHexdump_test.sh    \
	  tKeyValueMap_test.sh \
	  tLorrays_test.sh    \
	  tNumeric.sh \
	  tComplex.sh \
	  tRunOnNode.sh \
	  tStringUtil_test.sh

TESTS	= $(TESTSCRIPTS) \
	  tTimer

EXTRA_DIST = $(TESTSCRIPTS) \
	  tBlobField.in_be tBlobField.in_le \
          tBlobStream.in_be tBlobStream.in_le tBlobStream.stdout \
	  tComplex.stdout \
	  tDataConvert.stdout \
	  tDebug.stdout \
	  testLogger.log_prop testLogger.debug \
	  tHexdump.log_prop \
	  tKeyValueMap.in tKeyValueMap.stdout \
	  tLorrays.stdout \
	  tRunOnNode.log_prop tRunOnNode.stdout \
	  tStringUtil.stdout 

XFAIL_TESTS		= tLorrays_test.sh

tDebug_SOURCES			= tDebug.cc
tDebug_LDADD			= ../src/libcommon.la
tDebug_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

testSocket_SOURCES		= testSocket.cc
testSocket_LDADD		= ../src/libcommon.la	
testSocket_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

testLogger_SOURCES		= testLogger.cc
testLogger_LDADD		= ../src/libcommon.la	
testLogger_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tBlobString_SOURCES		= tBlobString.cc
tBlobString_LDADD		= ../src/libcommon.la
tBlobString_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tBlobStream_SOURCES		= tBlobStream.cc
tBlobStream_LDADD		= ../src/libcommon.la
tBlobStream_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tBlobArray_SOURCES		= tBlobArray.cc
tBlobArray_LDADD		= ../src/libcommon.la
tBlobArray_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tBlobField_SOURCES		= tBlobField.cc
tBlobField_LDADD		= ../src/libcommon.la
tBlobField_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tDataConvert_SOURCES		= tDataConvert.cc
tDataConvert_LDADD		= ../src/libcommon.la
tDataConvert_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tKeyValueMap_SOURCES		= tKeyValueMap.cc
tKeyValueMap_LDADD		= ../src/libcommon.la
tKeyValueMap_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tStringUtil_SOURCES		= tStringUtil.cc
tStringUtil_LDADD		= ../src/libcommon.la	
tStringUtil_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tHexdump_SOURCES		= tHexdump.cc
tHexdump_LDADD			= ../src/libcommon.la	
tHexdump_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tLorrays_SOURCES		= tLorrays.cc
tLorrays_LDADD			= ../src/libcommon.la	
tLorrays_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tRunOnNode_SOURCES		= tRunOnNode.cc
tRunOnNode_LDADD		= ../src/libcommon.la	
tRunOnNode_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tTimer_SOURCES			= tTimer.cc
tTimer_LDADD			= ../src/libcommon.la	
tTimer_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tComplex_SOURCES		= tComplex.cc
tComplex_LDADD			= ../src/libcommon.la	
tComplex_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tNumeric_SOURCES		= tNumeric.cc
tNumeric_LDADD			= ../src/libcommon.la	
tNumeric_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)


TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  LOFARROOT=$(LOFARROOT)


include $(top_srcdir)/Makefile.common
