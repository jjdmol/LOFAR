dmi_dir = $(lofar_sharedir)/../DMI

lib_LTLIBRARIES = libsolver.la

DOCHDRS = \
  BatchAgent.h   MeqCalSolver.h        \
  BaseSolver.h  DummySolver.h  \
  SolverControlAgent.h  TypeIter-Solver.h

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

include $(lofar_sharedir)/Makefile.common
include $(lofar_sharedir)/Makefile.aids


AM_CPPFLAGS = -DMAKE_LOFAR_SYMBOLS_GLOBAL
