bin_PROGRAMS 				= MACScheduler ObservationControl

MACScheduler_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
							  -Wno-deprecated \
							  -fmessage-length=0 \
							  -fdiagnostics-show-location=once

MACScheduler_SOURCES		= MACScheduler.cc \
							  MACSchedulerMain.cc \
							  ChildControl.cc \
							  LDState.cc
							  ParentControl.cc

MACScheduler_LDADD			= -lpqxx $(LOFAR_DEPEND)
MACScheduler_DEPENDENCIES	= $(LOFAR_DEPEND)

ObservationControl_CPPFLAGS		= -DBOOST_DISABLE_THREADS \
								  -Wno-deprecated \
								  -fmessage-length=0 \
								  -fdiagnostics-show-location=once

ObservationControl_SOURCES		= ObservationControl.cc \
								  ObservationControlMain.cc \
								  ChildControl.cc \
								  LDState.cc\
								  ParentControl.cc

ObservationControl_LDADD		= $(LOFAR_DEPEND)
ObservationControl_DEPENDENCIES	= $(LOFAR_DEPEND)

NOINSTHDRS 					= MACScheduler.h \
							  MACSchedulerDefines.h

INSTHDRS 					= ChildControl.h \
							  ParentControl.h \
							  LDState.h

pkginclude_HEADERS 			= $(NOINSTHDRS) $(INSTHDRS)

DOCHDRS						= $(pkginclude_HEADERS) $(BUILT_SOURCES)

EXTRA_DIST 					= $(configfiles_DATA) $(sysconf_DATA)

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA 	= customPrepPVSSDB.ctl

sysconf_DATA		= MACScheduler.conf \
					  MACScheduler.log_prop \
					  ObservationControl.conf \
					  ObservationControl.log_prop

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

%.ctl: %.ctl.in
	cp $< $@

clean-local:
	rm -f *.ph

include $(top_srcdir)/Makefile.common
