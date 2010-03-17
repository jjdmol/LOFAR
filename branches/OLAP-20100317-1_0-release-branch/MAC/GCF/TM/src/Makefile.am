SUBDIRS	= . 

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
  ServiceBroker/ServiceBrokerTask.h \
  $(BUILT_SOURCES)
    
lib_LTLIBRARIES 	= libgcftm.la

libgcftm_la_SOURCES= Package__Version.cc  $(DOCHDRS) \
  GCF_Handler.cc \
  GCF_Scheduler.cc \
  GCF_Fsm.cc \
  GCF_Protocols.cc \
  Port/GCF_PortInterface.cc \
  Port/GCF_Port.cc \
  Port/GCF_RawPort.cc \
  Timer/GTM_Timer.cc \
  Timer/GTM_TimerHandler.cc \
  PortImpl/GCF_DevicePort.cc \
  PortImpl/GCF_ETHRawPort.cc \
  PortImpl/GCF_ITCPort.cc \
  PortImpl/GCF_TCPPort.cc \
  PortImpl/GTM_Device.cc \
  PortImpl/GTM_ETHSocket.cc \
  PortImpl/GTM_File.cc \
  PortImpl/GTM_FileHandler.cc \
  PortImpl/GTM_TCPServerSocket.cc \
  PortImpl/GTM_TCPSocket.cc \
  ServiceBroker/GTM_SBTCPPort.cc \
  ServiceBroker/ServiceBrokerTask.cc 

libgcftm_la_CPPFLAGS=-I$(top_srcdir)/src/ServiceBroker 

libgcftm_la_CXXFLAGS=-fmessage-length=0

BUILT_SOURCES 		= 

EXTRA_DIST 			= $(sysconf_DATA)
	
sysconf_DATA 		= mac.log_prop

bin_PROGRAMS = versiongcftm

versiongcftm_SOURCES      = versiongcftm.cc
versiongcftm_LDADD        = libgcftm.la
versiongcftm_DEPENDENCIES = libgcftm.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

