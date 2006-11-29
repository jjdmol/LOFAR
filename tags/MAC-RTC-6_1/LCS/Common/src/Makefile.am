if HAVE_SHMEM
SUBDIRS	= shmem .
endif

lib_LTLIBRARIES 	= libcommon.la

libcommon_la_SOURCES 	= \
	Allocator.cc \
	DataConvert.cc \
	Exception.cc \
	FileLocator.cc \
	hexdump.cc \
	lofar_bitset.cc \
	LofarLogCout.cc \
	LofarLogger.cc \
	Numeric.cc \
	Process.cc \
	RunOnNode.cc \
	Stopwatch.cc \
	StreamUtil.cc \
	StringUtil.cc \
	Timer.cc \
	TypeNames.cc \
	Net/FdSet.cc \
	Net/Socket.cc 

if HAVE_SHMEM
libcommon_la_LIBADD	= shmem/libshmem.la
endif

include $(top_srcdir)/Makefile.common
