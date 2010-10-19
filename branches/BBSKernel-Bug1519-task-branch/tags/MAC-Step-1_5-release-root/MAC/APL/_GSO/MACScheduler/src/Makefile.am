bin_PROGRAMS 		= MACScheduler

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

MACScheduler_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once
#  -DUSE_PVSSPORT

MACScheduler_LDADD		= -lpqxx $(LOFAR_DEPEND)
MACScheduler_DEPENDENCIES	= $(LOFAR_DEPEND)

MACScheduler_SOURCES 		= \
  $(DOCHDRS) \
	MACScheduler.cc \
	BeamletAllocator.cc \
	LogicalSegmentAllocator.cc \
        WanLSPropertyProxy.cc \
	MACSchedulerMain.cc \
	SAS_Protocol.cc

NOINSTHDRS = \
	MACScheduler.h \
	BeamletAllocator.h \
	LogicalSegmentAllocator.h \
        WanLSPropertyProxy.h

INSTHDRS = 

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS	=	$(pkginclude_HEADERS) $(BUILT_SOURCES)

BUILT_SOURCES = \
	$(ph_HEADERS)

phdir=$(includedir)/APL/APLCommon
ph_HEADERS = \
	SAS_Protocol.ph

EXTRA_DIST = $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA = \
	customPrepPVSSDB.ctl

sysconf_DATA= \
	MACScheduler.conf \
	MACScheduler.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

%.ctl: %.ctl.in
	cp $< $@

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
