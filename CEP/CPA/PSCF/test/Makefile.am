CXXFLAGS += -Wno-unknown-pragmas -pg

dmi_dir = $(lofar_sharedir)/../DMI

dmi_lib = $(dmi_dir)/build/src/libdmi.la

INCLUDES		= -I$(dmi_dir)/src -I$(top_srcdir)/src -I$(lofar_sharedir)/../src-links


check_PROGRAMS		= test_pscf

bin_PROGRAMS   		= test_gw test_gw0 test_ping test_glish

test_pscf_SOURCES 	= test_pscf.cc EchoWP.cc AID-Testing-Registry.cc

test_pscf_LDADD		= $(top_builddir)/src/liboctopussy.la $(dmi_lib)

test_gw_SOURCES 	= test_gw.cc EchoWP.cc AID-Testing-Registry.cc

test_gw_LDADD		= $(top_builddir)/src/liboctopussy.la $(dmi_lib)

test_gw0_SOURCES 	= test_gw0.cc AID-Testing-Registry.cc

test_gw0_LDADD		= $(top_builddir)/src/liboctopussy.la $(dmi_lib)

test_ping_SOURCES 	= test_ping.cc EchoWP.cc AID-Testing-Registry.cc

test_ping_LDADD		= $(top_builddir)/src/liboctopussy.la $(dmi_lib)

test_glish_SOURCES 	= test_glish.cc EchoWP.cc AID-Testing-Registry.cc

test_glish_LDADD		= $(top_builddir)/src/Glish/liboctopussy-glish.la \
  $(top_builddir)/src/liboctopussy.la $(dmi_lib) -lglish -lsos -lnpd

TESTS				=  test_pscf

EXTRA_DIST			=

# script used to generate AID maps
MAPBUILDER = $(dmi_dir)/src/build_aid_maps.pl 

# All .aidlist files below this dir will be scanned
BASELISTDIR = $(lofar_sharedir)/..

# Your package's subdirectory
AID_DIR = $(top_srcdir)/test

aids:
	$(MAPBUILDER) `find $(BASELISTDIR) -name \*.aidlist` $(AID_DIR)/*.h

include $(lofar_sharedir)/Makefile.common
