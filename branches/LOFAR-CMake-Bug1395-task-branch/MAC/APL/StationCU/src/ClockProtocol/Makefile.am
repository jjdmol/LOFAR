AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO  $<
	cp $*.ph $(top_builddir)/include/APL/ClockProtocol

%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

EXTRA_DIST 				= Clock_Protocol.prot

BUILT_SOURCES 			= Clock_Protocol.ph Clock_Protocol.cc

pkgincludedir			= $(includedir)/APL/ClockProtocol
pkginclude_HEADERS		= $(BUILT_SOURCES)

lib_LTLIBRARIES			= libclock_protocol.la

libclock_protocol_la_SOURCES = $(BUILT_SOURCES)

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
