AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

pkgincludedir = $(includedir)/APLCommon

NOINSTHDRS = \
	../include/APLCommon/APL_Defines.h \
	../include/APLCommon/APLCommonExceptions.h \
	../include/APLCommon/APLUtilities.h \
	../include/APLCommon/LogicalDevice.h \
	../include/APLCommon/LogicalDeviceFactoryBase.h \
	../include/APLCommon/LogicalDeviceFactory.h \
	../include/APLCommon/SharedLogicalDeviceFactory.h \
	../include/APLCommon/SingleInstanceLogicalDeviceFactory.h \
	../include/APLCommon/PropertySetAnswer.h \
	../include/APLCommon/PropertySetAnswerHandlerInterface.h \
	../include/APLCommon/StartDaemon.h

INSTHDRS = \
	LogicalDevice_Protocol.ph \
	StartDaemon_Protocol.ph

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS	=	$(pkginclude_HEADERS)
      
lib_LTLIBRARIES 	= libaplcommon.la

libaplcommon_la_CPPFLAGS= \
  -I$(top_srcdir)/include \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

libaplcommon_la_SOURCES= $(DOCHDRS) \
	APLUtilities.cc \
	LogicalDevice.cc \
	StartDaemon.cc \
	PropertySetAnswer.cc \
	LogicalDevice_Protocol.cc \
	StartDaemon_Protocol.cc

BUILT_SOURCES = \
	LogicalDevice_Protocol.ph \
	StartDaemon_Protocol.ph

EXTRA_DIST = \
	LogicalDevice_Protocol.ph \
	StartDaemon_Protocol.ph

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common

