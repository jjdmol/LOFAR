if HAVE_CORBA
INCLUDES 	= \
	-I$(top_srcdir)/../../../../BaseSim/src \
	-I$(top_srcdir)/../../../../BaseSim/src/Corba \
	-I$(top_builddir)/BaseSim/src/Corba
else
INCLUDES	= -I$(top_srcdir)/../../../../BaseSim/src
endif

bin_PROGRAMS	= P2Perf
P2Perf_SOURCES = \
DH_GrowSize.cc					\
DH_GrowSize.h					\
DH_IntArray.cc					\
DH_IntArray.h					\
P2Perf.cc					\
P2Perf.h					\
Simulate.cc					\
StopWatch.cc					\
StopWatch.h					\
WH_GrowSize.cc					\
WH_GrowSize.h					\
WH_Square.cc					\
WH_Square.h

if HAVE_CORBA
P2Perf_LDADD = \
	$(top_builddir)/../../../../../BaseSim/build/src/libbasesim.la \
	$(top_builddir)/../../../../../BaseSim/build/src/Corba/libth_corba.la
else
P2Perf_LDADD = \
	$(top_builddir)/../../../../../BaseSim/build/src/libbasesim.la
endif

