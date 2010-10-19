bin_PROGRAMS 		= MACScheduler

AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

MACScheduler_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

MACScheduler_SOURCES 		= \
	MACScheduler.cc \
	BeamletAllocator.cc \
	MACSchedulerMain.cc \
	SAS_Protocol.cc

NOINSTHDRS = \
	MACScheduler.h \
	BeamletAllocator.h \
	SAS_Protocol.ph

INSTHDRS = 

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS	=	$(pkginclude_HEADERS)

MACScheduler_LDADD		= -lpqxx $(LOFAR_DEPEND)
MACScheduler_DEPENDENCIES	= $(LOFAR_DEPEND)

include_HEADERS = 

BUILT_SOURCES = 	\
	SAS_Protocol.ph \
	MACScheduler.conf \
	MACScheduler.log_prop \
	customPrepPVSSDB.ctl

EXTRA_DIST = $(BUILT_SOURCES) $(configfiles_DATA) $(sysconf_DATA)

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
