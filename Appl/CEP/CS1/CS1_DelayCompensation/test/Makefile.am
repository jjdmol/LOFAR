check_PROGRAMS		= tWH_DelayCompensation

# programs to run through supplied checktools
CHECKTOOLPROGS		= tWH_DelayCompensation
#ENDCHECKTOOLPROGS

# scripts and possible programs to run tests
TESTS			= tWH_DelayCompensation.sh

XFAIL_TESTS		=

tWH_DelayCompensation_SOURCES	= tWH_DelayCompensation.cc
tWH_DelayCompensation_LDADD	= ../src/libcs1_delaycompensation.la
tWH_DelayCompensation_DEPENDENCIES = ../src/libcs1_delaycompensation.la $(LOFAR_DEPEND)

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST		= $(TESTS)

TESTS_ENVIRONMENT	= lofar_sharedir=$(lofar_sharedir) \
			  srcdir=$(srcdir) \
			  LOFARROOT=$(LOFARROOT) \
			  LOFARDATAROOT=$(LOFARDATAROOT) \
			  MPIBIN=$(MPIBIN)

include $(top_srcdir)/Makefile.common
