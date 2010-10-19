# $Id$

include(LofarPackageVersion)

add_subdirectory(Contrib)

lofar_add_library(bbskernel
  Package__Version.cc
	Equator.cc
	Evaluator.cc
    Instrument.cc
	MeasurementAIPS.cc
	Model.cc
	ModelConfig.cc
	ParmManager.cc
	ParmProxy.cc
	Solver.cc
	SolverInterfaceTypes.cc
	VisData.cc
	VisDimensions.cc
	VisExpr.cc
	VisSelection.cc
	Expr/ArrayFactor.cc
	Expr/AzEl.cc
	Expr/Cache.cc
	Expr/ConditionNumber.cc
	Expr/Expr.cc
	Expr/ExprAdaptors.cc
	Expr/ExprParm.cc
	Expr/ExprValue.cc
	Expr/ExprValueView.cc
	Expr/ExprValueIterator.cc
	Expr/ExprVisData.cc
	Expr/ExternalFunction.cc
	Expr/FlagArray.cc
	Expr/GaussianCoherence.cc
	Expr/GaussianSource.cc
	Expr/HamakerDipole.cc
	Expr/Literal.cc
	Expr/LMN.cc
	Expr/Matrix.cc
	Expr/MatrixComplexArr.cc
	Expr/MatrixComplexSca.cc
	Expr/MatrixInverse.cc
	Expr/MatrixMul2.cc
	Expr/MatrixMul3.cc
	Expr/MatrixRealArr.cc
	Expr/MatrixRealSca.cc
	Expr/MatrixRep.cc
	Expr/MatrixSum.cc
	Expr/MatrixTmp.cc
	Expr/PhaseShift.cc
	Expr/PiercePoint.cc
	Expr/PointCoherence.cc
	Expr/PointSource.cc
	Expr/PolynomialPhaseScreen.cc
	Expr/Request.cc
	Expr/Resampler.cc
	Expr/ScalarMatrixMul.cc
	Expr/Source.cc
	Expr/SpectralIndex.cc
	Expr/StationShift.cc
	Expr/StationUVW.cc
	Expr/ValueSet.cc
	Expr/YatawattaDipole.cc)

# Because we're using dlopen()
target_link_libraries(bbskernel ${CMAKE_DL_LIBS})

lofar_add_bin_program(versionbbskernel versionbbskernel.cc)
