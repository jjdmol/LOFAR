#
# automake specification for TH_Corba transport.
#

bin_PROGRAMS	        = CorbaControlClient CorbaMonitorClient

CorbaControlClient_SOURCES  	= CorbaControlClient.cc
CorbaMonitorClient_SOURCES  	= CorbaMonitorClient.cc

CorbaControlClient_LDADD = \
      $(top_builddir)/src/Corba/libth_corba.la \
      $(top_builddir)/src/libbasesim.la
CorbaMonitorClient_LDADD = \
      $(top_builddir)/src/Corba/libth_corba.la \
      $(top_builddir)/src/libbasesim.la

include $(lofar_sharedir)/Makefile.common

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
