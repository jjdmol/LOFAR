INSTHDRS = \
	AllRegisterState.h \
	BeamletWeights.h \
	BypassSettings.h \
	HBASettings.h \
	MEPHeader.h \
	MEPData.h \
	RCUSettings.h \
	RSUSettings.h \
	SPUStatus.h \
	Statistics.h \
	SubbandSelection.h \
	SystemStatus.h \
	TBBSettings.h \
	TDStatus.h \
	Versions.h \
	WGSettings.h \
	XCStatistics.h

NOINSTHDRS =

TCCHDRS =

pkgincludedir = $(includedir)/APL/RSP_Protocol
pkginclude_HEADERS = Package__Version.h  $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
