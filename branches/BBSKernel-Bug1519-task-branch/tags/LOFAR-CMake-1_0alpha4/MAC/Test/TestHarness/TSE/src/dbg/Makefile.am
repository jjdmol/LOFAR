noinst_LTLIBRARIES 	= libdbg.la

DOCHDRS = \
	dbgtrace.h

libdbg_la_SOURCES= $(DOCHDRS) \
	dbgtrace.c

libdbg_la_CFLAGS = \
	-DDEBUG_AUTOTEST \
	-D_NO_CONTROL_PANEL_ \
	-DARITHMETRIC

include $(top_srcdir)/Makefile.common
