pkginclude_HEADERS = Package__Version.h

#
# if USE_CAL_THREAD is defined in Calibration/src/Makefile.am it should be
# defined here as well and vice versa
#

AM_CPPFLAGS 			= -I$(top_srcdir)/src \
						  -DUSE_CAL_THREAD

AUTOGEN 				= autogen
SUFFIXES 				= .ph

BUILT_SOURCES 			= 

EXTRA_DIST 				= $(BUILT_SOURCES)

sysconf_DATA 			= 

bin_PROGRAMS 			= iBeamServer ibeamctl

iBeamServer_SOURCES 		= BeamServerMain.cc BeamServerConstants.h \
							  BeamServer.h BeamServer.cc \
							  Beam.h Beam.cc \
							  DigitalBeam.h DigitalBeam.cc \
							  AnalogueBeam.h AnalogueBeam.cc \
							  AnaBeamMgr.h AnaBeamMgr.cc \
							  J2000Converter.h J2000Converter.cc \
							  CasaConverter.h CasaConverter.cc
iBeamServer_LDADD 			= 
iBeamServer_DEPENDENCIES	= $(LOFAR_DEPEND)

ibeamctl_SOURCES 		= beamctl.cc
ibeamctl_LDADD 			= 
ibeamctl_DEPENDENCIES	= $(LOFAR_DEPEND)

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

include $(top_srcdir)/Makefile.common
