INSTALL=/usr/bin/install -C

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../MACCommon/autogen $<

pkgincludedir = $(includedir)/APLCommon

pkginclude_HEADERS = \
	../include/APLCommon/APLInterTaskPort.h \
	../include/APLCommon/APL_Defines.h \
	../include/APLCommon/LogicalDevice.h \
	../include/APLCommon/PropertySetAnswer.h \
	../include/APLCommon/PropertySetAnswerHandlerInterface.h \
	LogicalDevice_Protocol.ph \
	StartDaemon_Protocol.ph

DOCHDRS	=	$(pkginclude_HEADERS)
      
lib_LTLIBRARIES 	= libaplcommon.la

libaplcommon_la_CPPFLAGS= \
  -I$(top_srcdir)/include \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

libaplcommon_la_SOURCES= $(DOCHDRS) \
	APLInterTaskPort.cc \
	LogicalDevice.cc \
	PropertySetAnswer.cc \
	LogicalDevice_Protocol.cc \
	StartDaemon_Protocol.cc

BUILT_SOURCES = \
	LogicalDevice_Protocol.ph \
	StartDaemon_Protocol.ph

EXTRA_DIST = \
	LogicalDevice_Protocol.ph \
	StartDaemon_Protocol.ph

include $(lofar_sharedir)/Makefile.common

