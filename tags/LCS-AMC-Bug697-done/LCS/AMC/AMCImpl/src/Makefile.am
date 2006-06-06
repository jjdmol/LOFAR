lib_LTLIBRARIES			= libamcimpl.la

libamcimpl_la_SOURCES 		= ConverterImpl.cc \
				  ConverterProcess.cc \
				  ConverterServer.cc

bin_PROGRAMS			= amcserver

amcserver_SOURCES		= amcserver.cc
amcserver_LDADD			= libamcimpl.la
amcserver_DEPENDENCIES		= libamcimpl.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
