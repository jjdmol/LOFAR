lib_LTLIBRARIES = libvisagent.la

AIDHDRS = VisAgentVocabulary.h

DOCHDRS = DataStreamMap.h InputAgent.h OutputAgent.h 

libvisagent_la_SOURCES = AID-VisAgent-Registry.cc \
  DataStreamMap.cc InputAgent.cc OutputAgent.cc 

include $(lofar_sharedir)/Makefile.common
include $(lofar_sharedir)/Makefile.aids


AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
