bin_PROGRAMS 		= THEcho

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

THEcho_CPPFLAGS= \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

THEcho_SOURCES 		= \
	THEcho.cc \
	THEcho_Protocol.cc \
	THEchoRouting_Protocol.cc
	
THEcho_LDADD		= 
	
include_HEADERS = 

BUILT_SOURCES = \
	THEcho_Protocol.ph \
	THEchoRouting_Protocol.ph

EXTRA_DIST = $(BUILT_SOURCES)

#in case of make install these files will be copied to the bindir beside the test apps
sysconf_DATA = \
	THEcho.conf \
	THEcho.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
