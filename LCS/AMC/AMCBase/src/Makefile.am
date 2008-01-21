lib_LTLIBRARIES		= libamcbase.la

libamcbase_la_SOURCES = Package__Version.cc  \
	BlobIO.cc \
	ConverterClient.cc \
	ConverterCommand.cc \
	ConverterStatus.cc \
	Coord3D.cc \
	DH_Request.cc \
	DH_Result.cc \
	Direction.cc \
	Epoch.cc \
	Position.cc

bin_PROGRAMS = versionamcbase

versionamcbase_SOURCES      = versionamcbase.cc
versionamcbase_LDADD        = libamcbase.la
versionamcbase_DEPENDENCIES = libamcbase.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
