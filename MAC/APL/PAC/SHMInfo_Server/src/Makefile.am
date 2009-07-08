AM_CPPFLAGS = -DUSES_NEW_IOSTREAM
pkginclude_HEADERS = Package__Version.h

AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

DOCHDRS	=	 $(BUILT_SOURCES) \
	SHMInfoServer.h \
	SHMDefines.h \
	SHMSession.h \
	XCStatistics.h \
	RspStatus.h

noinst_LTLIBRARIES 	= libshminfoserver.la

libshminfoserver_la_SOURCES	= Package__Version.cc \
	$(DOCHDRS) \
	SHMInfoServer.cc \
	SHMSession.cc \
	SHM_Protocol.cc \
	XCStatistics.cc \
	RspStatus.cc

bin_PROGRAMS   		= versionshminfo_server SHMInfoServer
  
SHMInfoServer_SOURCES		  = SHMInfoServerMain.cc
SHMInfoServer_LDADD		    = libshminfoserver.la $(LOFAR_DEPEND)
SHMInfoServer_DEPENDENCIES	= libshminfoserver.la $(LOFAR_DEPEND)

BUILT_SOURCES = SHM_Protocol.ph

EXTRA_DIST = $(sysconf_DATA)
	
sysconf_DATA= 
	
%.conf: %.conf.in
	cp $< $@

clean-local:
	rm -f *.ph
	
versionshminfo_server_SOURCES      = versionshminfo_server.cc
versionshminfo_server_LDADD        = libshminfoserver.la
versionshminfo_server_DEPENDENCIES = libshminfoserver.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

