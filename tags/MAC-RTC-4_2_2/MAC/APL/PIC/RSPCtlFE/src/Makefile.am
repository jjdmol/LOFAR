bin_PROGRAMS = rspctlfe

rspctlfe_CPPFLAGS= \
	-DBOOST_DISABLE_THREADS \
	-Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once \
	-DUSE_TCPPORT_INSTEADOF_PVSSPORT

rspctlfe_SOURCES = \
	rspctlfe.h \
	rspctlfe.cc

rspctlfe_LDADD = $(LOFAR_DEPEND)
rspctlfe_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS =

INSTHDRS =

pkginclude_HEADERS = $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS =       $(pkginclude_HEADERS)

include_HEADERS =

BUILT_SOURCES = \
	rspctlfe.conf

EXTRA_DIST = $(BUILT_SOURCES) $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA =

sysconf_DATA= $(BUILT_SOURCES)

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
