lib_LTLIBRARIES = libbbskernel.la

#	Expr/ExternalFunction.cc \
#	Expr/MeqAzEl.cc \
#	Expr/MeqDiag.cc \
#	Expr/MeqDipoleBeam.cc \
#	Expr/MeqDipoleBeamExternal.cc \
#	Expr/MeqGaussianCoherency.cc \
#	Expr/MeqGaussianSource.cc \
#	Expr/MeqJonesCMul2.cc \
#	Expr/MeqJonesMul3.cc \
#	Expr/MeqNumericalDipoleBeam.cc

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
	Expr/ExprParm.cc \
	Expr/MeqDFTPS.cc \
	Expr/MeqExpr.cc \
	Expr/MeqLMN.cc \
	Expr/MeqJonesCMul3.cc \
	Expr/MeqJonesExpr.cc \
	Expr/MeqJonesInvert.cc \
	Expr/MeqJonesMul.cc \
	Expr/MeqJonesMul2.cc \
	Expr/MeqJonesNode.cc \
	Expr/MeqJonesSum.cc \
	Expr/MeqJonesVisData.cc \
	Expr/MeqMatrix.cc \
	Expr/MeqMatrixComplexArr.cc \
	Expr/MeqMatrixComplexSca.cc \
	Expr/MeqMatrixRealArr.cc \
	Expr/MeqMatrixRealSca.cc \
	Expr/MeqMatrixRep.cc \
	Expr/MeqMatrixTmp.cc \
	Expr/MeqPhaseRef.cc \
    Expr/MeqPhaseShift.cc \
	Expr/MeqPointCoherency.cc \
	Expr/MeqPointSource.cc \
	Expr/MeqRequest.cc \
	Expr/MeqResult.cc \
	Expr/MeqResultVec.cc \
    Expr/MeqSource.cc \
    Expr/MeqStatUVW.cc

bin_PROGRAMS = versionbbskernel metams

versionbbskernel_SOURCES      = versionbbskernel.cc
versionbbskernel_LDADD        = libbbskernel.la
versionbbskernel_DEPENDENCIES = libbbskernel.la $(LOFAR_DEPEND)

metams_SOURCES      = metams.cc
metams_LDADD        = libbbskernel.la
metams_DEPENDENCIES = libbbskernel.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
