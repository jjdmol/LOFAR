dmi_dir = $(lofar_sharedir)/../DMI

lib_LTLIBRARIES = libmeqserver.la

DOCHDRS = MeqServer.h VisDataMux.h VisHandlerNode.h Sink.h Spigot.h

libmeqserver_la_SOURCES = \
  AID-MeqServer-Registry.cc \
  MeqServer.cc \
  VisDataMux.cc \
  VisHandlerNode.cc \
  Sink.cc Spigot.cc
  
bin_PROGRAMS   		= meqserver
  
meqserver_SOURCES		    = meqserver.cc
meqserver_LDADD		      = libmeqserver.la 
# -lglish -lsos -lnpd
meqserver_DEPENDENCIES	= libmeqserver.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish/meq
dist_glish_SCRIPTS = meqserver.g

include $(lofar_sharedir)/Makefile.common
include $(lofar_sharedir)/Makefile.aids

