# if HAVE_SHMEM
# SUBDIRS = shmem .
# endif

INSTHDRS = \
	gnuplot_i.h \
	Marshalling.h \
	MarshallBlitz.h \
	Observer.h \
	PSAccess.h \
	Subject.h \
	TestSuite.h \
	Timestamp.h \
	daemonize.h \
	RegisterState.h \
	StringVector.h 

NOINSTHDRS =

TCCHDRS =

apldir = $(includedir)/APL/RTCCommon
apl_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

include $(top_srcdir)/Makefile.common
