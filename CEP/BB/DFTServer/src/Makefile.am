bin_PROGRAMS    = DFTServer

DOCHDRS         =                               \
DH_DFTRequest.h					\
DH_DFTResult.h					\
WH_DFTServer.h                                  \
DFTServer_ClientStub.h				\
DFTServer_ServerStub.h				\
DFTServer.h	                                


DFTServer_SOURCES          = $(DOCHDRS)                \
DH_DFTRequest.cc					\
DH_DFTResult.cc					\
WH_DFTServer.cc                                  \
DFTServer.cc	                                \
DFTServer_Main.cc


DFTServer_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common

