if HAVE_SHMEM
SUBDIRS	= shmem .
endif

lib_LTLIBRARIES 	= libcommon.la

libcommon_la_SOURCES 	= \
        Allocator.cc \
        BlobArray.cc \
        BlobField2.cc \
        BlobFieldSet.cc \
        BlobHeader.cc \
        BlobIBufChar.cc \
        BlobIBufStream.cc \
        BlobIStream.cc \
        BlobOBufChar.cc \
        BlobOBufNull.cc \
        BlobOBufStream.cc \
        BlobOBufString.cc \
        BlobOStream.cc \
        BlobString.cc \
        BlobStringType.cc \
        DataConvert.cc \
        Debug.cc \
        Exception.cc \
        hexdump.cc \
        KeyParse.yy \
        KeyTokenize.ll \
        KeyParser.cc \
        KeyValue.cc \
        KeyValueMap.cc \
        LofarLogCout.cc \
        LofarLogger.cc \
	Numeric.cc \
        RunOnNode.cc \
        Stopwatch.cc \
        StringUtil.cc \
	Timer.cc \
        TypeNames.cc \
        Net/FdSet.cc \
        Net/Socket.cc \
        Profiling/PerfProfile.cc \
        Thread/SmartLock.cc \
        Thread/Thread.cc 

EXTRA_DIST = backward/Debug.cc

AM_YFLAGS		= -d -p KeyParse
AM_LFLAGS		= -PKeyTokenize
LEX_OUTPUT_ROOT		= lex.KeyTokenize

if HAVE_SHMEM
libcommon_la_LIBADD	= shmem/libshmem.la
endif

bin_PROGRAMS		= pkgdep
pkgdep_SOURCES		= pkgdep.cc

include $(top_srcdir)/Makefile.common
