if HAVE_CORBA
INCLUDES		= \
	-I$(basesim_top_srcdir)/Corba \
	-I$(basesim_top_builddir)/src/Corba
endif

DOCHDRS		 	= 			\
DH_Ring.h					\
DH_Test.h					\
RingBuilder.h					\
WH_FromRing.h					\
WH_Ring.h					\
WH_RingOut.h					\
WH_RingSimul.h					\
WH_ToRing.h   

lib_LTLIBRARIES 	= libringsim.la

libringsim_la_SOURCES 	= $(DOCHDRS)		\
DH_Ring.cc					\
DH_Test.cc					\
RingBuilder.cc					\
WH_FromRing.cc					\
WH_Ring.cc					\
WH_RingOut.cc					\
WH_RingSimul.cc					\
WH_ToRing.cc

bin_PROGRAMS		= Simulate
Simulate_SOURCES 	= Simulate.cc

if HAVE_CORBA
Simulate_LDADD		= $(top_builddir)/src/libringsim.la \
			  $(basesim_top_builddir)/src/Corba/libth_corba.la
else
Simulate_LDADD		= $(top_builddir)/src/libringsim.la
endif

EXTRA_DIST		=

include $(lofar_sharedir)/Makefile.common
