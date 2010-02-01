check_PROGRAMS			= tConverter tJ2000Converter tAnaBeamMgr

tConverter_SOURCES		= tConverter.cc
tConverter_DEPENDENCIES	= $(LOFAR_DEPEND)

tJ2000Converter_SOURCES			= tJ2000Converter.cc ../src/J2000Converter.cc
tJ2000Converter_DEPENDENCIES	= $(LOFAR_DEPEND)

tAnaBeamMgr_SOURCES			= tAnaBeamMgr.cc ../src/Beam.cc ../src/AnalogueBeam.cc ../src/AnaBeamMgr.cc
tAnaBeamMgr_DEPENDENCIES	= $(LOFAR_DEPEND)


include $(top_srcdir)/Makefile.common
