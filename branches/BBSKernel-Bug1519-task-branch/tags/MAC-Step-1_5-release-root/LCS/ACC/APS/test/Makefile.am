
check_PROGRAMS		= tKeyPrefix tParameterSet 

tKeyPrefix_SOURCES	= tKeyPrefix.cc
tParameterSet_SOURCES	= tParameterSet.cc

LDADD			= ../src/libaps.la
DEPENDENCIES		= ../src/libaps.la $(LOFAR_DEPEND) 

TESTSCRIPTS 		= tKeyPrefix.sh \
			  tParameterSet_test.sh \
			  tgetparsetvalue.sh

TESTS			= $(TESTSCRIPTS)


EXTRA_DIST		= tParameterSet.in_merge_nocase \
			  tParameterSet.in_merge_normal \
			  tParameterSet.in_param_nocase \
			  tParameterSet.in_param_normal \
			  tParameterSet.log_prop \
			  tParameterSet.stdout \
			  tgetparsetvalue.run \
			  tgetparsetvalue.parset \
			  tgetparsetvalue.stdout \
			  $(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common

