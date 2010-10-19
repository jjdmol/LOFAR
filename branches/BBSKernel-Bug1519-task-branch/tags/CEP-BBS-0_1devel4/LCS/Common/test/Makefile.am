# programs to run through supplied checktools
CHECKTOOLPROGS	=	testLogger tAllocator tDataConvert \
			tFileLocator tStringUtil tStreamUtil \
			tHexdump testSocket tComplex tProcess \
			tRunOnNode tTimer tNumeric tSingleton \
			tObjectFactory tSystemUtil tSaxpy

check_PROGRAMS	=	$(CHECKTOOLPROGS)

TESTSCRIPTS	= 	testLogger.sh tAllocator.sh tDataConvert.sh \
			tFileLocator.sh tStringUtil.sh tStreamUtil.sh \
			tHexdump.sh testSocket.sh tComplex.sh tProcess.sh \
			tRunOnNode.sh tTimer.sh tNumeric.sh tSingleton.sh \
			tObjectFactory.sh

TESTS		=	$(TESTSCRIPTS)

EXTRA_DIST	=	$(TESTSCRIPTS) \
			testLogger.log_prop testLogger.debug \
			testLogger.in_log4cxx.log_prop \
			tDataConvert.stdout \
			tStringUtil.stdout \
			tStreamUtil.stdout \
			tHexdump.log_prop \
			testSocket.run \
			tComplex.stdout \
			tProcess.log_prop tProcess.run tProcess.stdout \
			tRunOnNode.log_prop tRunOnNode.stdout \
			tNumeric.run tNumeric.stdout \
			tObjectFactory.stdout

testLogger_SOURCES		= testLogger.cc
testLogger_LDADD		= ../src/libcommon.la	
testLogger_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tAllocator_SOURCES		= tAllocator.cc
tAllocator_LDADD		= ../src/libcommon.la	
tAllocator_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tDataConvert_SOURCES		= tDataConvert.cc
tDataConvert_LDADD		= ../src/libcommon.la
tDataConvert_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tFileLocator_SOURCES		= tFileLocator.cc
tFileLocator_LDADD		    = ../src/libcommon.la
tFileLocator_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tStringUtil_SOURCES		= tStringUtil.cc
tStringUtil_LDADD		= ../src/libcommon.la	
tStringUtil_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tStreamUtil_SOURCES		= tStreamUtil.cc
tStreamUtil_LDADD		= ../src/libcommon.la	
tStreamUtil_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tHexdump_SOURCES		= tHexdump.cc
tHexdump_LDADD			= ../src/libcommon.la	
tHexdump_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

testSocket_SOURCES		= testSocket.cc
testSocket_LDADD		= ../src/libcommon.la	
testSocket_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tComplex_SOURCES		= tComplex.cc
tComplex_LDADD			= ../src/libcommon.la	
tComplex_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tProcess_SOURCES		= tProcess.cc
tProcess_LDADD			= ../src/libcommon.la	
tProcess_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tRunOnNode_SOURCES		= tRunOnNode.cc
tRunOnNode_LDADD		= ../src/libcommon.la	
tRunOnNode_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tTimer_SOURCES			= tTimer.cc
tTimer_LDADD			= ../src/libcommon.la	
tTimer_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tNumeric_SOURCES		= tNumeric.cc
tNumeric_LDADD			= ../src/libcommon.la	
tNumeric_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tSingleton_SOURCES		= tSingleton.cc
tSingleton_LDADD		= ../src/libcommon.la	
tSingleton_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

tObjectFactory_SOURCES		= tObjectFactory.cc \
				  Shapes.h Shapes.cc \
				  Coordinates.h Coordinates.cc
tObjectFactory_LDADD		= ../src/libcommon.la	
tObjectFactory_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tSystemUtil_SOURCES		= tSystemUtil.cc
tSystemUtil_LDADD		= ../src/libcommon.la	
tSystemUtil_DEPENDENCIES	= ../src/libcommon.la $(LOFAR_DEPEND)

tSaxpy_SOURCES			= tSaxpy.cc
tSaxpy_LDADD			= ../src/libcommon.la	
tSaxpy_DEPENDENCIES		= ../src/libcommon.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
