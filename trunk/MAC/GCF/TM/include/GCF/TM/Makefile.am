INSTHDRS =

pkgincludedir = $(includedir)/GCF/TM
pkginclude_HEADERS = \
	GCF_Control.h \
	GCF_DevicePort.h \
	GCF_ETHRawPort.h \
	GCF_Event.h \
	GCF_Fsm.h \
	GCF_Handler.h \
	GCF_ITCPort.h \
	GCF_Port.h \
	GCF_PortInterface.h \
	GCF_Protocols.h \
	GCF_RawPort.h \
	GCF_Task.h \
	GCF_TCPPort.h \
	GCF_TimerPort.h \
	EventPort.h 

NOINSTHDRS =

TCCHDRS = 

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common

