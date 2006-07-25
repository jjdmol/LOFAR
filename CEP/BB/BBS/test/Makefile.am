check_PROGRAMS = tFillRow tMeqPolc tMeqPolcLog tMeqTabular tMeqParm \
tParmMerge tPredSolv tPredict tSubtract tCorrect tPerturbed tMSData \
tShift

tMeqPolc_SOURCES = tMeqPolc.cc
tMeqPolc_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tMeqPolcLog_SOURCES = tMeqPolcLog.cc
tMeqPolcLog_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tMeqTabular_SOURCES = tMeqTabular.cc
tMeqTabular_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tFillRow_SOURCES = tFillRow.cc
tFillRow_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tMeqParm_SOURCES = tMeqParm.cc
tMeqParm_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tParmMerge_SOURCES = tParmMerge.cc
tParmMerge_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tPredSolv_SOURCES = tPredSolv.cc
tPredSolv_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tPredict_SOURCES = tPredict.cc
tPredict_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tSubtract_SOURCES = tSubtract.cc
tSubtract_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tCorrect_SOURCES = tCorrect.cc
tCorrect_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tPerturbed_SOURCES = tPerturbed.cc
tPerturbed_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tMSData_SOURCES = tMSData.cc
tMSData_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

tShift_SOURCES = tShift.cc
tShift_DEPENDENCIES = ../src/libbbs.la $(LOFAR_DEPEND)

LDADD				= ../src/libbbs.la 

TESTS_ENVIRONMENT		= lofar_sharedir=$(lofar_sharedir) \
				  prefix=$(prefix) \
				  srcdir=${srcdir} \
				  MPIBIN=$(MPIBIN) \
				  AIPSPP=$(AIPSPP)

TESTS = tFillRow.sh tMeqPolc.sh tMeqPolcLog.sh tMeqTabular.sh tMeqParm.sh \
tParmMerge.sh tMSData.sh tPredict.sh tPerturbed.sh tSubtract.sh tCorrect.sh \
tPredSolv.sh tBandpass.sh

# programs to run through supplied checktools
CHECKTOOLPROGS = tFillRow tMeqPolc tMeqPolcLog tMeqTabular tMeqParm tParmMerge \
tMSData tPredict tPerturbed tSubtract tCorrect
#ENDCHECKTOOLPROGS

include $(top_srcdir)/Makefile.common
