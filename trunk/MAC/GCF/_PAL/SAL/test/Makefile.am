cppflags = -I$(top_srcdir)/SAL/src \
	-I$(top_srcdir)/include

check_PROGRAMS 		= salEcho salPing

TESTS			= #salEcho salPing

salEcho_SOURCES 			= Echo.cc Service.cc Echo_Protocol.cc
salEcho_LDADD					= ../src/libsal.la $(LOFAR_DEPEND)
salEcho_DEPENDENCIES	= ../src/libsal.la $(LOFAR_DEPEND)
salEcho_CPPFLAGS			= $(cppflags)

salPing_SOURCES 					= Ping.cc  Echo_Protocol.cc
salPing_LDADD						=  ../src/libsal.la $(LOFAR_DEPEND)
salPing_DEPENDENCIES			= ../src/libsal.la $(LOFAR_DEPEND)
salPing_CPPFLAGS= $(cppflags)

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

BUILT_SOURCES = \
	Echo_Protocol.ph

EXTRA_DIST = $(BUILT_SOURCES)

sysconf_DATA= \
	salEcho.conf \
	salPing.conf

%.conf: sal-test.conf.in
	ln -sf $< $@

clean-local:
	rm -f *.ph

include $(lofar_sharedir)/Makefile.common



