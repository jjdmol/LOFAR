
bin_PROGRAMS			= BGPlogProcessor CEPlogProcessor

sysconf_DATA			= BGPlogProcessor.conf

BGPlogProcessorHEADERS	= BGPlogProcessor.h

CEPlogProcessorHEADERS	= CEPlogProcessor.h

noinst_HEADERS 			= $(BGPlogProcessorHEADERS) $(CEPlogProcessorHEADERS)

BGPlogProcessor_SOURCES			= BGPlogProcessor.cc \
								  BGPlogProcessorMain.cc
BGPlogProcessor_DEPENDENCIES	= $(LOFAR_DEPEND) $(BGPlogProcessorHEADERS)
BGPlogProcessor_LDADD       	=

CEPlogProcessor_SOURCES			= CEPlogProcessor.cc \
								  CEPlogProcessorMain.cc
CEPlogProcessor_DEPENDENCIES	= $(LOFAR_DEPEND) $(CEPlogProcessorHEADERS)
CEPlogProcessor_LDADD       	=

include $(top_srcdir)/Makefile.common
