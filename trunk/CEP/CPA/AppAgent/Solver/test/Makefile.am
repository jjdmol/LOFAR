dmi_dir = $(lofar_sharedir)/../DMI

CXXFLAGS += -D_GNU_SOURCE 

check_PROGRAMS		= test_solver

bin_PROGRAMS   		= 

test_solver_SOURCES		    = test_solver.cc
test_solver_LDADD		      = ../src/libsolver.la
test_solver_DEPENDENCIES	  = ../src/libsolver.la $(LOFAR_DEPEND)

TESTS				=  

EXTRA_DIST			=

# script used to generate AID maps
MAPBUILDER = $(dmi_dir)/src/build_aid_maps.pl 

# All .aidlist files below this dir will be scanned
BASELISTDIR = $(lofar_sharedir)/..

# Your package's subdirectory
AID_DIR = $(top_srcdir)/test

aids:
#	$(MAPBUILDER) `find $(BASELISTDIR) -name \*.aidlist` $(AID_DIR)/*.h

include $(lofar_sharedir)/Makefile.common
