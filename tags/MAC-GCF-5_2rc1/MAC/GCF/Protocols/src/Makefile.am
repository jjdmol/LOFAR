pkgincludedir = $(includedir)/GCF/Protocols
pkginclude_HEADERS = \
	DH_PIProtocol.h \
	PI_Protocol.ph \
	PA_Protocol.ph

AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

DOCHDRS	= $(pkginclude_HEADERS)
    
lib_LTLIBRARIES 	= libgcfpicepprot.la libgcfpirtcprot.la libgcfpaprot.la

libgcfpicepprot_la_SOURCES= $(DOCHDRS) \
  DH_PIProtocol.cc

libgcfpicepprot_la_CPPFLAGS= -I$(top_srcdir)/include

libgcfpirtcprot_la_SOURCES= $(DOCHDRS) \
  PI_Protocol.cc

libgcfpaprot_la_SOURCES= $(DOCHDRS) \
  PA_Protocol.cc

BUILT_SOURCES = \
	PI_Protocol.ph \
	PA_Protocol.ph

EXTRA_DIST = \
	PI_Protocol.ph \
	PA_Protocol.ph

clean-local:
	rm -f *.ph

include $(lofar_sharedir)/Makefile.common

