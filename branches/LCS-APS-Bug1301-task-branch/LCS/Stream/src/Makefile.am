lib_LTLIBRARIES			= libstream.la 

libstream_la_DEPENDENCIES	= $(LOFAR_DEPEND)

libstream_la_SOURCES		= Package__Version.cc  \
	FileDescriptorBasedStream.cc \
	FileStream.cc \
	NullStream.cc \
	SocketStream.cc \
	Stream.cc \
	SystemCallException.cc

bin_PROGRAMS = versionstream

versionstream_SOURCES      = versionstream.cc
versionstream_LDADD        = libstream.la
versionstream_DEPENDENCIES = libstream.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common

