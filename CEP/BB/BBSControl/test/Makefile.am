check_PROGRAMS			= tBlobAipsIO \
				  tBBSStrategy

# programs to run through supplied checktools
CHECKTOOLPROGS			=
#ENDCHECKTOOLPROGS

TESTSCRIPTS			= tBlobAipsIO.sh \
				  tBBSStrategy.sh

TESTS				= $(TESTSCRIPTS)

EXTRA_DIST			= $(TESTSCRIPTS)

tBlobAipsIO_SOURCES		= tBlobAipsIO.cc
tBlobAipsIO_LDADD		= ../src/libbbscontrol.la
tBlobAipsIO_DEPENDENCIES	= ../src/libbbscontrol.la $(LOFAR_DEPEND)

tBBSStrategy_SOURCES		= tBBSStrategy.cc
tBBSStrategy_LDADD		= ../src/libbbscontrol.la
tBBSStrategy_DEPENDENCIES	= ../src/libbbscontrol.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  LOFARROOT=$(LOFARROOT)

include $(top_srcdir)/Makefile.common
