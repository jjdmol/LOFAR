
if HAVE_SYS_CAPABILITY_H
CAPLIBS=$(top_builddir)/src/libnetraw.la -lcap
else
CAPLIBS=
endif

#
# Define ENABLE_CAP_NET_RAW to let RSPDriver run as setuid root
# with only cap_net_raw capability enabled.
#
AM_CPPFLAGS =

INSTALL=install -C

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = 

clean-local:
	rm -f *.ph

EXTRA_DIST = \
	$(BUILT_SOURCES)

etcdir=$(prefix)/etc
etc_SCRIPTS = \
	rc.status

sysconf_DATA =

lib_LTLIBRARIES = librtccommon.la libnetraw.la libgnuplot_i.la

include_HEADERS = \
	Marshalling.h \
	TestSuite.h \
	Timestamp.h \
	Observer.h \
	Subject.h \
	netraw.h \
	gnuplot_i.h \
	PSAccess.h

librtccommon_la_SOURCES = \
	Marshalling.h \
	TestSuite.h \
	Timestamp.h \
	Timestamp.cc \
	Observer.h \
	Subject.h \
	Subject.cc

libnetraw_la_SOURCES = \
	netraw.h \
	netraw.cc

libgnuplot_i_la_SOURCES = \
	gnuplot_i.h \
	gnuplot_i.cc

%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

%.log_prop: %.log_prop.in
	cp $< $@

%.ns: %.ns.in
	cp $< $@

%.top: %.top.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
