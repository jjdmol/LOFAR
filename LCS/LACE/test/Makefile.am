check_PROGRAMS        = tAddress tSAP tConnector tAcceptor ManualSocketTest

# programs to run through supplied checktools
CHECKTOOLPROGS        = $(check_PROGRAMS)

# scripts used to run tests
TESTSCRIPTS           = 

# scripts and possible programs to run tests
TESTS	              = $(TESTSCRIPTS)

XFAIL_TESTS           =

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST            = $(TESTSCRIPTS)

tAddress_SOURCES		= tAddress.cc
tAddress_LDADD			= ../src/liblace.la	
tAddress_DEPENDENCIES	= ../src/liblace.la $(LOFAR_DEPEND)

tSAP_SOURCES			= tSAP.cc
tSAP_LDADD				= ../src/liblace.la	
tSAP_DEPENDENCIES		= ../src/liblace.la $(LOFAR_DEPEND)

tConnector_SOURCES		= tConnector.cc
tConnector_LDADD		= ../src/liblace.la	
tConnector_DEPENDENCIES	= ../src/liblace.la $(LOFAR_DEPEND)

tAcceptor_SOURCES		= tAcceptor.cc
tAcceptor_LDADD			= ../src/liblace.la	
tAcceptor_DEPENDENCIES	= ../src/liblace.la $(LOFAR_DEPEND)

ManualSocketTest_SOURCES		= ManualSocketTest.cc
ManualSocketTest_LDADD			= ../src/liblace.la	
ManualSocketTest_DEPENDENCIES	= ../src/liblace.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
