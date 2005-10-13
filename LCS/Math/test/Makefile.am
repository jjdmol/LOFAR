check_PROGRAMS	= tLCSMath tIComplex tComplexFunc

TESTSCRIPTS	= tLCSMath_test.sh \
	  	  tIComplex_test.sh \
	  	  tComplexFunc_test.sh

TESTS = $(TESTSCRIPTS)

EXTRA_DIST = $(TESTSCRIPTS)

tLCSMath_SOURCES		= tLCSMath.cc
tLCSMath_LDADD			= ../src/libmath.la
tLCSMath_DEPENDENCIES		= ../src/libmath.la $(LOFAR_DEPEND)

tIComplex_SOURCES		= tIComplex.cc
tIComplex_LDADD			= ../src/libmath.la
tIComplex_DEPENDENCIES		= ../src/libmath.la $(LOFAR_DEPEND)

tComplexFunc_SOURCES		= tComplexFunc.cc
tComplexFunc_LDADD		= ../src/libmath.la
tComplexFunc_DEPENDENCIES	= ../src/libmath.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir)


include $(top_srcdir)/Makefile.common
