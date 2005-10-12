INSTHDRS         	=\
RSPTimeStamp.h          \
DH_RSPSync.h            \
DH_Delay.h		\
DH_PhaseCorr.h		\
DH_RSP.h                \
DH_Subband.h            \
DH_PPF.h		\
DH_CorrCube.h		\
DH_Vis.h		\
DH_VisArray.h		\
RectMatrix.h		\
Stub_Corr.h		\
Stub_RSP.h		\
Stub_Delay.h		\
Stub_PhaseCorr.h	\
Stub_FIR.h		\
TFC_Config.h

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
DH_PhaseCorr.cc			\
DH_RSP.cc                	\
DH_Subband.cc                   \
DH_PPF.cc			\
DH_CorrCube.cc			\
DH_Vis.cc			\
DH_VisArray.cc			\
Stub_FIR.cc               	\
Stub_RSP.cc			\
Stub_Corr.cc			\
Stub_Delay.cc			\
Stub_PhaseCorr.cc

configfilesdir=$(bindir)
configfiles_DATA=../../TFlopCorrelator.cfg

include $(top_srcdir)/Makefile.common
