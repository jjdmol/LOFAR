if HAVE_CORBA
INCLUDES 	= \
	-I$(top_srcdir)/BaseSim \
	-I$(top_srcdir)/BaseSim/Corba \
	-I$(top_builddir)/BaseSim/Corba
else
INCLUDES	= -I$(top_srcdir)/BaseSim
endif

bin_PROGRAMS	= SeqTest
SeqTest_SOURCES = \
DH_GrowSize.cc					\
DH_GrowSize.h					\
DH_IntArray.cc					\
DH_IntArray.h					\
SeqSim.cc					\
SeqSim.h					\
Simulate.cc					\
StopWatch.cc					\
StopWatch.h					\
WH_GrowSize.cc					\
WH_GrowSize.h					\
WH_Square.cc					\
WH_Square.h

if HAVE_CORBA
SeqTest_LDADD = \
	$(top_builddir)/BaseSim/libbasesim.la \
	$(top_builddir)/BaseSim/Corba/libth_corba.la
else
SeqTest_LDADD = \
	$(top_builddir)/BaseSim/libbasesim.la
endif

