DOCHDRS	=	\
  GCF_Defines.h \
  GCF_Event.h \
  GCF_Fsm.h \
  GCF_TMProtocols.h \
  GCF_Task.h \
  GCF_Handler.h \
  PortInterface/GCF_PortInterface.h \
  PortInterface/GCF_Port.h \
  PortInterface/GCF_RawPort.h \
  PortInterface/GCF_PeerAddr.h \
  PortInterface/GTM_Config.h \
  PortInterface/GTM_NameService.h \
  PortInterface/GTM_TopologyService.h \
  Timer/GTM_Timer.h \
  Timer/GTM_TimerHandler.h \
  Socket/GCF_TCPPort.h \
  Socket/GTM_Socket.h \
  Socket/GTM_ServerSocket.h \
  Socket/GTM_HandlerSocket.h
    
lib_LTLIBRARIES 	= libtm.la

libtm_la_SOURCES= $(DOCHDRS) \
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

include $(lofar_sharedir)/Makefile.common

