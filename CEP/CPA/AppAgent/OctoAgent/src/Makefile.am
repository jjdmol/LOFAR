lib_LTLIBRARIES = liboctoagent.la

DOCHDRS = EventSink.h EventMultiplexer.h

liboctoagent_la_SOURCES = AID-OctoAgent-Registry.cc \
  EventSink.cc \
  EventMultiplexer.cc

include $(lofar_sharedir)/Makefile.common
include $(lofar_sharedir)/Makefile.aids


AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
