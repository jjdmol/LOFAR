common_top_builddir=@common_top_builddir@

check_PROGRAMS = tBBS2

tBBS2_SOURCES = tBBS2.cc

LDADD				= ../src/libpss3.la 
DEPENDENCIES			= ../src/libpss3.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

#TESTS = tBBS2_test.sh	


include $(lofar_sharedir)/Makefile.common
