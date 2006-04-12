# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

INSTHDRS = \
	MEPHeader.h \
	MEPData.h \
	BeamletWeights.h \
	Clocks.h \
	RCUSettings.h \
	Statistics.h \
	SubbandSelection.h \
	SystemStatus.h \
	Versions.h \
	WGSettings.h \
	XCStatistics.h \
	MEPHeader.h \
	MEPData.h

NOINSTHDRS =

TCCHDRS =

#nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

apldir = $(includedir)/APL/RSP_Protocol
apl_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
