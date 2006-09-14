
check_PROGRAMS			= tParameterSet 

tParameterSet_SOURCES	= tParameterSet.cc

LDADD					= ../src/libaps.la
DEPENDENCIES			= ../src/libaps.la $(LOFAR_DEPEND) 

TESTSCRIPTS 			= \
						  tParameterSet_test.sh

TESTS					= $(TESTSCRIPTS)


EXTRA_DIST				= \
						  tParameterSet.in_merge \
						  tParameterSet.in_param \
						  tParameterSet.log_prop \
						  tParameterSet.stdout \
						  $(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common

