#check_PROGRAMS = tFillRow tMeqPolc tMeqPolcLog tMeqTabular \
#tMeqParm tParmMerge tPredSolv tPredict tSubtract tCorrect tPerturbed tMSData \
#tShift tDFT

check_PROGRAMS = tFillRow tMeqPolc tMeqPolcLog tMeqTabular \
tMeqParm tShift tDFT

tFillRow_SOURCES = tFillRow.cc
tFillRow_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

tMeqPolc_SOURCES = tMeqPolc.cc
tMeqPolc_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

tMeqPolcLog_SOURCES = tMeqPolcLog.cc
tMeqPolcLog_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

tMeqTabular_SOURCES = tMeqTabular.cc
tMeqTabular_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

tMeqParm_SOURCES = tMeqParm.cc
tMeqParm_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

#tParmMerge_SOURCES = tParmMerge.cc
#tParmMerge_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

#tPredSolv_SOURCES = tPredSolv.cc
#tPredSolv_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

#tPredict_SOURCES = tPredict.cc
#tPredict_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

#tSubtract_SOURCES = tSubtract.cc
#tSubtract_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

#tCorrect_SOURCES = tCorrect.cc
#tCorrect_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

#tPerturbed_SOURCES = tPerturbed.cc
#tPerturbed_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

#tMSData_SOURCES = tMSData.cc
#tMSData_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

tShift_SOURCES = tShift.cc
tShift_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

tDFT_SOURCES = tDFT.cc
tDFT_DEPENDENCIES = ../src/libbbskernel.la $(LOFAR_DEPEND)

LDADD				= ../src/libbbskernel.la 

#TESTS = tFillRow.sh tMeqPolc.sh tMeqPolcLog.sh tMeqTabular.sh \
#tMeqParm.sh tParmMerge.sh tMSData.sh tPredict.sh tPerturbed.sh tSubtract.sh \
#tCorrect.sh tPredSolv.sh tBandpass.sh

#EXTRA_DIST = $(TESTS) tMeqParm.stdout tMeqPolcLog.stdout \
#tMeqPolc.stdout tParmMerge.stdout tPredSolv.stdout t3C343.run tBandpass.run \
#tCorrect.run tMeqPolcLog.run tMSData.run tParmMerge.run tPerturbed.run \
#tPredict.run tPredSolv.run tSubtract.run makeDemoParmDB makeDemoParmDBCorr \
#makeDemoParmDBPred tParmMerge.in_mep1 tParmMerge.in_mep2

# Currently, these tests fail due to floating point discrepancies. It should
# be investigated how this can be solved.
#XFAIL_TESTS = tPredSolv.sh tBandpass.sh

# programs to run through supplied checktools
#CHECKTOOLPROGS = tFillRow tMeqPolc tMeqPolcLog tMeqTabular \
#tMeqParm tParmMerge tMSData tPredict tPerturbed tSubtract tCorrect

TESTS = tFillRow.sh tMeqPolc.sh tMeqPolcLog.sh tMeqTabular.sh \
tMeqParm.sh

EXTRA_DIST = $(TESTS) tMeqPolc.stdout tMeqPolcLog.stdout \
tMeqPolcLog.run tMeqParm.stdout

CHECKTOOLPROGS = tFillRow tMeqPolc tMeqPolcLog tMeqTabular \
tMeqParm

include $(top_srcdir)/Makefile.common
