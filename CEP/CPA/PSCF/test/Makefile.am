CXXFLAGS += -Wno-unknown-pragmas

INCLUDES			= -I$(top_srcdir)/src -I$(top_srcdir)/src/pscf
check_PROGRAMS			= test_crefs test_pscf

test_crefs_SOURCES 		= test_crefs.cc

test_crefs_LDADD		= $(top_builddir)/src/libdmi.la

test_pscf_SOURCES 		= test_pscf.cc EchoWP.cc

test_pscf_LDADD		= $(top_builddir)/src/pscf/libpscf.la $(top_builddir)/src/libdmi.la 

TESTS				= test_crefs test_pscf

EXTRA_DIST			=

include $(lofar_sharedir)/Makefile.common
