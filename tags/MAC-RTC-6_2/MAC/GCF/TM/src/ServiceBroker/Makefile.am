DOCHDRS						= GSB_Controller.h \
							  GSB_Defines.h

bin_PROGRAMS		   		= ServiceBroker
  
ServiceBroker_SOURCES		= GSB_Controller.cc GSB_Main.cc
ServiceBroker_LDADD			= $(top_builddir)/src/libgcftm.la $(LOFAR_DEPEND)
ServiceBroker_DEPENDENCIES	= $(top_builddir)/src/libgcftm.la $(LOFAR_DEPEND)
	
ServiceBroker_CXXFLAGS		= -fmessage-length=0

BUILT_SOURCES 				= 

EXTRA_DIST 					= $(sysconf_DATA)

sysconf_DATA				= ServiceBroker.conf

%.conf: ServiceBroker.conf.in
	cp $< $@
	
include $(top_srcdir)/Makefile.common

