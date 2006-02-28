lib_LTLIBRARIES			= libamcimpl.la

libamcimpl_la_SOURCES 		= ConverterImpl.cc \
				  AMCServer/ConverterProcess.cc \
				  AMCServer/ConverterServer.cc

bin_PROGRAMS			= amcserver

amcserver_SOURCES		= AMCServer/amcserver.cc
amcserver_LDADD			= libamcimpl.la
amcserver_DEPENDENCIES		= libamcimpl.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
