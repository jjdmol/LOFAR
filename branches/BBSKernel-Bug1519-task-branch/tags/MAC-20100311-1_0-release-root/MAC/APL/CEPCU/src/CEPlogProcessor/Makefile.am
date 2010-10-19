
bin_PROGRAMS			= CEPlogProcessor

sysconf_DATA			= CEPlogProcessor.conf

CEPlogProcessorHEADERS	= CEPlogProcessor.h

noinst_HEADERS 			= $(CEPlogProcessorHEADERS)

CEPlogProcessor_SOURCES			= CEPlogProcessor.cc \
								  CEPlogProcessorMain.cc
CEPlogProcessor_DEPENDENCIES	= $(LOFAR_DEPEND) $(CEPlogProcessorHEADERS)
CEPlogProcessor_LDADD       	=

include $(top_srcdir)/Makefile.common
