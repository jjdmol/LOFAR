AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/autogen $<

check_PROGRAMS 		= tmEcho tmPing tGCFPort tProtocol tEventPort tTimer

tmEcho_SOURCES 		= $(BUILT_SOURCES) Echo_Protocol.cc Echo.cc Echo.h 
tmEcho_LDADD		= ../src/libgcftm.la
tmEcho_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tmPing_SOURCES 		= $(BUILT_SOURCES) Echo_Protocol.cc Ping.cc Ping.h
tmPing_LDADD		= ../src/libgcftm.la
tmPing_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tGCFPort_SOURCES 		= $(BUILT_SOURCES) Echo_Protocol.cc \
						  tGCFPort.cc tGCFPort.h 
tGCFPort_LDADD			= ../src/libgcftm.la
tGCFPort_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tProtocol_SOURCES 		= testprotocol.ph testprotocol.cc \
						  Echo_Protocol.ph Echo_Protocol.cc \
						  tProtocol.cc 
tProtocol_LDADD			= ../src/libgcftm.la
tProtocol_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)

tEventPort_SOURCES 		= tEventPort.cc
tEventPort_LDADD		= ../src/ServiceBroker/libeventport.la \
						  $(libdir)/librsp_protocol.la \
						  -L$(includedir)/APL/RSP_Protocol
tEventPort_DEPENDENCIES	= ../src/ServiceBroker/libeventport.la $(LOFAR_DEPEND)

tTimer_SOURCES 		= $(BUILT_SOURCES) tTimer.cc
tTimer_LDADD		= ../src/libgcftm.la
tTimer_DEPENDENCIES	= ../src/libgcftm.la tTimer.h $(LOFAR_DEPEND)

BUILT_SOURCES = Echo_Protocol.ph testprotocol.ph

EXTRA_DIST = \
	tmEcho.conf \
	tmPing.conf \
	tGCFPort.conf \
	tProtocol.log_prop
	
%.conf:	tm-test.conf.in
	ln -sf $< $@

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common



