dmi_dir = $(lofar_sharedir)/../DMI

INCLUDES = -I$(dmi_dir)/src -I$(top_srcdir)/src

CXXFLAGS += -Wall -Wno-unknown-pragmas 

lib_LTLIBRARIES = liboctopussy-glish.la

liboctopussy_glish_la_SOURCES = GlishClientWP.cc AID-PSCF_Glish-Registry.cc
  
include $(lofar_sharedir)/Makefile.common

