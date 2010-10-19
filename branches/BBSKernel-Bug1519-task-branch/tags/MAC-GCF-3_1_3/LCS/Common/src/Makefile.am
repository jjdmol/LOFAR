SUBDIRS	= shmem .

INSTHDRS = \
Allocator.h BlobStringType.h BlobString.h BlobStringTraits.h \
BlobHeader.h BlobArrayHeader.h BlobOStream.h BlobIStream.h BlobArray.h \
BlobIBuffer.h BlobIBufStream.h BlobOBuffer.h BlobOBufStream.h \
BlobIBufChar.h BlobIBufVector.h BlobOBufChar.h BlobOBufVector.h \
BlobOBufString.h BlobIBufString.h BlobOBufNull.h \
BlobField.h BlobFieldSet.h BlobException.h \
LofarTypedefs.h LofarTypes.h TypeNames.h DataFormat.h DataConvert.h \
KeyValue.h KeyValueMap.h \
KeyParser.h \
Debug.h \
Exception.h \
Net/Socket.h \
Thread.h \
Thread/Thread.h \
Thread/Key.h \
Thread/Mutex.h \
Thread/Condition.h \
Thread/SmartLock.h \
Stopwatch.h \
StringUtil.h \
CheckConfig.h \
Lorrays.h Lorrays-Blitz.h blitz_array-impl.h BlitzToAips.h \
Lonumerics.h \
lofar_iostream.h				\
lofar_fstream.h					\
lofar_strstream.h				\
lofar_algorithm.h				\
lofar_deque.h					\
lofar_list.h					\
lofar_vector.h					\
lofar_complex.h					\
lofar_map.h					\
lofar_stack.h					\
lofar_iosfwd.h					\
lofar_string.h					\
Profiling/PerfProfile.h \
LofarLogger.h

NOINSTHDRS =

TCCHDRS = \
BlobArray.tcc TypeNames.tcc

pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libcommon.la

libcommon_la_SOURCES 	= \
Allocator.cc BlobStringType.cc BlobString.cc \
BlobHeader.cc BlobOStream.cc BlobIStream.cc BlobArray.cc \
BlobIBufStream.cc BlobOBufStream.cc BlobOBufString.cc BlobOBufNull.cc \
BlobIBufChar.cc BlobOBufChar.cc \
BlobField.cc BlobFieldSet.cc \
TypeNames.cc DataConvert.cc \
KeyValue.cc KeyValueMap.cc \
KeyParse.yy KeyTokenize.ll KeyParser.cc \
Debug.cc \
Exception.cc \
Stopwatch.cc \
StringUtil.cc \
Net/Socket.cc \
Thread/Thread.cc \
Thread/SmartLock.cc \
Profiling/PerfProfile.cc \
LofarLogger.cc

AM_YFLAGS		= -d -p KeyParse
AM_LFLAGS		= -PKeyTokenize
LEX_OUTPUT_ROOT		= lex.KeyTokenize

libcommon_la_LIBADD	= shmem/libshmem.la

bin_PROGRAMS		= LimitExec
LimitExec_SOURCES	= LimitExec.cc

include $(lofar_sharedir)/Makefile.common
