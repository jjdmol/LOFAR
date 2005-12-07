bin_PROGRAMS = StationStartDaemon

StationStartDaemon_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once
#  -DUSE_PVSSPORT

StationStartDaemon_LDADD = $(LOFAR_DEPEND)
StationStartDaemon_DEPENDENCIES	= $(LOFAR_DEPEND)

StationStartDaemon_SOURCES = \
	$(DOCHDRS) \
	StationStartDaemonMain.cc

NOINSTHDRS =

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS = $(pkginclude_HEADERS) $(BUILT_SOURCES)

include_HEADERS =

BUILT_SOURCES = 

EXTRA_DIST = $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA =

sysconf_DATA= \
	StationStartDaemon.conf \
	StationStartDaemon.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
