pkginclude_HEADERS = Package__Version.h

#
# if USE_CAL_THREAD is defined in Calibration/src/Makefile.am it should be
# defined here as well and vice versa
#

AM_CPPFLAGS 			= -I$(top_srcdir)/src \
						  -DUSE_CAL_THREAD

AUTOGEN 				= autogen
SUFFIXES 				= .ph

BUILT_SOURCES 			= BeamServer.conf \
							beamctl.conf \
						  BeamServer.log_prop \
						  beamctl.log_prop \
						  HBADeltas.conf \
						  HBADelays.conf 

EXTRA_DIST 				= $(BUILT_SOURCES)

sysconf_DATA 			= BeamServer.conf \
							beamctl.conf \
						  BeamServer.log_prop \
						  beamctl.log_prop \
						  HBADeltas.conf \
						  HBADelays.conf 

lib_LTLIBRARIES 		= libabsdevel.la
libabsdevel_la_SOURCES 	= Package__Version.cc \
						  Beam.h \
						  Beam.cc \
						  Beamlet.h \
						  Beamlet.cc \
						  Beams.h \
						  Beams.cc \
						  Beamlets.h \
						  Beamlets.cc

bin_PROGRAMS 			= versionbeamserver BeamServer beamctl

BeamServer_SOURCES 		= BeamServer.h BeamServerConstants.h BeamServer.cc
BeamServer_LDADD 		= $(top_builddir)/src/libabsdevel.la
BeamServer_DEPEND 		= $(LOFAR_DEPEND)

beamctl_SOURCES 		= beamctl.h beamctl.cc

bin_SCRIPTS 			= beamserver

%.ph: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

%.cc: %.prot
	$(AUTOGEN) --writable -L $(datadir)/GCF/TM $<

%.properties: %.properties.in
	cp $< $@

%.log_prop: %.log_prop.in
	cp $< $@

%.conf: %.conf.in
	cp $< $@

clean-local:
	rm -f *.ph

versionbeamserver_SOURCES      = versionbeamserver.cc
versionbeamserver_LDADD        = libabsdevel.la
versionbeamserver_DEPENDENCIES = libabsdevel.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
