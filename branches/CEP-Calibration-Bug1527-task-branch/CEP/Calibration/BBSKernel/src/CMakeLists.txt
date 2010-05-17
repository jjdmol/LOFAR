# $Id$

include(LofarPackageVersion)

add_subdirectory(Contrib)

lofar_add_library(bbskernel
  Package__Version.cc
  BaselineFilter.cc
  BaselineMask.cc
  Correlation.cc
  CorrelationMask.cc
  ElementBeamExpr.cc
  Evaluator.cc
  Instrument.cc
  IonosphereExpr.cc
  MeasurementAIPS.cc
  MeasurementExprLOFAR.cc
  ModelConfig.cc
  ParmManager.cc
  ParmProxy.cc
  Solver.cc
  SolverInterfaceTypes.cc
  VisBuffer.cc
  VisDimensions.cc
  VisEquator.cc
  VisSelection.cc
  Expr/ArrayFactor.cc
  Expr/AzEl.cc
  Expr/CachePolicy.cc
  Expr/Cache.cc
  Expr/ConditionNumber.cc
  Expr/Element.cc
  Expr/EquatorialCentroid.cc
  Expr/Expr.cc
  Expr/ExprAdaptors.cc
  Expr/ExprParm.cc
  Expr/ExprValue.cc
  Expr/ExprValueView.cc
  Expr/ExprValueIterator.cc
  Expr/ExprVisData.cc
  Expr/ExternalFunction.cc
  Expr/FaradayRotation.cc
  Expr/FlagArray.cc
  Expr/GaussianCoherence.cc
  Expr/GaussianSource.cc
  Expr/HamakerDipole.cc
  Expr/LinearToCircularRL.cc
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
  Expr/PolynomialLayer.cc
  Expr/Request.cc
  Expr/Resampler.cc
  Expr/ScalarMatrixMul.cc
  Expr/Scope.cc
  Expr/Source.cc
  Expr/SpectralIndex.cc
  Expr/StationShift.cc
  Expr/StationUVW.cc
  Expr/TileArrayFactor.cc
  Expr/YatawattaDipole.cc)

# Because we're using dlopen()
target_link_libraries(bbskernel ${CMAKE_DL_LIBS})

lofar_add_bin_program(versionbbskernel versionbbskernel.cc)
