AM_CPPFLAGS = -I$(top_srcdir)/SAL/src

check_PROGRAMS 		= salEcho salPing

TESTS			= #salEcho salPing

salEcho_SOURCES 			= Echo_Protocol.ph Echo.cc Service.cc Echo_Protocol.cc
salEcho_LDADD					= ../src/libsal.la $(LOFAR_DEPEND)
salEcho_DEPENDENCIES	= ../src/libsal.la $(LOFAR_DEPEND)

salPing_SOURCES 					= Echo_Protocol.ph Ping.cc  Echo_Protocol.cc
salPing_LDADD						=  ../src/libsal.la $(LOFAR_DEPEND)
salPing_DEPENDENCIES			= ../src/libsal.la $(LOFAR_DEPEND)

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = 

EXTRA_DIST = Echo_Protocol.prot $(sysconf_DATA)

sysconf_DATA= \
	salEcho.conf \
	salPing.conf

%.conf: sal-test.conf.in
	ln -sf $< $@

include $(top_srcdir)/Makefile.common



