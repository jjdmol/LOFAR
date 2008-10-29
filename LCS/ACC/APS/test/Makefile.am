
check_PROGRAMS		= tKeyPrefix tParameterValue \
			  tParameterSet tParameterSetPerf

tKeyPrefix_SOURCES	= tKeyPrefix.cc
tParameterValue_SOURCES	= tParameterValue.cc
tParameterSet_SOURCES	= tParameterSet.cc
tParameterSetPerf_SOURCES= tParameterSetPerf.cc

LDADD			= ../src/libaps.la
DEPENDENCIES		= ../src/libaps.la $(LOFAR_DEPEND) 

TESTSCRIPTS 		= tKeyPrefix.sh \
			  tParameterValue.sh \
			  tParameterSet_test.sh \
			  tParameterSetPerf.sh \
			  tgetparsetvalue.sh

TESTS			= $(TESTSCRIPTS)


EXTRA_DIST		= tParameterSet.in_merge_nocase \
			  tParameterSet.in_merge_normal \
			  tParameterSet.in_param_nocase \
			  tParameterSet.in_param_normal \
			  tParameterSet.log_prop \
			  tParameterSet.stdout \
			  tParameterSetPerf.run \
			  tgetparsetvalue.run \
			  tgetparsetvalue.parset \
			  tgetparsetvalue.stdout \
			  $(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common

