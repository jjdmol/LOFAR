check_PROGRAMS	= tLCSMath tIComplex tLorrays

TESTSCRIPTS	= tLCSMath_test.sh \
	  	  tIComplex_test.sh \
		  tLorrays_test.sh

TESTS		= $(TESTSCRIPTS)

XFAIL_TESTS	= tLorrays_test.sh

EXTRA_DIST	= $(TESTSCRIPTS)

tLCSMath_SOURCES		= tLCSMath.cc
tLCSMath_LDADD			= ../src/libmath.la
tLCSMath_DEPENDENCIES		= ../src/libmath.la $(LOFAR_DEPEND)

tIComplex_SOURCES		= tIComplex.cc
tIComplex_LDADD			= ../src/libmath.la
tIComplex_DEPENDENCIES		= ../src/libmath.la $(LOFAR_DEPEND)

tLorrays_SOURCES		= tLorrays.cc
tLorrays_LDADD			= ../src/libmath.la
tLorrays_DEPENDENCIES		= ../src/libmath.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir)


include $(top_srcdir)/Makefile.common
