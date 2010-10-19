check_PROGRAMS        = tAH_TestStorage

# programs to run through supplied checktools
CHECKTOOLPROGS        =
#ENDCHECKTOOLPROGS

# scripts used to run tests
TESTSCRIPTS           =

# scripts and possible programs to run tests
TESTS	              = $(TESTSCRIPTS)

XFAIL_TESTS           =

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST            = $(TESTSCRIPTS)

tAH_TestStorage_SOURCES		= tAH_TestStorage.cc 
tAH_TestStorage_LDADD		= ../src/libstorage.la
tAH_TestStorage_DEPENDENCIES	= ../src/libstorage.la $(LOFAR_DEPEND)



include $(top_srcdir)/Makefile.common
