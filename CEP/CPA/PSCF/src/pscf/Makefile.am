
INCLUDES = -I$(srcdir)/..

CXXFLAGS += -Wno-unknown-pragmas 

lib_LTLIBRARIES = libpscf.la

libpscf_la_SOURCES = Dispatcher.cc  Message.cc  MsgAddress.cc  \
	WPInterface.cc WorkProcess.cc Timestamp.cc PSCFDebugContext.cc

include $(lofar_sharedir)/Makefile.common

