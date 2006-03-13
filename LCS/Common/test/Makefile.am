check_PROGRAMS	      =	testLogger tException \
			tAllocator tDataConvert tStringUtil tHexdump \
			testSocket tComplex tProcess \
			tRunOnNode tTimer tNumeric

# programs to run through supplied checktools
CHECKTOOLPROGS 	      =	testLogger \
			tAllocator tDataConvert tStringUtil tHexdump \
			testSocket tComplex tProcess \
			tRunOnNode tTimer tNumeric
#ENDCHECKTOOLPROGS

TESTSCRIPTS = \
	tException.sh \
	tComplex.sh \
	tAllocator.sh \
	tDataConvert_test.sh \
	testLogger_test.sh \
	tHexdump_test.sh    \
	tNumeric.sh \
	tProcess.sh \
	tRunOnNode.sh \
	tTimer.sh \
	tStringUtil_test.sh

TESTS = $(TESTSCRIPTS)

EXTRA_DIST = $(TESTSCRIPTS) \
	tComplex.stdout \
	tDataConvert.stdout \
	testLogger.log_prop testLogger.debug \
	tException.log_prop tException.stdout \
	tHexdump.log_prop \
	tProcess.log_prop tProcess.stdout \
	tRunOnNode.log_prop tRunOnNode.stdout \
	tStringUtil.stdout 

tException_SOURCES		= tException.cc
tException_LDADD		= ../src/libcommon.la	
tException_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

testSocket_SOURCES		= testSocket.cc
testSocket_LDADD		= ../src/libcommon.la	
testSocket_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

testLogger_SOURCES		= testLogger.cc
testLogger_LDADD		= ../src/libcommon.la	
testLogger_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tAllocator_SOURCES		= tAllocator.cc
tAllocator_LDADD		= ../src/libcommon.la	
tAllocator_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tDataConvert_SOURCES		= tDataConvert.cc
tDataConvert_LDADD		= ../src/libcommon.la
tDataConvert_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tStringUtil_SOURCES		= tStringUtil.cc
tStringUtil_LDADD		= ../src/libcommon.la	
tStringUtil_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tHexdump_SOURCES		= tHexdump.cc
tHexdump_LDADD			= ../src/libcommon.la	
tHexdump_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tProcess_SOURCES		= tProcess.cc
tProcess_LDADD			= ../src/libcommon.la	
tProcess_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

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
