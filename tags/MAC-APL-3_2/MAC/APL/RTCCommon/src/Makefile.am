
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
	$(BUILT_SOURCES) \
	rc.status

etcdir=$(prefix)/etc
etc_SCRIPTS = \
	rc.status

sysconf_DATA =

lib_LTLIBRARIES = librtccommon.la libgnuplot_i.la

librtccommon_la_SOURCES = \
	Timestamp.cc \
	Subject.cc

libgnuplot_i_la_SOURCES = \
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

include $(top_srcdir)/Makefile.common
