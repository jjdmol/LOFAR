dmi_dir = $(lofar_sharedir)/../DMI

CXXFLAGS += -Wall -Wno-unknown-pragmas -Wno-deprecated -D_GNU_SOURCE

lib_LTLIBRARIES = liboctopussy-glish.la

liboctopussy_glish_la_SOURCES = GlishClientWP.cc AID-Glish-Registry.cc

bin_PROGRAMS   		= octoglish

octoglish_SOURCES 	= octoglish.cc 

octoglish_LDADD		= liboctopussy-glish.la ../liboctopussy.la \
  ../../test/AID-Testing-Registry.o \
  -lglish -lsos -lnpd

octoglish_DEPENDENCIES	= liboctopussy-glish.la ../liboctopussy.la \
  ../../test/AID-Testing-Registry.o $(LOFAR_DEPEND)
  
include $(lofar_sharedir)/Makefile.common

