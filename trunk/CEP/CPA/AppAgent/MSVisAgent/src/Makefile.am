dmi_dir = $(lofar_sharedir)/../DMI

lib_LTLIBRARIES = libmsvisagent.la

DOCHDRS = MSInputSink.h MSOutputSink.h MSVisAgentVocabulary.h

libmsvisagent_la_SOURCES = \
  MSInputSink.cc \
  MSOutputSink.cc \
  MSVisAgentDebugContext.cc \
  AID-MSVisAgent-Registry.cc

include $(lofar_sharedir)/Makefile.common
include $(lofar_sharedir)/Makefile.aids


AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
