check_PROGRAMS	      =	tDFTServer tmul

# programs to run through supplied checktools
CHECKTOOLPROGS 	      =	tDFTServer

TESTS	= tDFTServer_test.sh

tmul_SOURCES		= tmul.cc

tDFTServer_SOURCES		= tDFTServer.cc
tDFTServer_LDADD		= ../src/libdftserver.la
tDFTServer_DEPENDENCIES		= ../src/libdftserver.la $(LOFAR_DEPEND)

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir)


include $(lofar_sharedir)/Makefile.common
