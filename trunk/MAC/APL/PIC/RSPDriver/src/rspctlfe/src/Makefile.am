AM_CPPFLAGS = \
	-I$(top_srcdir)/src

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	RSPFE_Protocol.ph \
	rspctlfe.conf

clean-local:
	rm -f \
	RSPFE_Protocol.ph \
	RSPFE_Protocol.cc

EXTRA_DIST = \
	RSPFE_Protocol.ph \
	$(BUILT_SOURCES)

sysconf_DATA = \
	rspctlfe.conf

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

lib_LTLIBRARIES = librspfe.la

include_HEADERS = \
	RSPFE_Protocol.ph
	
librspfe_la_SOURCES = \
  RSPFE_Protocol.cc
  
bin_PROGRAMS = \
	rspctlfe

rspctlfe_SOURCES = \
	rspctlfe.h \
	rspctlfe.cc

rspctlfe_LDADD = \
	$(top_builddir)/src/librspfe.la

include $(top_srcdir)/Makefile.common
