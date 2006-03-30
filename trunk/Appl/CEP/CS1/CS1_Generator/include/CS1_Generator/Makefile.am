# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

INSTHDRS   = 	StationData.h \
		DH_RSP.h \
		AH_FakeStation.h \
		AH_Recorder.h \
		WH_FakeStation.h \
		WH_Signal.h \
		WH_Strip.h \
		WH_Wrap.h


NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
