noinst_LTLIBRARIES 	= libtsed.la

DOCHDRS = \
	atdriver.h \
	atd_socket.h \
	devicelist.h \
	protocollist.h \
	receivebuffer.h

libtsed_la_SOURCES= $(DOCHDRS) \
	atdriver.c \
	atd_socket_linux.c \
	devicelist.c \
	protocollist.c \
	receivebuffer.c

libtsed_la_CFLAGS = \
	-DDEBUG_AUTOTEST \
	-D_NO_CONTROL_PANEL_ \
	-DARITHMETRIC \
	-I$(top_srcdir)/src/tseg \
	-I$(top_srcdir)/src/tsek \
	-I$(top_srcdir)/src/dbg

include $(top_srcdir)/Makefile.common
