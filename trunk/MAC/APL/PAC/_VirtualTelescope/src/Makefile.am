lib_LTLIBRARIES 	= libvirtualtelescope.la

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

pkginclude_HEADERS = \
	../include/VirtualTelescope/VirtualTelescope.h \
	../include/VirtualTelescope/VirtualTelescopeFactory.h

libvirtualtelescope_la_CPPFLAGS= \
	-I $(top_srcdir)/include \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

libvirtualtelescope_la_SOURCES 		= \
	VirtualTelescope.cc 
	
include_HEADERS = 
  
BUILT_SOURCES = 

EXTRA_DIST = $(BUILT_SOURCES)

%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
