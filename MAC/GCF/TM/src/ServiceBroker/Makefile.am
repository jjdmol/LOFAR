DOCHDRS						= GSB_Controller.h \
							  GSB_Defines.h \
							  EventPort.h

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/autogen $<

lib_LTLIBRARIES 			= libeventport.la

libeventport_la_SOURCES		= GSB_Defines.h SB_Protocol.ph EventPort.h \
							  SB_Protocol.cc EventPort.cc 

bin_PROGRAMS		   		= ServiceBroker
  
ServiceBroker_SOURCES		= GSB_Controller.cc GSB_Main.cc
ServiceBroker_LDADD			= $(top_builddir)/src/libgcftm.la $(LOFAR_DEPEND)
ServiceBroker_DEPENDENCIES	= $(top_builddir)/src/libgcftm.la $(LOFAR_DEPEND)
	
ServiceBroker_CXXFLAGS		= -fmessage-length=0

BUILT_SOURCES 				= SB_Protocol.ph

EXTRA_DIST 					= $(sysconf_DATA)

sysconf_DATA				= ServiceBroker.conf

%.conf: ServiceBroker.conf.in
	cp $< $@
	
include $(top_srcdir)/Makefile.common

