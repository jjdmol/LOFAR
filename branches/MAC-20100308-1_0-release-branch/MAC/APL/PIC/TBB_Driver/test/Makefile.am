check_PROGRAMS        =

# programs to run through supplied checktools
CHECKTOOLPROGS        =

# scripts used to run tests
#TESTSCRIPTS           =

# scripts and possible programs to run tests
#TESTS	              = $(TESTSCRIPTS)

XFAIL_TESTS           =

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
#EXTRA_DIST            = $(TESTSCRIPTS)

# Lines to build a test program testprg
#testprg_SOURCES		= testprg.cc
#testprg_LDADD			= ../src/libtbbdriver.la
#testprg_DEPENDENCIES		= ../src/libtbbdriver.la $(LOFAR_DEPEND)

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src

# TESTS = \
# 	TBBTest \
# 	TPStub

AUTOGEN = autogen
SUFFIXES = .ph
%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/TBBDriver/TP_Protocol

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<
	cp $*.ph $(top_builddir)/include/TBBDriver/TP_Protocol
	
BUILT_SOURCES = \
	TBBTest.conf \
	TPStub.conf 

lib_LTLIBRARIES = \
	libtbbstub.la	
	
EXTRA_DIST = \
	$(BUILT_SOURCES) \
	TP_Protocol.prot 

sysconf_DATA = \
	TPStub.conf \
	TBBTest.conf 


%.properties: $(MAC_CONFIG)/%.properties
	cp $< $@

%.conf: %.conf.in
	cp $< $@

libtbbstub_la_SOURCES = \
	StubRawEvent.h \
	StubRawEvent.cc 

sbin_PROGRAMS = \
	TPStub \
	TBBTest

#check_PROGRAMS = \
#	TBBTest

TBBTest_SOURCES = \
	TBBTest.h \
	TBBTest.cc

TPStub_SOURCES = \
	TPStub.h \
	TPStub.cc

TPStub_LDADD = \
	libtbbstub.la \
  $(top_builddir)/src/libtp_protocol.la \
  $(top_builddir)/src/libtbbdriver.la
 
TPStub_DEPENDENCIES = \
	libtbbstub.la \
	$(top_builddir)/src/libtbbdriver.la \
	$(top_builddir)/src/libtp_protocol.la \
	$(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
