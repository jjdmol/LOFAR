SUBDIRS	= shmem .

DOCHDRS	=	\
Debug.h						\
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

lib_LTLIBRARIES 	= libcommon.la

libcommon_la_SOURCES 	= $(DOCHDRS) \
Debug.cc \
Exception.cc \
Stopwatch.cc \
Net/Socket.cc \
Thread/Thread.cc \
Thread/SmartLock.cc \
Profiling/PerfProfile.cc

libcommon_la_LIBADD	= shmem/libshmem.la

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = debug_methods.g

EXTRA_DIST = assay

bin_PROGRAMS		= LimitExec
LimitExec_SOURCES	= LimitExec.c

include $(lofar_sharedir)/Makefile.common
