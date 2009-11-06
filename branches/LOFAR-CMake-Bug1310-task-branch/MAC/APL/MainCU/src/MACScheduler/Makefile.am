bin_PROGRAMS 				= MACScheduler 

MACScheduler_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
							  -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

MACScheduler_SOURCES		= ../Package__Version.cc \
							  MACScheduler.cc \
							  ObsClaimer.cc \
							  MACSchedulerMain.cc

MACScheduler_LDADD			= -lpqxx $(LOFAR_DEPEND)
MACScheduler_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= MACScheduler.h \
							  ObsClaimer.h \
							  MACSchedulerDefines.h

INSTHDRS 					= 

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= 

sysconf_DATA		= MACScheduler.conf

%.conf: %.conf.in
	cp $< $@

include $(top_srcdir)/Makefile.common
