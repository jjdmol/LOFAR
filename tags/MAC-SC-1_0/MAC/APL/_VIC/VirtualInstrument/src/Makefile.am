bin_PROGRAMS 		= VirtualInstrument

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

VirtualInstrument_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

VirtualInstrument_SOURCES 		= \
	VirtualInstrument.cc \
	VirtualInstrumentStartDaemonMain.cc
	
VirtualInstrument_LDADD		= 
	
include_HEADERS = 

BUILT_SOURCES = 	

EXTRA_DIST =   

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA = \
	VirtualInstrument.conf \
	VirtualInstrument.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
