AM_CPPFLAGS = -DUSES_NEW_IOSTREAM
pkginclude_HEADERS = Package__Version.h

AUTOGEN = autogen
SUFFIXES = .ph

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/MACIO $<

DOCHDRS	=	 $(BUILT_SOURCES) \
	MISDaemon.h \
	MISDefines.h \
	MISSession.h \
	XCStatistics.h \
	RspStatus.h

#	MISSubscription.h 
#	PropertyProxy.h 

noinst_LTLIBRARIES 	= libmis.la

libmis_la_SOURCES	= Package__Version.cc \
	$(DOCHDRS) \
	MISDaemon.cc \
	MISSession.cc \
	MIS_Protocol.cc \
	XCStatistics.cc \
	RspStatus.cc

#	MISSubscription.cc 
#	PropertyProxy.cc 

bin_PROGRAMS   		= versionmis MACInformationServer
  
MACInformationServer_SOURCES		  = MISMain.cc
MACInformationServer_LDADD		    = libmis.la $(LOFAR_DEPEND)
MACInformationServer_DEPENDENCIES	= libmis.la $(LOFAR_DEPEND)

BUILT_SOURCES = \
	MIS_Protocol.ph

EXTRA_DIST = $(sysconf_DATA)
	
sysconf_DATA= 
	
%.conf: %.conf.in
	cp $< $@

clean-local:
	rm -f *.ph
	
versionmis_SOURCES      = versionmis.cc
versionmis_LDADD        = libmis.la
versionmis_DEPENDENCIES = libmis.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

