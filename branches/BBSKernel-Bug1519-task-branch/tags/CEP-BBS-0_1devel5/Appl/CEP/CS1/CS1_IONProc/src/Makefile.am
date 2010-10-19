pkginclude_HEADERS = Package__Version.h \
BeamletBuffer.h			\
BGL_Personality.h		\
InputSection.h			\
InputThread.h			\
InputThreadAsm.h		\
ION_Allocator.h			\
LockedRanges.h			\
LogThread.h			\
OutputSection.h			\
ReaderWriterSynchronization.h	\
RSP.h				\
SlidingPointer.h		\
WallClockTime.h			\
WH_DelayCompensation.h		\
FCNP_ServerStream.h

liblofar_impl_la_SOURCES = $(pkginclude_HEADERS) \
BeamletBuffer.cc		\
BGL_Personality.cc		\
CS1_ION_main.cc		 	\
InputSection.cc			\
InputThread.cc			\
InputThreadAsm.S		\
ION_Allocator.cc		\
LogThread.cc			\
OutputSection.cc		\
ReaderWriterSynchronization.cc	\
WH_DelayCompensation.cc		\
FCNP_ServerStream.cc		\
Package__Version.cc

#if HAVE_ZOID
#lib_LTLIBRARIES			= liblofar_impl.la
#bin_PROGRAMS			= versioncs1_ionproc
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

bin_PROGRAMS			= versioncs1_ionproc IONProc
#endif

versioncs1_ionproc_SOURCES      = versioncs1_ionproc.cc Package__Version.cc 
versioncs1_ionproc_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
