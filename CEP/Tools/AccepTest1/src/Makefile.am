bin_PROGRAMS = Correlator FrontEnd

DOCHDRS			=				\
Config.h						\
Correlator.h						\
FrontEnd.h						\
DH_Vis.h						\
DH_CorrCube.h						\
WH_Correlator.h						\
WH_Random.h						\
WH_Dump.h						\
DH_Empty.h						

Correlator_SOURCES	=	$(DOCHDRS)		\
Config.cc						\
Correlator.cc						\
DH_Vis.cc						\
DH_CorrCube.cc						\
WH_Correlator.cc					\
WH_Random.cc						\
WH_Dump.cc						\
DH_Empty.cc						

FrontEnd_SOURCES 	=	$(DOCHDRS)		\
Config.cc						\
FrontEnd.cc						\
DH_Vis.cc						\
DH_CorrCube.cc						\
DH_Empty.cc						\
WH_Random.cc						\
WH_Correlator.cc					\
WH_Dump.cc						


Correlator_LDADD = -lmpich
Correlator_DEPENDENCIES = $(LOFAR_DEPEND)

include $(lofar_sharedir)/Makefile.common