lib_LTLIBRARIES			= libamcimpl.la

libamcimpl_la_SOURCES 		= ConverterImpl.cc \
				  ConverterProcess.cc \
				  ConverterServer.cc

bin_PROGRAMS			= amcserver WGS84toITRF

amcserver_SOURCES		= amcserver.cc
amcserver_LDADD			= libamcimpl.la
amcserver_DEPENDENCIES		= libamcimpl.la $(LOFAR_DEPEND)

WGS84toITRF_SOURCES		= WGS84toITRF.cc
WGS84toITRF_LDADD		= libamcimpl.la
WGS84toITRF_DEPENDENCIES	= libamcimpl.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
