bin_PROGRAMS 		= THPVSSBridge

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

THPVSSBridge_CPPFLAGS= \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

THPVSSBridge_SOURCES 		= \
	THPVSSBridge.cc \
	THPVSSBridgeMain.cc \
	THPVSSBridge_Protocol.cc
	
THPVSSBridge_LDADD		= 
	
include_HEADERS = 

BUILT_SOURCES = 	\
	THPVSSBridge_Protocol.ph

EXTRA_DIST =   \
	THPVSSBridge_Protocol.ph

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA = \
	THPVSSBridge.conf \
	THPVSSBridge.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
