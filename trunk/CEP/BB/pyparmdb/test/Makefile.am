check_PROGRAMS        = tParmFacade

# programs to run through supplied checktools
CHECKTOOLPROGS        = tParmFacade
#ENDCHECKTOOLPROGS

# scripts used to run tests
TESTSCRIPTS           = tParmFacade.sh

# scripts and possible programs to run tests
TESTS	              = $(TESTSCRIPTS)

XFAIL_TESTS           =

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST            = $(TESTSCRIPTS)

# Lines to build a test program testprg
tParmFacade_SOURCES		= tParmFacade.cc
tParmFacade_LDADD		= ../src/libparmfacade.la
tParmFacade_DEPENDENCIES	= ../src/libparmfacade.la $(LOFAR_DEPEND)



include $(top_srcdir)/Makefile.common
