
check_PROGRAMS = tBGL_Processing transpose


AM_CPPFLAGS = $(EXTRA_CPPFLAGS)

AM_CXXFLAGS = \
	-I../../../src

tBGL_Processing_SOURCES =			\
tBGL_Processing.cc				\
../src/Allocator.cc				\
../src/BGL_Processing.cc			\
../src/Correlator.cc				\
../src/FIR.cc					\
../src/PPF.cc

transpose_SOURCES =				\
transpose.cc					

CCAS			= $(CC)
CCASFLAGS		= $(patsubst -q%,,$(CPPFLAGS)) $(EXTRA_CPPFLAGS)


TESTS   = 					\
tBGL_Processing.sh


include $(top_srcdir)/Makefile.common
