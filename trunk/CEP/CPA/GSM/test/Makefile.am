
TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) srcdir=$(srcdir)

TESTS = test_gsm.sh

EXTRA_DIST = test_gsm.sh test_gsm.run test_gsm.g test_gsm.out

include $(lofar_sharedir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
