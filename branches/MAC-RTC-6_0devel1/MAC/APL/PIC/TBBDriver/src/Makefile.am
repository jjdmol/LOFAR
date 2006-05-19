
#
# Define ENABLE_RSPFE to enable the rspctl frontend
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/src 

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	TBBDriver.conf \
	TBBDriver.log_prop 

clean-local:
	rm -f *.ph

EXTRA_DIST = \
	$(BUILT_SOURCES)

sysconf_DATA = \
	TBBDriver.conf \
	TBBDriver.log_prop 

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

include_HEADERS =

bin_PROGRAMS = \
	TBBDriver

TBBDriver_SOURCES = \
	TBBDriver.cc \
	TBBDriver.h

TBBDriver_LDADD = \
	$(RTCCOMMON_LDADD)

include $(top_srcdir)/Makefile.common
