dmi_dir = $(lofar_sharedir)/../DMI

CXXFLAGS += -D_GNU_SOURCE 

check_PROGRAMS		= test_solver test_dummy glishsolver

bin_PROGRAMS   		= 

# glishwriter

test_solver_SOURCES		    = test_solver.cc
test_solver_LDADD		      = ../src/libsolver.la -lglish -lsos -lnpd
test_solver_DEPENDENCIES	  = ../src/libsolver.la $(LOFAR_DEPEND)

test_dummy_SOURCES		    = test_dummy.cc
test_dummy_LDADD		      = ../src/libsolver.la -lglish -lsos -lnpd
test_dummy_DEPENDENCIES	  = ../src/libsolver.la $(LOFAR_DEPEND)

glishsolver_SOURCES		    = glishsolver.cc
glishsolver_LDADD		      = ../src/libsolver.la -lglish -lsos -lnpd
glishsolver_DEPENDENCIES	= ../src/libsolver.la $(LOFAR_DEPEND)

# glishwriter_SOURCES		    = glishwriter.cc
# glishwriter_LDADD		      = ../src/libsolver.la -lglish -lsos -lnpd
# glishwriter_DEPENDENCIES	= ../src/libsolver.la $(LOFAR_DEPEND)

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

AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
