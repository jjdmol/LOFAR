CXXFLAGS += -Wno-unknown-pragmas

INCLUDES			= -I$(top_srcdir)/src -I$(top_srcdir)/src/pscf

check_PROGRAMS			= test_crefs test_pscf

bin_PROGRAMS   =  test_gw

test_crefs_SOURCES 		= test_crefs.cc

test_crefs_LDADD		= $(top_builddir)/src/libdmi.la

test_pscf_SOURCES 		= test_pscf.cc EchoWP.cc AID-Testing-Registry.cc

test_pscf_LDADD		= $(top_builddir)/src/pscf/libpscf.la \
$(top_builddir)/src/libdmi.la 

test_gw_SOURCES 		= test_gw.cc EchoWP.cc AID-Testing-Registry.cc

test_gw_LDADD		= $(top_builddir)/src/pscf/libpscf.la \
$(top_builddir)/src/libdmi.la 

TESTS				= test_crefs test_pscf

EXTRA_DIST			=

# script used to generate AID maps
MAPBUILDER = $(top_srcdir)/src/build_aid_maps.pl 

# All .aidlist files below this dir will be scanned
BASELISTDIR = $(lofar_sharedir)/..

# Your package's subdirectory
AID_DIR = $(top_srcdir)/test

ids:
	$(MAPBUILDER) `find $(BASELISTDIR) -name \*.aidlist` $(AID_DIR)/*.h


include $(lofar_sharedir)/Makefile.common
