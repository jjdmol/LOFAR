if HAVE_CORBA
INCLUDES 	= \
	-I$(basesim_top_srcdir)/Corba \
	-I$(basesim_top_builddir)/src/Corba
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
P2Perf_LDADD = $(basesim_top_builddir)/src/Corba/libth_corba.la
endif

