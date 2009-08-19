

pkginclude_HEADERS = Package__Version.h \
BeamletBuffer.h			\
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

IONProc_SOURCES			= $(liblofar_impl_la_SOURCES)
IONProc_DEPENDENCIES		= $(LOFAR_DEPEND)

bin_PROGRAMS			= versionionproc IONProc

versionionproc_SOURCES      = versionionproc.cc Package__Version.cc 
versionionproc_DEPENDENCIES = $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
