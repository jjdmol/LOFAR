pkginclude_HEADERS = Package__Version.h \
BeamletBuffer.h			\
BGL_Personality.h		\
Connector.h			\
InputThread.h			\
ION_Allocator.h			\
LockedRanges.h			\
ReaderWriterSynchronization.h	\
SlidingPointer.h		\
WallClockTime.h			\
InputSection.h			\
WH_DelayCompensation.h		\
AH_ION_Gather.h			\
WH_ION_Gather.h			\
TH_ZoidServer.h	

liblofar_impl_la_SOURCES = $(pkginclude_HEADERS) \
BeamletBuffer.cc		\
BGL_Personality.cc		\
CS1_ION_main.cc		 	\
Connector.cc			\
InputThread.cc			\
ION_Allocator.cc		\
ReaderWriterSynchronization.cc	\
InputSection.cc			\
WH_DelayCompensation.cc		\
AH_ION_Gather.cc		\
WH_ION_Gather.cc		\
TH_ZoidServer.cc                \
Package__Version.cc

#if HAVE_ZOID
lib_LTLIBRARIES			= liblofar_impl.la
bin_PROGRAMS			= versioncs1_ionproc

install-exec-local:
	ln -sf .libs/liblofar_impl.so.0.0.0 lofar_impl.so
	cp lofar_impl.so /bgl/lofar-utils/zoid-test/lib

uninstall-local:
	rm lofar_impl.so
#else
#IONProc_SOURCES			= $(liblofar_impl_la_SOURCES)
#IONProc_DEPENDENCIES		= $(LOFAR_DEPEND)

#bin_PROGRAMS			= versioncs1_ionproc IONProc
#endif

versioncs1_ionproc_SOURCES      = versioncs1_ionproc.cc Package__Version.cc 
versioncs1_ionproc_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
