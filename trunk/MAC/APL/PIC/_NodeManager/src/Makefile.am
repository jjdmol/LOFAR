INSTALL=/usr/bin/install -C

AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(top_srcdir)/../../../MACCommon/autogen $<

pkgincludedir = $(includedir)/APL
pkginclude_HEADERS = \
	../include/APL/NodeManager.h
	
DOCHDRS	=	$(pkginclude_HEADERS) \
	NodeManagerDaemon.h \
	NodeManagerClient.h \
	PropertyProxy.h \
	NM_Protocol.h
   
lib_LTLIBRARIES 	= libnm.la

libnm_la_SOURCES	= $(DOCHDRS) \
	NodeManager.cc \
	NM_Protocol.cc
	
libnm_la_CPPFLAGS = -I$(top_srcdir)/include

noinst_LTLIBRARIES 	= libnmd.la

libnmd_la_SOURCES	= $(DOCHDRS) \
	NodeManagerDaemon.cc \
	NodeManagerClient.cc \
	PropertyProxy.cc \
	NM_Protocol.cc

bin_PROGRAMS   		= NodeManager
  
NodeManager_SOURCES		  = NMMain.cc
NodeManager_LDADD		    = libnmd.la $(LOFAR_DEPEND)
NodeManager_DEPENDENCIES	= libnmd.la $(LOFAR_DEPEND)
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

