check_PROGRAMS	      =	tDFTServer

# programs to run through supplied checktools
CHECKTOOLPROGS 	      =	tDFTServer

TESTS	= tDFTServer_test.sh

tDFTServer_SOURCES		= tDFTServer.cc
tDFTServer_LDADD		= ../src/libdftserver.la
tDFTServer_DEPENDENCIES		= ../src/libdftserver.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir)


include $(lofar_sharedir)/Makefile.common
