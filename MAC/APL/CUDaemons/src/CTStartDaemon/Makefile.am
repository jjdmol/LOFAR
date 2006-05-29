# Executables
bin_PROGRAMS 				= LDStartDaemon

LDStartDaemon_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
							  -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

LDStartDaemon_SOURCES		= LogicalDeviceStarter.cc \
							  LDStartDaemon.cc \
							  LDStartDaemonMain.cc
LDStartDaemon_LDADD			= $(LOFAR_DEPEND) 
LDStartDaemon_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= LDStartDaemon.h \
							  LogicalDeviceStarter.h

INSTHDRS 					= 

BUILT_SOURCES				= $(ph_HEADERS)

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 

sysconf_DATA		= LDStartDaemon.conf \
					  LDStartDaemon.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

%.ctl: %.ctl.in
	cp $< $@

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
