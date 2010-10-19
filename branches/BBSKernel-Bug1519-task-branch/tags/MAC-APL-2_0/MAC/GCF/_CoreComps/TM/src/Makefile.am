pkginclude_HEADERS = Socket/GCF_ETHRawPort.h
 
DOCHDRS = $(pkginclude_HEADERS) \
  GTM_Defines.h \
  PortInterface/GTM_Config.h \
  PortInterface/GTM_NameService.h \
  PortInterface/GTM_TopologyService.h \
  Timer/GTM_Timer.h \
  Timer/GTM_TimerHandler.h \
  Socket/GTM_Socket.h \
  Socket/GTM_TCPSocket.h \
  Socket/GTM_TCPServerSocket.h \
  Socket/GTM_SocketHandler.h \
  Socket/GTM_ETHSocket.h \
  Socket/GTM_Device.h
    
noinst_LTLIBRARIES 	= libtm.la

libtm_la_SOURCES=  \
  GCF_Fsm.cc \
  GCF_TMProtocols.cc \
  GCF_Task.cc \
  GCF_Event.cc \
  PortInterface/GCF_Port.cc \
  PortInterface/GCF_RawPort.cc \
  PortInterface/GCF_PeerAddr.cc \
  PortInterface/GTM_Config.cc \
  PortInterface/GTM_NameService.cc \
  PortInterface/GTM_TopologyService.cc \
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
  Socket/GTM_Device.cc

libtm_la_CPPFLAGS=-I$(top_srcdir)/GCFCommon/src

include $(lofar_sharedir)/Makefile.common

