if HAVE_CORBA
INCLUDES 	= \
	-I$(basesim_top_srcdir)/Corba \
	-I$(basesim_top_builddir)/src/Corba
endif

bin_PROGRAMS	= P2Perf

DOCHDRS 	=				\
DH_GrowSize.h					\
DH_IntArray.h					\
P2Perf.h					\
StopWatch.h					\
WH_GrowSize.h					\
WH_Square.h

P2Perf_SOURCES  	= $(DOCHDRS)		\
DH_GrowSize.cc					\
DH_IntArray.cc					\
P2Perf.cc					\
Simulate.cc					\
StopWatch.cc					\
WH_GrowSize.cc					\
WH_Square.cc					

if HAVE_CORBA
P2Perf_LDADD = $(basesim_top_builddir)/src/Corba/libth_corba.la
endif


include $(lofar_sharedir)/Makefile.common
