common_top_builddir=@common_top_builddir@

check_PROGRAMS = tPredSolv

tPredSolv_SOURCES = tPredSolv.cc


LDADD				= ../src/libbbs3.la 
DEPENDENCIES			= ../src/libbbs3.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= top_srcdir=$(top_srcdir) \
				  lofar_sharedir=$(lofar_sharedir) \
				  common_top_builddir=$(common_top_builddir)

TESTS = tPredSolv.sh


include $(lofar_sharedir)/Makefile.common
