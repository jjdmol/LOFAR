
check_PROGRAMS		= tKeyPrefix tParameterSet 

tKeyPrefix_SOURCES	= tKeyPrefix.cc
tParameterSet_SOURCES	= tParameterSet.cc

LDADD			= ../src/libaps.la
DEPENDENCIES		= ../src/libaps.la $(LOFAR_DEPEND) 

TESTSCRIPTS 		= tKeyPrefix.sh \
			  tParameterSet_test.sh

TESTS			= $(TESTSCRIPTS)


EXTRA_DIST		= tParameterSet.in_merge \
			  tParameterSet.in_param \
			  tParameterSet.log_prop \
			  tParameterSet.stdout \
			  $(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common

