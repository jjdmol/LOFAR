SUBDIRS	= shmem .

INSTHDRS = \
        Allocator.h \
        BlitzToAips.h \
        BlobArray.h \
        BlobArrayHeader.h \
        BlobException.h \
        BlobField.h \
        BlobFieldSet.h \
        BlobHeader.h \
        BlobIBufChar.h \
        BlobIBuffer.h \
        BlobIBufStream.h \
        BlobIBufString.h \
        BlobIBufVector.h \
        BlobIStream.h \
        BlobOBufChar.h \
        BlobOBuffer.h \
        BlobOBufNull.h \
        BlobOBufStream.h \
        BlobOBufString.h \
        BlobOBufVector.h \
        BlobOStream.h \
        BlobString.h \
        BlobStringTraits.h \
        BlobStringType.h \
        CheckConfig.h \
        DataConvert.h \
        DataFormat.h \
        Debug.h \
        Exception.h \
        hexdump.h \
        KeyParser.h \
        KeyValue.h \
        KeyValueMap.h \
        lofar_algorithm.h \
        lofar_complex.h \
        lofar_deque.h \
        lofar_fstream.h \
        lofar_global_symbol_warning.h \
        lofar_iosfwd.h \
        lofar_iostream.h \
        lofar_list.h \
        LofarLog4Cplus.h \
        LofarLogCout.h \
        LofarLogger.h \
        lofar_map.h \
        lofar_sstream.h \
        lofar_stack.h \
        lofar_string.h \
        lofar_strstream.h \
        LofarTypedefs.h \
        LofarTypes.h \
        lofar_vector.h \
        Lonumerics.h \
        Lorrays-Blitz.h \
        Lorrays.h \
        RunOnNode.h \
        Stopwatch.h \
        StringUtil.h \
        Thread.h \
        TypeNames.h \
        Net/Socket.h \
        Profiling/PerfProfile.h \
        Thread/Condition.h \
        Thread/Key.h \
        Thread/Mutex.h \
        Thread/SmartLock.h \
        Thread/Thread.h

NOINSTHDRS =

TCCHDRS = \
	BlobField.cc \
	BlobArray.tcc \
	TypeNames.tcc

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libcommon.la

libcommon_la_SOURCES 	= \
        Allocator.cc \
        BlobArray.cc \
        BlobField2.cc \
        BlobField.cc \
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
        KeyParser.cc \
        KeyParse.yy \
        KeyTokenize.ll \
        KeyValue.cc \
        KeyValueMap.cc \
        LofarLogCout.cc \
        LofarLogger.cc \
        RunOnNode.cc \
        Stopwatch.cc \
        StringUtil.cc \
        TypeNames.cc \
        Net/Socket.cc \
        Profiling/PerfProfile.cc \
        Thread/SmartLock.cc \
        Thread/Thread.cc

AM_YFLAGS		= -d -p KeyParse
AM_LFLAGS		= -PKeyTokenize
LEX_OUTPUT_ROOT		= lex.KeyTokenize

libcommon_la_LIBADD	= shmem/libshmem.la

bin_PROGRAMS		= LimitExec
LimitExec_SOURCES	= LimitExec.cc

include $(lofar_sharedir)/Makefile.common
