lib_LTLIBRARIES = libbbskernel.la

#	MNS/ExternalFunction.cc \
#	MNS/MeqAzEl.cc \
#	MNS/MeqDiag.cc \
#	MNS/MeqDipoleBeam.cc \
#	MNS/MeqDipoleBeamExternal.cc \
#	MNS/MeqGaussianCoherency.cc \
#	MNS/MeqGaussianSource.cc \
#	MNS/MeqJonesCMul2.cc \
#	MNS/MeqJonesMul3.cc \
#	MNS/MeqNumericalDipoleBeam.cc

libbbskernel_la_SOURCES = Package__Version.cc \
	BlobIOExtensions.cc \
	Evaluator.cc \
	Equator.cc \
	Instrument.cc \
	MeasurementAIPS.cc \
	MetaMeasurement.cc \
	Model.cc \
	ModelConfig.cc \
	ParmManager.cc \
	ParmProxy.cc \
	Solver.cc \
	SolverInterfaceTypes.cc \
	VisData.cc \
	VisDimensions.cc \
	VisSelection.cc \
	MNS/ExprParm.cc \
	MNS/MeqDFTPS.cc \
	MNS/MeqExpr.cc \
	MNS/MeqLMN.cc \
	MNS/MeqJonesCMul3.cc \
	MNS/MeqJonesExpr.cc \
	MNS/MeqJonesInvert.cc \
	MNS/MeqJonesMul.cc \
	MNS/MeqJonesMul2.cc \
	MNS/MeqJonesNode.cc \
	MNS/MeqJonesSum.cc \
	MNS/MeqJonesVisData.cc \
	MNS/MeqMatrix.cc \
	MNS/MeqMatrixComplexArr.cc \
	MNS/MeqMatrixComplexSca.cc \
	MNS/MeqMatrixRealArr.cc \
	MNS/MeqMatrixRealSca.cc \
	MNS/MeqMatrixRep.cc \
	MNS/MeqMatrixTmp.cc \
	MNS/MeqPhaseRef.cc \
    MNS/MeqPhaseShift.cc \
	MNS/MeqPointCoherency.cc \
	MNS/MeqPointSource.cc \
	MNS/MeqRequest.cc \
	MNS/MeqResult.cc \
	MNS/MeqResultVec.cc \
    MNS/MeqSource.cc \
    MNS/MeqStatUVW.cc

bin_PROGRAMS = versionbbskernel metams

versionbbskernel_SOURCES      = versionbbskernel.cc
versionbbskernel_LDADD        = libbbskernel.la
versionbbskernel_DEPENDENCIES = libbbskernel.la $(LOFAR_DEPEND)

metams_SOURCES      = metams.cc
metams_LDADD        = libbbskernel.la
metams_DEPENDENCIES = libbbskernel.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
