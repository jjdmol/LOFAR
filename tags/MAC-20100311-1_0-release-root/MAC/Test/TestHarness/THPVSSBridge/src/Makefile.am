bin_PROGRAMS 		= THPVSSBridge

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

THPVSSBridge_CPPFLAGS= \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

THPVSSBridge_SOURCES 		= $(BUILT_SOURCES) \
	PropertyProxy.cc \
	THPVSSBridge.cc \
	THPVSSBridgeMain.cc \
	THPVSSBridge_Protocol.cc
	
THPVSSBridge_LDADD		= 
	
include_HEADERS = 

BUILT_SOURCES = 	\
	THPVSSBridge_Protocol.ph

EXTRA_DIST =  $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
sysconf_DATA = \
	THPVSSBridge.conf \
	THPVSSBridge.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
