bin_PROGRAMS = Correlator FrontEnd BackEnd BasicFE BasicCorr BasicBE

DOCHDRS			=				\
Config.h						\
Correlator.h						\
AH_FrontEnd.h						\
AH_BackEnd.h						\
DH_Vis.h						\
DH_CorrCube.h						\
WH_Correlator.h						\
WH_Random.h						\
WH_Dump.h						

SHAREDSRCS		=	$(DOCHDRS)		\
TestRange.h						\
DH_Vis.cc						\
DH_CorrCube.cc						\
WH_Random.cc						\
WH_Correlator.cc					\
WH_Dump.cc						

Correlator_SOURCES	=	$(SHAREDSRCS)		\
CorrelatorMain.cc					\
AH_Correlator.cc					

FrontEnd_SOURCES 	=	$(SHAREDSRCS)		\
FrontEndMain.cc						\
AH_FrontEnd.cc

BackEnd_SOURCES 	=	$(SHAREDSRCS)		\
BackEndMain.cc						\
AH_BackEnd.cc

BasicFE_SOURCES 	=                     	        \
BasicFE.cc						

BasicCorr_SOURCES 	=	          		\
BasicCorrelator.cc       

BasicBE_SOURCES 	=	          		\
BasicBE.cc						

Correlator_LDADD = # /bgl/broekema/traceback/traceback.o
Correlator_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
