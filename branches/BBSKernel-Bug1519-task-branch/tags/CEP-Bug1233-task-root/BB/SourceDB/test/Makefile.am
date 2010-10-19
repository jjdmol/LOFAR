check_PROGRAMS        = tSourceDBStress

# programs to run through supplied checktools
CHECKTOOLPROGS        =

# scripts used to run tests
TESTSCRIPTS           = tsourcedb.sh tSourceDBStress.sh

# scripts and possible programs to run tests
TESTS	              = $(TESTSCRIPTS)

XFAIL_TESTS           =

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST            = $(TESTSCRIPTS) \
			tsourcedb.run tsourcedb.in tsourcedb.stdout \
			tSourceDBStress.run

tSourceDBStress_SOURCES		= tSourceDBStress.cc
tSourceDBStress_LDADD		= ../src/libsourcedb.la
tSourceDBStress_DEPENDENCIES	= ../src/libsourcedb.la $(LOFAR_DEPEND)


include $(top_srcdir)/Makefile.common
