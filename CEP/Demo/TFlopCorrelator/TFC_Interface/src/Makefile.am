INSTHDRS         	=\
DH_RSPInputSync.h       \
DH_Sync.h		\
DH_RSP.h                \
DH_FIR.h		\
DH_PPF.h		\
DH_CorrCube.h		\
DH_Vis.h		\
RSPTimeStamp.h          \
RectMatrix.h		\
Stub_SB.h               \
Stub_Corr.h		\
Stub_RSP.h		\
Stub_Delay.h

NOINSTHDRS 		=

TCCHDRS 		=

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES	= libtfc_interface.la

libtfc_interface_la_SOURCES  	= \
DH_RSPInputSync.cc              \
DH_Sync.cc			\
DH_RSP.cc                	\
DH_FIR.cc			\
DH_PPF.cc			\
DH_CorrCube.cc			\
DH_Vis.cc			\
RSPTimeStamp.cc          	\
Stub_SB.cc               	\
Stub_RSP.cc			\
Stub_Corr.cc			\
Stub_Delay.cc			\
RectMatrix.cc			

include $(lofar_sharedir)/Makefile.common
