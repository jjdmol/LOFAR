bin_PROGRAMS 		= THEchoTest

THEchoTest_CPPFLAGS= \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

THEchoTest_SOURCES 		= \
	THEchoTest.cc \
	../src/THEcho_Protocol.cc \
	../src/THEchoRouting_Protocol.cc
	
THEchoTest_LDADD		= 
	
include_HEADERS = 

BUILT_SOURCES =
EXTRA_DIST =

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA = \
	THEchoTest.conf \
	THEchoTest.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
