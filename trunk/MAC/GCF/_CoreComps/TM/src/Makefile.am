include_HEADERS =	
 
DOCHDRS = $(include_HEADERS) \
  GTM_Defines.h \
  PortInterface/GTM_Config.h \
  PortInterface/GTM_NameService.h \
  PortInterface/GTM_TopologyService.h \
  Timer/GTM_Timer.h \
  Timer/GTM_TimerHandler.h \
  Socket/GTM_Socket.h \
  Socket/GTM_ServerSocket.h \
  Socket/GTM_SocketHandler.h
    
noinst_LTLIBRARIES 	= libtm.la

libtm_la_SOURCES=  \
  GCF_Fsm.cc \
  GCF_TMProtocols.cc \
  GCF_Task.cc \
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
  Socket/GTM_ServerSocket.cc \
  Socket/GTM_SocketHandler.cc 

libtm_la_CPPFLAGS=-I$(top_srcdir)/GCFCommon/src

include $(lofar_sharedir)/Makefile.common

