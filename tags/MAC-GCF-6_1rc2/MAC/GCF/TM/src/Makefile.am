SUBDIRS	= . ServiceBroker

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: ServiceBroker/%.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/autogen $<

DOCHDRS = \
  GTM_Defines.h \
  Timer/GTM_Timer.h \
  Timer/GTM_TimerHandler.h \
  PortImpl/GTM_File.h \
  PortImpl/GTM_FileHandler.h \
  PortImpl/GTM_TCPSocket.h \
  PortImpl/GTM_TCPServerSocket.h \
  PortImpl/GTM_ETHSocket.h \
  PortImpl/GTM_Device.h \
  ServiceBroker/GTM_SBTCPPort.h \
  ServiceBroker/GTM_ServiceBroker.h \
  $(BUILT_SOURCES)
    
lib_LTLIBRARIES 	= libgcftm.la

libgcftm_la_SOURCES= $(DOCHDRS) \
  GCF_Fsm.cc \
  GCF_Protocols.cc \
  GCF_Task.cc \
  GCF_Event.cc \
  Port/GCF_Port.cc \
  Port/GCF_RawPort.cc \
  Timer/GTM_Timer.cc \
  Timer/GTM_TimerHandler.cc \
  PortImpl/GCF_TCPPort.cc \
  PortImpl/GTM_File.cc \
  PortImpl/GTM_TCPSocket.cc \
  PortImpl/GTM_TCPServerSocket.cc \
  PortImpl/GTM_FileHandler.cc \
  PortImpl/GTM_ETHSocket.cc \
  PortImpl/GCF_ETHRawPort.cc \
  PortImpl/GCF_DevicePort.cc \
  PortImpl/GTM_Device.cc \
  ServiceBroker/GTM_SBTCPPort.cc \
  ServiceBroker/GTM_ServiceBroker.cc \
	SB_Protocol.cc

libgcftm_la_CPPFLAGS=-I$(top_srcdir)/src/ServiceBroker

libgcftm_la_CXXFLAGS=-fmessage-length=0

BUILT_SOURCES = SB_Protocol.ph

EXTRA_DIST = $(sysconf_DATA)
	
sysconf_DATA = \
	mac.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

clean-local:
	rm -f *.ph
	
include $(top_srcdir)/Makefile.common

