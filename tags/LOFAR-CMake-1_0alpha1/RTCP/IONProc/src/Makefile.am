

pkginclude_HEADERS = Package__Version.h \
BeamletBuffer.h			\
CN_Personality.h		\
InputSection.h			\
InputThread.h			\
InputThreadAsm.h		\
ION_Allocator.h			\
LockedRanges.h			\
LogThread.h			\
OutputThread.h			\
OutputSection.h			\
ReaderWriterSynchronization.h	\
RSP.h				\
Scheduling.h			\
SlidingPointer.h		\
WallClockTime.h			\
WH_DelayCompensation.h		\
FCNP_ServerStream.h

liblofar_impl_la_SOURCES = $(pkginclude_HEADERS) \
BeamletBuffer.cc		\
CN_Personality.cc		\
ION_main.cc		 	\
InputSection.cc			\
InputThread.cc			\
InputThreadAsm.S		\
ION_Allocator.cc		\
LogThread.cc			\
OutputThread.cc			\
OutputSection.cc		\
ReaderWriterSynchronization.cc	\
Scheduling.cc			\
WH_DelayCompensation.cc		\
FCNP_ServerStream.cc		\
Package__Version.cc

#if HAVE_ZOID
#lib_LTLIBRARIES			= liblofar_impl.la
#bin_PROGRAMS			= versionionproc
#
#install-exec-local:
#	ln -sf .libs/liblofar_impl.so.0.0.0 lofar_impl.so
#	cp lofar_impl.so /bgl/lofar-utils/zoid-test/lib
#
#uninstall-local:
#	rm lofar_impl.so
#else
IONProc_SOURCES			= $(liblofar_impl_la_SOURCES)
IONProc_DEPENDENCIES		= $(LOFAR_DEPEND)

bin_PROGRAMS			= versionionproc IONProc
#endif

versionionproc_SOURCES      = versionionproc.cc Package__Version.cc 
versionionproc_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
