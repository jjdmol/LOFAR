INSTHDRS         	=\
DH_Sync.h		\
DH_RSP.h                \
DH_SubBand.h		\
DH_PPF.h		\
DH_CorrCube.h		\
DH_Vis.h		\
RSPTimeStamp.h          \
RectMatrix.h		\
Stub_SB.h               \
Stub_Corr.h		\
Stub_RSP.h

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES	= libtfc_interface.la

libtfc_interface_la_SOURCES  	= \
DH_Sync.cc			\
DH_RSP.cc                	\
DH_SubBand.cc			\
DH_PPF.cc			\
DH_CorrCube.cc			\
DH_Vis.cc			\
RSPTimeStamp.cc          	\
Stub_SB.cc               	\
Stub_RSP.cc			\
Stub_Corr.cc			\
RectMatrix.cc			

include $(lofar_sharedir)/Makefile.common
