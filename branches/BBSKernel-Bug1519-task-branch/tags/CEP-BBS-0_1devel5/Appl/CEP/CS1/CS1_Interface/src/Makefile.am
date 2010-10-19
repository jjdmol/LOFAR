lib_LTLIBRARIES			= libcs1_interface.la

libcs1_interface_la_SOURCES	= Package__Version.cc \
				  Allocator.cc \
				  BGL_Configuration.cc \
				  BGL_Mapping.cc \
				  CS1_Parset.cc \
				  RSPTimeStamp.cc

configfilesdir			= $(bindir)

bin_PROGRAMS = versioncs1_interface

versioncs1_interface_SOURCES      = versioncs1_interface.cc
versioncs1_interface_LDADD        = libcs1_interface.la
versioncs1_interface_DEPENDENCIES = libcs1_interface.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
