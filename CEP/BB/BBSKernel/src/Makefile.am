lib_LTLIBRARIES = libbbskernel.la

libbbskernel_la_SOURCES = Package__Version.cc  	\
	Axis.cc			\
	BlobIOExtensions.cc	\
	Grid.cc			\
	Instrument.cc		\
	MeasurementAIPS.cc	\
	MetaMeasurement.cc	\
	Model.cc		\
	ModelConfig.cc \
	Prediffer.cc		\
	Solver.cc		\
	SolverInterfaceTypes.cc	\
	VisData.cc		\
	VisDimensions.cc	\
	VisSelection.cc		\
	MNS/ExternalFunction.cc		\
	MNS/MeqAzEl.cc		\
	MNS/MeqDFTPS.cc		\
	MNS/MeqDiag.cc		\
	MNS/MeqDipoleBeam.cc	\
	MNS/MeqDipoleBeamExternal.cc	\
	MNS/MeqDomain.cc	\
	MNS/MeqExpr.cc		\
	MNS/MeqFunklet.cc	\
	MNS/MeqGaussianCoherency.cc	\
	MNS/MeqGaussianSource.cc	\
	MNS/MeqJonesCMul2.cc	\
	MNS/MeqJonesCMul3.cc	\
	MNS/MeqJonesExpr.cc	\
	MNS/MeqJonesInvert.cc	\
	MNS/MeqJonesMul.cc	\
	MNS/MeqJonesMul2.cc	\
	MNS/MeqJonesMul3.cc	\
	MNS/MeqJonesNode.cc	\
	MNS/MeqJonesSum.cc	\
	MNS/MeqJonesVisData.cc	\
	MNS/MeqLMN.cc		\
	MNS/MeqMatrix.cc	\
	MNS/MeqMatrixComplexArr.cc \
	MNS/MeqMatrixComplexSca.cc \
	MNS/MeqMatrixRealArr.cc	\
	MNS/MeqMatrixRealSca.cc	\
	MNS/MeqMatrixRep.cc	\
	MNS/MeqMatrixTmp.cc	\
	MNS/MeqNumericalDipoleBeam.cc \
	MNS/MeqParm.cc		\
	MNS/MeqParmExpr.cc	\
	MNS/MeqParmFunklet.cc	\
	MNS/MeqParmSingle.cc	\
	MNS/MeqPhaseRef.cc	\
	MNS/MeqPhaseShift.cc	\
	MNS/MeqPointCoherency.cc	\
	MNS/MeqPointSource.cc	\
	MNS/MeqPolc.cc		\
	MNS/MeqPolcLog.cc	\
	MNS/MeqRequest.cc	\
	MNS/MeqResult.cc	\
	MNS/MeqResultVec.cc	\
	MNS/MeqSource.cc	\
	MNS/MeqSourceList.cc	\
	MNS/MeqStatExpr.cc	\
	MNS/MeqStation.cc	\
	MNS/MeqStatUVW.cc	\
	MNS/MeqTabular.cc 

bin_PROGRAMS = versionbbskernel metams

versionbbskernel_SOURCES      = versionbbskernel.cc
versionbbskernel_LDADD        = libbbskernel.la
versionbbskernel_DEPENDENCIES = libbbskernel.la $(LOFAR_DEPEND)

metams_SOURCES      = metams.cc
metams_LDADD        = libbbskernel.la
metams_DEPENDENCIES = libbbskernel.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
