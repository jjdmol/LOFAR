check_PROGRAMS			= tLCSMath tIComplex

TESTS	= tLCSMath_test.sh \
	  tIComplex_test.sh

tLCSMath_SOURCES		= tLCSMath.cc
tLCSMath_LDADD			= ../src/libmath.la
tLCSMath_DEPENDENCIES		= ../src/libmath.la $(LOFAR_DEPEND)

tIComplex_SOURCES		= tIComplex.cc
tIComplex_LDADD			= ../src/libmath.la
tIComplex_DEPENDENCIES		= ../src/libmath.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir)


include $(lofar_sharedir)/Makefile.common
