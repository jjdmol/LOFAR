bin_PROGRAMS 		= tmEcho tmPing

tmEcho_SOURCES 		= Echo.cc Echo_Protocol.cc
tmEcho_LDADD		= ../src/libgcftm.la
tmEcho_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)
tmEcho_CPPFLAGS=-I$(top_srcdir)/include

tmPing_SOURCES 		= Ping.cc Echo_Protocol.cc
tmPing_LDADD		= ../src/libgcftm.la
tmPing_DEPENDENCIES	= ../src/libgcftm.la $(LOFAR_DEPEND)
tmPing_CPPFLAGS=-I$(top_srcdir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/autogen $<

BUILT_SOURCES = \
	Echo_Protocol.ph

EXTRA_DIST = \
	Echo_Protocol.ph

sysconf_DATA=\
	tmEcho.conf \
	tmPing.conf
	
%.conf:	tm-test.conf.in
	ln -sf $< $@

clean-local:
	rm -f *.ph

include $(lofar_sharedir)/Makefile.common



