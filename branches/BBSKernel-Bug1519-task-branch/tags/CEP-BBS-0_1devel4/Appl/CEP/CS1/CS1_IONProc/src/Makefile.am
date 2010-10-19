pkginclude_HEADERS = Package__Version.h

lib_LTLIBRARIES		= liblofar_impl.la

liblofar_impl_la_SOURCES = Package__Version.cc  $(DOCHDRS) \
BeamletBuffer.h			\
BeamletBuffer.cc		\
BGL_Personality.h		\
BGL_Personality.cc		\
CS1_ION_main.h		 	\
CS1_ION_main.cc		 	\
Connector.h			\
Connector.cc			\
InputThread.h			\
InputThread.cc			\
ION_Allocator.h			\
ION_Allocator.cc		\
LockedRanges.h			\
ReaderWriterSynchronization.h	\
ReaderWriterSynchronization.cc	\
SlidingPointer.h		\
WallClockTime.h			\
AH_InputSection.h		\
AH_InputSection.cc		\
WH_InputSection.h		\
WH_InputSection.cc		\
AH_ION_Gather.h			\
AH_ION_Gather.cc		\
WH_ION_Gather.h			\
WH_ION_Gather.cc		\
TH_ZoidServer.h			\
TH_ZoidServer.cc

install-exec-local:
	ln -sf .libs/liblofar_impl.so.0.0.0 lofar_impl.so
	cp lofar_impl.so /bgl/lofar-utils/zoid-test/lib

uninstall-local:
	rm lofar_impl.so

bin_PROGRAMS = versioncs1_ionproc

versioncs1_ionproc_SOURCES      = versioncs1_ionproc.cc
versioncs1_ionproc_LDADD        = liblofar_impl.la
versioncs1_ionproc_DEPENDENCIES = liblofar_impl.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
