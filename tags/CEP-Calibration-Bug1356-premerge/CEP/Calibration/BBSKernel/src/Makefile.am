#AM_CXXFLAGS = -Wno-ignored-qualifiers -Werror
lib_LTLIBRARIES = libbbskernel.la

#	Expr/DipoleBeam.cc

#	Equator.cc \
#	Expr/Diag.cc \
#	Expr/ExternalFunction.cc \
#	Expr/GaussianCoherence.cc \
#	Expr/GaussianSource.cc \
#	Expr/HamakerDipole.cc \
#	Expr/JonesCMul2.cc \
#	Expr/JonesExpr.cc \
#	Expr/JonesMul2.cc \
#	Expr/JonesMul3.cc \
#	Expr/JonesNode.cc \
#	Expr/JonesVisData.cc \
#	Expr/Result.cc \
#	Expr/ResultVec.cc \
#	Expr/YatawattaDipole.cc
#	Expr/JonesCMul3.cc \
#	Expr/JonesMul.cc \
#	Expr/JonesSum.cc \
#	Expr/PointCoherence.cc \
#	Expr/Expr.cc \
#	Expr/PointSource.cc \
#	Expr/Source.cc

#	Expr/UVW.cc

libbbskernel_la_SOURCES = Package__Version.cc \
	BlobIOExtensions.cc \
	Equator.cc \
	Evaluator.cc \
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
	Expr/AzEl.cc \
	Expr/Cache.cc \
	Expr/ConditionNumber.cc \
	Expr/DFTPS.cc \
	Expr/Expr.cc \
	Expr/ExprParm.cc \
	Expr/ExprResult.cc \
	Expr/FlagArray.cc \
	Expr/JonesInvert.cc \
	Expr/JonesVisData.cc \
	Expr/LMN.cc \
	Expr/Matrix.cc \
	Expr/MatrixComplexArr.cc \
	Expr/MatrixComplexSca.cc \
	Expr/MatrixMul2.cc \
	Expr/MatrixMul3.cc \
	Expr/MatrixRealArr.cc \
	Expr/MatrixRealSca.cc \
	Expr/MatrixRep.cc \
	Expr/MatrixSum.cc \
	Expr/MatrixTmp.cc \
	Expr/MergeFlags.cc \
	Expr/MIM.cc \
	Expr/Mul.cc \
	Expr/PhaseRef.cc \
	Expr/PhaseShift.cc \
	Expr/PiercePoint.cc \
	Expr/PointCoherence.cc \
	Expr/PointSource.cc \
	Expr/Request.cc \
	Expr/Source.cc \
	Expr/StatUVW.cc \
	Expr/StationUVW.cc

bin_PROGRAMS = versionbbskernel

versionbbskernel_SOURCES      = versionbbskernel.cc
versionbbskernel_LDADD        = libbbskernel.la
versionbbskernel_DEPENDENCIES = libbbskernel.la $(LOFAR_DEPEND)

include $(top_srcdir)/Makefile.common
