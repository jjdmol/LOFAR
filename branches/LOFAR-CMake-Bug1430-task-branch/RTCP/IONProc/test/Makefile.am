
check_PROGRAMS = tDelayCompensation


AM_CPPFLAGS = $(EXTRA_CPPFLAGS)

AM_CXXFLAGS = \
	-I../../../src

tDelayCompensation_SOURCES =			\
tDelayCompensation.cc				\
../src/WH_DelayCompensation.cc

EXTRA_DIST =					\
tDelayCompensation.parset

CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) $(EXTRA_CPPFLAGS)


TESTS   = 					\
tDelayCompensation.sh


include $(top_srcdir)/Makefile.common
