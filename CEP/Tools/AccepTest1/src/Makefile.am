bin_PROGRAMS = Correlator FrontEnd

DOCHDRS			=				\
Socket.h						\
TH_Socket.h						\
Config.h						\
Correlator.h						\
FrontEnd.h						\
DH_Vis.h						\
DH_CorrCube.h						\
WH_Correlator.h						\
WH_Random.h						\
WH_Dump.h						\
DH_Empty.h						

SHAREDSRCS		=	$(DOCHDRS)		\
TestRange.h						\
TH_Socket.cc						\
Socket.cc						\
Config.cc						\
DH_Vis.cc						\
DH_CorrCube.cc						\
DH_Empty.cc						\
WH_Random.cc						\
WH_Correlator.cc					\
WH_Dump.cc						


Correlator_SOURCES	=	$(SHAREDSRCS)		\
Correlator.cc						

FrontEnd_SOURCES 	=	$(SHAREDSRCS)		\
FrontEnd.cc						

Correlator_LDADD = # /bgl/broekema/traceback/traceback.o
Correlator_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common
