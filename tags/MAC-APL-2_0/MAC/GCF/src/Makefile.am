pkginclude_HEADERS =	\
	GCF_Defines.h \
	GCF_PropertyBase.h \
	GCF_Property.h \
	GCF_MyProperty.h \
	GCF_PropertySetBase.h \
	GCF_PropertySet.h \
	GCF_MyPropertySet.h \
	GCF_Apc.h \
	GCF_Answer.h \
	GCF_PropertyProxy.h \
	GCF_PValue.h \
	GCF_PVBool.h \
	GCF_PVDouble.h \
	GCF_PVInteger.h \
	GCF_PVUnsigned.h \
	GCF_PVString.h \
	GCF_PVChar.h \
	GCF_PVDynArr.h \
	GCF_Control.h \
	GCF_Event.h \
	GCF_Fsm.h \
	GCF_TMProtocols.h \
	GCF_Task.h \
	GCF_Handler.h \
	GCF_PortInterface.h \
	GCF_Port.h \
	GCF_RawPort.h \
	GCF_PeerAddr.h \
	GCF_TCPPort.h \
	GCF_DevicePort.h

DOCHDRS	=	$(pkginclude_HEADERS)

lib_LTLIBRARIES 	= libgcf.la

libgcf_la_SOURCES= $(DOCHDRS)
libgcf_la_LIBADD= \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la \
	$(top_builddir)/CoreComps/TM/src/libtm.la \
	$(top_builddir)/CoreComps/SAL/src/libsal.la \
	$(top_builddir)/CoreComps/PML/src/libpml.la \
	$(top_builddir)/Services/PMLlite/src/libpmllite.la

libgcf_la_DEPENDENCIES= $(LOFAR_DEPEND) \
	$(top_builddir)/GCFCommon/src/libgcfcommon.la \
	$(top_builddir)/CoreComps/TM/src/libtm.la \
	$(top_builddir)/CoreComps/SAL/src/libsal.la \
	$(top_builddir)/CoreComps/PML/src/libpml.la \
	$(top_builddir)/Services/PMLlite/src/libpmllite.la

BUILT_SOURCES =
	log4cplus.properties

EXTRA_DIST =
	log4cplus.properties
	
MAC_CONFIG = $(top_srcdir)/../Config
%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

configfilesdir=$(bindir)
configfiles_DATA=log4cplus.properties
 
include $(lofar_sharedir)/Makefile.common

