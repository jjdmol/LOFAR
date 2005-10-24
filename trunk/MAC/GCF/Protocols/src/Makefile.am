pkgincludedir = $(includedir)/GCF/Protocols
pkginclude_HEADERS = $(BUILT_SOURCES) \
	DH_PIProtocol.h 

AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

DOCHDRS	= $(pkginclude_HEADERS)
    
lib_LTLIBRARIES 	= libgcfpicepprot.la libgcfpirtcprot.la libgcfpaprot.la

libgcfpicepprot_la_SOURCES= $(DOCHDRS) \
  DH_PIProtocol.cc

libgcfpirtcprot_la_SOURCES= $(DOCHDRS) \
  PI_Protocol.cc

libgcfpaprot_la_SOURCES= $(DOCHDRS) \
  PA_Protocol.cc

BUILT_SOURCES = \
	PI_Protocol.ph \
	PA_Protocol.ph

EXTRA_DIST = $(BUILT_SOURCES) gcf-pi.conf.in

sysconf_DATA= \
	PropertyInterface.conf
		
%.conf: gcf-pi.conf.in
	cp $< $@

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common

