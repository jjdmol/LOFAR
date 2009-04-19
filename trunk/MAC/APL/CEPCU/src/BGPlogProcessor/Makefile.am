
bin_PROGRAMS			= BGPlogProcessor

sysconf_DATA			= BGPlogProcessor.conf

BGPlogProcessorHEADERS	= BGPlogProcessor.h

noinst_HEADERS 			= $(BGPlogProcessorHEADERS)

BGPlogProcessor_SOURCES			= BGPlogProcessor.cc \
								  BGPlogProcessorMain.cc
BGPlogProcessor_DEPENDENCIES	= $(LOFAR_DEPEND) $(BGPlogProcessorHEADERS)
BGPlogProcessor_LDADD       	=

include $(top_srcdir)/Makefile.common
