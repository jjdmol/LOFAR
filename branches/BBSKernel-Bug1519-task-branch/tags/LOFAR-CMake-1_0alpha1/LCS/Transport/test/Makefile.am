AM_CPPFLAGS = \
	-DHAVE_MMAP=0 \
	-DMORECORE=shmbrk \
	-DMORECORE_CONTIGUOUS=0 \
	-DMORECORE_CANNOT_TRIM=1 \
	-DSHMEM_ALLOC

CHECKTOOLPROGS		  = Example \
			    TestBidirectional \
			    ExampleMem \
		    	    ExampleBlMem \
			    ExampleNonBlocking \
			    ExampleVarBuf \
			    ExampleShMem \
			    ExampleMPI \
			    ExampleSocket

check_PROGRAMS		  = Example \
			    TestBidirectional \
			    ExampleMem \
		    	    ExampleBlMem \
			    ExampleNonBlocking \
			    ExampleVarBuf \
			    ExampleShMem \
			    ExampleMPI \
			    ExampleSocket \
			    ExampleEthernet \
			    tServer tClient \
			    tCSConnection

Example_SOURCES 	  = Example.cc DH_Example.cc

ExampleMem_SOURCES 	  = ExampleMem.cc DH_Example.cc DH_ExampleExtra.cc

ExampleBlMem_SOURCES	  = ExampleBlMem.cc DH_Example.cc DH_ExampleExtra.cc

ExampleNonBlocking_SOURCES= ExampleNonBlocking.cc DH_Example.cc

ExampleVarBuf_SOURCES     = ExampleVarBuf.cc DH_VarBuf.cc

ExampleShMem_SOURCES 	  = ExampleShMem.cc DH_Example.cc DH_ExampleExtra.cc

ExampleMPI_SOURCES 	  = ExampleMPI.cc DH_Example.cc DH_ExampleExtra.cc

TestBidirectional_SOURCES = TestBidirectional.cc DH_Example.cc

ExampleSocket_SOURCES     = ExampleSocket.cc DH_Socket.cc

ExampleEthernet_SOURCES	  = ExampleEthernet.cc DH_Ethernet.cc

tServer_SOURCES		  = tServer.cc DH_Socket.cc

tClient_SOURCES		  = tClient.cc DH_Socket.cc

tCSConnection_SOURCES	  = tCSConnection.cc

LDADD			  = ../src/libtransport.la 
DEPENDENCIES		  = ../src/libtransport.la $(LOFAR_DEPEND)

TESTSCRIPTS = \
	Example.sh		\
	TestBidirectional.sh	\
	ExampleMem.sh		\
	ExampleBlMem.sh		\
	ExampleNonBlocking.sh	\
	ExampleShMem.sh		\
	ExampleMPI.sh		\
	ExampleVarBuf.sh	\
	ExampleSocket.sh

TESTS = $(TESTSCRIPTS)


EXTRA_DIST =				\
	DH_Ethernet.h DH_Example.h DH_ExampleExtra.h \
	DH_Socket.h DH_VarBuf.h StopWatch.h \
	ExampleEthernet.run ExampleMPI.run \
	ExampleShMem.run ExampleSocket.run \
	$(TESTSCRIPTS)

include $(top_srcdir)/Makefile.common
