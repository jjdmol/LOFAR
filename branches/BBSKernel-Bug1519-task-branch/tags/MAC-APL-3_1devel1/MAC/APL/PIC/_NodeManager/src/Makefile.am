INSTALL=/usr/bin/install -C

AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

pkgincludedir = $(includedir)/APL
pkginclude_HEADERS = \
	../include/APL/NodeManager.h \
	../include/APL/NMUtilities.h
	
DOCHDRS	=	$(pkginclude_HEADERS) \
	NodeManagerDaemon.h \
	NodeManagerClient.h \
	PropertyProxy.h \
	NM_Protocol.h
   
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

EXTRA_DIST = \
	NM_Protocol.ph
	
configfilesdir=$(bindir)
configfiles_DATA= \
	NodeManager.conf
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

