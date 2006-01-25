check_PROGRAMS	      =	testLogger \
			tDataConvert tStringUtil tHexdump \
			testSocket tComplex \
			tRunOnNode tTimer tNumeric

# programs to run through supplied checktools
CHECKTOOLPROGS 	      =	testLogger tBlobString \
			tBlobStream tBlobArray tBlobField \
			tDataConvert tKeyValueMap tStringUtil tHexdump \
			tTimer tNumeric
#ENDCHECKTOOLPROGS

TESTSCRIPTS = \
	  tComplex.sh \
	  tDataConvert_test.sh \
	  testLogger_test.sh \
	  tHexdump_test.sh    \
	  tNumeric.sh \
	  tRunOnNode.sh \
	  tStringUtil_test.sh

TESTS	= $(TESTSCRIPTS) \
	  tTimer

EXTRA_DIST = $(TESTSCRIPTS) \
	  tComplex.stdout \
	  tDataConvert.stdout \
	  testLogger.log_prop testLogger.debug \
	  tHexdump.log_prop \
	  tRunOnNode.log_prop tRunOnNode.stdout \
	  tStringUtil.stdout 

testSocket_SOURCES		= testSocket.cc
testSocket_LDADD		= ../src/libcommon.la	
testSocket_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

testLogger_SOURCES		= testLogger.cc
testLogger_LDADD		= ../src/libcommon.la	
testLogger_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tDataConvert_SOURCES		= tDataConvert.cc
tDataConvert_LDADD		= ../src/libcommon.la
tDataConvert_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tStringUtil_SOURCES		= tStringUtil.cc
tStringUtil_LDADD		= ../src/libcommon.la	
tStringUtil_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tHexdump_SOURCES		= tHexdump.cc
tHexdump_LDADD			= ../src/libcommon.la	
tHexdump_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

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
