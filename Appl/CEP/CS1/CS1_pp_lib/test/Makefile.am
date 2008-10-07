check_PROGRAMS        = test_flaggers

# programs to run through supplied checktools
CHECKTOOLPROGS        =
#ENDCHECKTOOLPROGS

# scripts used to run tests
TESTSCRIPTS           =

# scripts and possible programs to run tests
TESTS                 = $(TESTSCRIPTS)

XFAIL_TESTS           =

# all files (.run, .stdout, .in, .log_prop, etc.) needed to run tests
EXTRA_DIST            = $(TESTSCRIPTS) CS1_IDPPP.parset

# Lines to build a test program testprg
test_flaggers_SOURCES                = $(top_srcdir)/src/test_flaggers.cc \
																			$(top_srcdir)/src/MsInfo.h \
																			$(top_srcdir)/src/MsInfo.cc \
																			$(top_srcdir)/src/DataBuffer.h \
																			$(top_srcdir)/src/DataBuffer.cc \
																			$(top_srcdir)/src/RunDetails.h \
																			$(top_srcdir)/src/RunDetails.cc \
																			$(top_srcdir)/src/FlaggerStatistics.h \
																			$(top_srcdir)/src/FlaggerStatistics.cc \
																			$(top_srcdir)/src/MADFlagger.h \
																			$(top_srcdir)/src/MADFlagger.cc
test_flaggers_LDADD                  =
test_flaggers_DEPENDENCIES           = $(LOFAR_DEPEND)



include $(top_srcdir)/Makefile.common
