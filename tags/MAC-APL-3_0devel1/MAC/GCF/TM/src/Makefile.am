SUBDIRS	= . ServiceBroker

INSTALL=/usr/bin/install -C
AUTOGEN = autogen
SUFFIXES = .ph
%.ph: ServiceBroker/%.prot
	$(AUTOGEN) -L $(top_srcdir)/../../MACCommon/autogen $< && \
	chmod a+w $@

pkgincludedir = $(includedir)/GCF/TM
pkginclude_HEADERS = \
	../include/GCF/TM/GCF_ETHRawPort.h \
	../include/GCF/TM/GCF_TCPPort.h \
	../include/GCF/TM/GCF_RawPort.h \
	../include/GCF/TM/GCF_Port.h \
	../include/GCF/TM/GCF_PortInterface.h \
	../include/GCF/TM/GCF_Event.h \
	../include/GCF/TM/GCF_Protocols.h \
	../include/GCF/TM/GCF_Task.h \
	../include/GCF/TM/GCF_Handler.h \
	../include/GCF/TM/GCF_Fsm.h \
	../include/GCF/TM/GCF_Control.h \
	../include/GCF/TM/GCF_DevicePort.h
 
DOCHDRS = $(pkginclude_HEADERS) \
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
  ServiceBroker/GTM_ServiceBroker.h
    
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

libgcftm_la_CPPFLAGS=-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/ServiceBroker

libgcftm_la_CXXFLAGS=-fmessage-length=0

BUILT_SOURCES = \
	SB_Protocol.ph

EXTRA_DIST = \
	SB_Protocol.ph
	
include $(lofar_sharedir)/Makefile.common

