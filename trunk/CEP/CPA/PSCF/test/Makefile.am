CXXFLAGS += -Wno-unknown-pragmas

INCLUDES			= -I$(top_srcdir)/src
check_PROGRAMS			= test_crefs

test_crefs_SOURCES 		= test_crefs.cc

test_crefs_LDADD		= $(top_builddir)/src/libpscf.la

TESTS				= test_crefs

EXTRA_DIST			=

include $(lofar_sharedir)/Makefile.common
