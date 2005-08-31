INSTHDRS         	=\
RSPTimeStamp.h          \
DH_RSPSync.h            \
DH_Delay.h		\
DH_RSP.h                \
DH_FIR.h		\
DH_PPF.h		\
DH_CorrCube.h		\
DH_Vis.h		\
DH_VisArray.h		\
RectMatrix.h		\
Stub_FIR.h              \
Stub_Corr.h		\
Stub_RSP.h		\
Stub_Delay.h		

NOINSTHDRS 		=

TCCHDRS 		= RectMatrix.tcc

pkginclude_HEADERS 	= $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS 		= $(NOINSTHDRS)

DOCHDRS 		= $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES	= libtfc_interface.la

libtfc_interface_la_SOURCES  	= \
RSPTimeStamp.cc          	\
DH_RSPSync.cc                   \
DH_Delay.cc			\
DH_RSP.cc                	\
DH_FIR.cc			\
DH_PPF.cc			\
DH_CorrCube.cc			\
DH_Vis.cc			\
DH_VisArray.cc			\
Stub_FIR.cc               	\
Stub_RSP.cc			\
Stub_Corr.cc			\
Stub_Delay.cc			

configfilesdir=$(bindir)
configfiles_DATA=../../TFlopCorrelator.cfg

include $(lofar_sharedir)/Makefile.common
