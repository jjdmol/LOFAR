
DOCHDRS = arithmetric.h \
	atkernel_intern.h \
	atkernel.h \
	basic_parser.h \
	bp.h \
	bp_admin.h \
	bp_file.h \
	bp_log.h \
	codingstandard.h \
	driver_configuration.h \
	eventprocessor.h \
	eventreceiver.h \
	executor_lib.h \
	executor_master.h \
	expirator.h \
	general_lib.h \
	parser.h \
	parser_lib.h \
	scanner.h \
	stepper.h \
	stores.h \
	test_kernel.h \
	test_parser.h \
	teststuff.h

noinst_LTLIBRARIES 	= libtsek.la

libtsek_la_SOURCES = $(DOCHDRS) \
	atkernel.c \
	basic_parser.c \
	bp.c \
	bp_file.c \
	bp_log.c \
	driver_configuration.c \
	eventprocessor.c \
	eventreceiver.c \
	executor.c \
	executor_lib.c \
	executor_master.c \
	expirator.c \
	general_lib.c \
	parser.c \
	parser_arithmetric.c \
	parser_debug.c \
	parser_lib.c \
	scanner.c \
	stepper.c \
	test_kernel.c \
	test_parser.c 
	

libtsek_la_CFLAGS = \
	-DDEBUG_AUTOTEST \
	-D_NO_CONTROL_PANEL_ \
	-DARITHMETRIC \
	-I$(top_srcdir)/src/tseg \
	-I$(top_srcdir)/src/tsed \
	-I$(top_srcdir)/src/dbg

include $(top_srcdir)/Makefile.common
