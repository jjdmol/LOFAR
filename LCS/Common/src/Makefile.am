SUBDIRS	= shmem .

INSTHDRS = \
Allocator.h BlobStringType.h BlobString.h \
BlobHeader.h BlobArrayHeader.h BlobOStream.h BlobIStream.h BlobArray.h \
BlobIBuffer.h BlobIBufStream.h BlobOBuffer.h BlobOBufStream.h \
BlobIBufChar.h BlobIBufVector.h BlobOBufChar.h BlobOBufVector.h \
LofarTypedefs.h LofarTypes.h TypeNames.h DataFormat.h DataConvert.h \
Debug.h \
Exception.h \
Net/Socket.h \
Thread/Thread.h \
Thread/Key.h \
Thread/Mutex.h \
Thread/Condition.h \
Thread/SmartLock.h \
Stopwatch.h \
CheckConfig.h \
Lorrays.h \
Lorrays-Blitz.h \
Lonumerics.h \
lofar_iostream.h				\
lofar_strstream.h				\
lofar_algorithm.h				\
lofar_list.h					\
lofar_vector.h					\
lofar_complex.h					\
lofar_map.h					\
lofar_fstream.h					\
lofar_stack.h					\
lofar_iosfwd.h					\
lofar_string.h					\
Profiling/PerfProfile.h

NOINSTHDRS =

TCCHDRS = \
BlobHeader.tcc BlobArray.tcc TypeNames.tcc

include_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS	= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES 	= libcommon.la

libcommon_la_SOURCES 	= \
Allocator.cc BlobStringType.cc BlobString.cc \
BlobOStream.cc BlobIStream.cc BlobArray.cc \
BlobIBufStream.cc BlobOBufStream.cc BlobOBufString.cc \
BlobIBufChar.cc BlobOBufChar.cc \
TypeNames.cc DataConvert.cc \
Debug.cc \
Exception.cc \
Stopwatch.cc \
Net/Socket.cc \
Thread/Thread.cc \
Thread/SmartLock.cc \
Profiling/PerfProfile.cc

libcommon_la_LIBADD	= shmem/libshmem.la

EXTRA_DIST = assay

bin_PROGRAMS		= LimitExec
LimitExec_SOURCES	= LimitExec.c

include $(lofar_sharedir)/Makefile.common
