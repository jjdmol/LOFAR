SUBDIRS	= . ServiceBroker

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
  Socket/GTM_Socket.h \
  Socket/GTM_TCPSocket.h \
  Socket/GTM_TCPServerSocket.h \
  Socket/GTM_SocketHandler.h \
  Socket/GTM_ETHSocket.h \
  Socket/GTM_Device.h \
  ServiceBroker/GTM_SBTCPPort.h \
  ServiceBroker/GTM_ServiceBroker.h
    
lib_LTLIBRARIES 	= libgcftm.la

libgcftm_la_SOURCES= $(DOCHDRS) \
  GCF_Fsm.cc \
  GCF_Protocols.cc \
  GCF_Task.cc \
  GCF_Event.cc \
  PortInterface/GCF_Port.cc \
  PortInterface/GCF_RawPort.cc \
  Timer/GTM_Timer.cc \
  Timer/GTM_TimerHandler.cc \
  Socket/GCF_TCPPort.cc \
  Socket/GTM_Socket.cc \
  Socket/GTM_TCPSocket.cc \
  Socket/GTM_TCPServerSocket.cc \
  Socket/GTM_SocketHandler.cc \
  Socket/GTM_ETHSocket.cc \
  Socket/GCF_ETHRawPort.cc \
  Socket/GCF_DevicePort.cc \
  Socket/GTM_Device.cc \
  ServiceBroker/GTM_SBTCPPort.cc \
  ServiceBroker/GTM_ServiceBroker.cc \
	SB_Protocol.cc

libgcftm_la_CPPFLAGS=-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/ServiceBroker

BUILT_SOURCES = \
	SB_Protocol.ph

EXTRA_DIST = \
	SB_Protocol.ph

include $(lofar_sharedir)/Makefile.common

