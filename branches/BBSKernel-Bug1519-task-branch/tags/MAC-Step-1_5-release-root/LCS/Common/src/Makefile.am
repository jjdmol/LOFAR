if HAVE_SHMEM
SUBDIRS	= shmem .
endif

lib_LTLIBRARIES 	= libcommon.la

libcommon_la_SOURCES 	= Package__Version.cc \
	Allocator.cc \
	DataConvert.cc \
	Exception.cc \
	FileLocator.cc \
	hexdump.cc \
	lofar_bitset.cc \
	LofarLogCout.cc \
	LofarLogger.cc \
	Numeric.cc \
	PrettyUnits.cc \
	Process.cc \
	RunOnNode.cc \
	Stopwatch.cc \
	StreamUtil.cc \
	StringUtil.cc \
	SystemUtil.cc \
	Timer.cc \
	TypeNames.cc \
	Version.cc \
	Net/FdSet.cc \
	Net/Socket.cc 

if HAVE_SHMEM
libcommon_la_LIBADD	= shmem/libshmem.la
endif

bin_PROGRAMS = versioncommon

versioncommon_SOURCES      = versioncommon.cc
versioncommon_LDADD        = libcommon.la
versioncommon_DEPENDENCIES = libcommon.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
