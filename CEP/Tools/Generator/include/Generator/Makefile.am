# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

INSTHDRS           = Signals.h \
	 RSPTimeStamp.h \
	 WH_FakeStation.h \
	 WH_Signal.h \
	 WH_Wrap.h \
	 WH_Strip.h \
	 AH_Recorder.h \
	 AH_Generator.h \
	 DH_RSP.h \
	 RSPEthFrame.h

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
