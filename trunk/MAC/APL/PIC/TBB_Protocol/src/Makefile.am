
AM_CPPFLAGS = \
	-I$(top_builddir)/include

AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<
	cp $*.ph $(top_builddir)/include/APL/TBB_Protocol

EXTRA_DIST = \
	TBB_Protocol.prot

BUILT_SOURCES = \
	TBB_Protocol.ph\
	TBB_Protocol.cc
	
pkgincludedir=$(includedir)/APL/TBB_Protocol
pkginclude_HEADERS = \
	$(BUILT_SOURCES)

lib_LTLIBRARIES         = libtbb_protocol.la

libtbb_protocol_la_SOURCES = Package__Version.cc  \
	$(BUILT_SOURCES) 

clean-local:
	rm -f *.ph

bin_PROGRAMS = versiontbb_protocol

versiontbb_protocol_SOURCES      = versiontbb_protocol.cc
versiontbb_protocol_LDADD        = libtbb_protocol.la
versiontbb_protocol_DEPENDENCIES = libtbb_protocol.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
