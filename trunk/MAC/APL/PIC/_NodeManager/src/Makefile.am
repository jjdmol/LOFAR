AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

pkgincludedir = $(includedir)/APL
pkginclude_HEADERS = \
	../include/APL/NodeManager.h \
	../include/APL/NMUtilities.h
	
DOCHDRS	=	$(pkginclude_HEADERS) \
	NodeManagerDaemon.h \
	NodeManagerClient.h \
	PropertyProxy.h \
	NMDefines.h \
	NM_Protocol.ph
   
lib_LTLIBRARIES 	= libnm.la

libnm_la_SOURCES	= $(DOCHDRS) \
	NodeManager.cc
	
libnm_la_CPPFLAGS = -I$(top_srcdir)/include
libnm_la_LIBADD = libsnm.la

noinst_LTLIBRARIES 	= libnmd.la libsnm.la

libnmd_la_SOURCES	= $(DOCHDRS) \
	NodeManagerDaemon.cc \
	NodeManagerClient.cc \
	PropertyProxy.cc
libnmd_la_CPPFLAGS = -I$(top_srcdir)/include

libsnm_la_SOURCES	= $(DOCHDRS) \
	NMUtilities.cc \
	NM_Protocol.cc
libsnm_la_CPPFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS   		= NodeManager
  
NodeManager_SOURCES		  = NMMain.cc
NodeManager_LDADD		    = libnmd.la libsnm.la $(LOFAR_DEPEND)
NodeManager_DEPENDENCIES	= libnmd.la libsnm.la $(LOFAR_DEPEND)
NodeManager_CPPFLAGS			=

BUILT_SOURCES = \
	NM_Protocol.ph

EXTRA_DIST = $(BUILT_SOURCES)
	
sysconf_DATA= \
	NodeManager.conf
	
%.conf: %.conf.in
	cp $< $@

clean-local:
	rm -f *.ph
	
include $(top_srcdir)/Makefile.common

