bin_PROGRAMS = Correlator FrontEnd BackEnd

DOCHDRS			=				\
Config.h						\
Correlator.h						\
AH_FrontEnd.h						\
AH_BackEnd.h						\
DH_Vis.h						\
DH_CorrCube.h						\
WH_Correlator.h						\
WH_Random.h						\
WH_Dump.h						\
DH_Empty.h						


#Socket.h						\
#TH_Socket.h						\
#

SHAREDSRCS		=	$(DOCHDRS)		\
TestRange.h						\
Config.cc						\
DH_Vis.cc						\
DH_CorrCube.cc						\
DH_Empty.cc						\
WH_Random.cc						\
WH_Correlator.cc					\
WH_Dump.cc						

#TH_Socket.cc						\
#Socket.cc						\
#

Correlator_SOURCES	=	$(SHAREDSRCS)		\
CorrelatorMain.cc					\
AH_Correlator.cc					

FrontEnd_SOURCES 	=	$(SHAREDSRCS)		\
FrontEndMain.cc						\
AH_FrontEnd.cc

BackEnd_SOURCES 	=	$(SHAREDSRCS)		\
BackEndMain.cc						\
AH_BackEnd.cc

Correlator_LDADD = # /bgl/broekema/traceback/traceback.o
Correlator_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
