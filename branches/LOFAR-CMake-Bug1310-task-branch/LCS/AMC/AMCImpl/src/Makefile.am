lib_LTLIBRARIES			= libamcimpl.la

libamcimpl_la_SOURCES 		= Package__Version.cc  ConverterImpl.cc \
				  ConverterProcess.cc \
				  ConverterServer.cc

bin_PROGRAMS			= versionamcimpl AMCServer WGS84toITRF

AMCServer_SOURCES		= AMCServer.cc
AMCServer_LDADD			= libamcimpl.la
AMCServer_DEPENDENCIES		= libamcimpl.la $(LOFAR_DEPEND)

WGS84toITRF_SOURCES		= WGS84toITRF.cc
WGS84toITRF_LDADD		= libamcimpl.la
WGS84toITRF_DEPENDENCIES	= libamcimpl.la $(LOFAR_DEPEND)

versionamcimpl_SOURCES      = versionamcimpl.cc
versionamcimpl_LDADD        = libamcimpl.la
versionamcimpl_DEPENDENCIES = libamcimpl.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
