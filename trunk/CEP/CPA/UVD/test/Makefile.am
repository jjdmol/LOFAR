CXXFLAGS += -Wno-unknown-pragmas -pg

check_PROGRAMS		= test_filler

bin_PROGRAMS   		= test_filler

test_filler_SOURCES 	= test_filler.cc 

test_filler_LDADD		= $(top_builddir)/src/libfillers.la 

TESTS				=  test_filler

EXTRA_DIST			=

include $(lofar_sharedir)/Makefile.common
