dmi_dir = $(lofar_sharedir)/../DMI

lib_LTLIBRARIES = libsolver.la

libsolver_la_SOURCES = \
  AID-Solver-Registry.cc \
  BaseSolver.cc \
  DummySolver.cc \
  MeqCalSolver.cc \
  SolverControlAgent.cc \
  BatchAgent.cc
  
bin_PROGRAMS   		= applauncher
  
applauncher_SOURCES		    = applauncher.cc
applauncher_LDADD		      = libsolver.la -lglish -lsos -lnpd
applauncher_DEPENDENCIES	= libsolver.la $(LOFAR_DEPEND)

glishdir = $(libexecdir)/glish/pss3
dist_glish_SCRIPTS = glishsolver.g

# script used to generate AID maps
MAPBUILDER = $(dmi_dir)/src/build_aid_maps.pl 

# All .aidlist files below this dir will be scanned
BASELISTDIR = $(lofar_sharedir)/..

# Your package's subdirectory
AID_DIR = $(top_srcdir)/src


aids:
	$(MAPBUILDER) `find $(BASELISTDIR) -name \*.aidlist` $(AID_DIR)/*.h 
 

include $(lofar_sharedir)/Makefile.common

