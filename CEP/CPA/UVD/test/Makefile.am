CXXFLAGS += -Wno-unknown-pragmas -pg

check_PROGRAMS		= test_filler 

bin_PROGRAMS   		= test_filler test_msi test_sort

test_filler_SOURCES 	= test_filler.cc 

test_filler_LDADD		= $(top_builddir)/src/libuvd.la 

test_filler_DEPENDENCIES	= $(top_builddir)/src/libuvd.la $(LOFAR_DEPEND)

test_msi_SOURCES 	= test_msi.cc 

test_msi_LDADD		= $(top_builddir)/src/libuvd.la 

test_msi_DEPENDENCIES	= $(top_builddir)/src/libuvd.la $(LOFAR_DEPEND)

test_sort_SOURCES 	= test_sort.cc 

test_sort_LDADD		= $(top_builddir)/src/libuvd.la 

test_sort_DEPENDENCIES	= $(top_builddir)/src/libuvd.la $(LOFAR_DEPEND)

TESTS				=  test_filler

EXTRA_DIST			=

include $(lofar_sharedir)/Makefile.common
