dmi_dir = $(lofar_sharedir)/../DMI

CXXFLAGS += -Wall -Wno-unknown-pragmas -Wno-deprecated -DUSE_THREADS -D_GNU_SOURCE -pthread

lib_LTLIBRARIES = liboctopussy-glish.la

liboctopussy_glish_la_SOURCES = GlishClientWP.cc AID-Glish-Registry.cc
  
include $(lofar_sharedir)/Makefile.common

