# $Id$

# Create symbolic link to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/include/${PACKAGE_NAME})

# Install header files.
install(FILES
  Equator.h
  Evaluator.h
  Exceptions.h
  ExprSet.h
  Instrument.h
  Measurement.h
  MeasurementAIPS.h
  Model.h
  ModelConfig.h
  ParmManager.h
  ParmProxy.h
  Solver.h
  SolverInterfaceTypes.h
  Types.h
  VisData.h
  VisDimensions.h
  VisExpr.h
  VisSelection.h
  DESTINATION include/${PACKAGE_NAME})
install(FILES
  Expr/ArrayFactor.h
  Expr/AzEl.h
  Expr/BasicExpr.h
  Expr/Cache.h
  Expr/ConditionNumber.h
  Expr/DFTPS.h
  Expr/Expr.h
  Expr/ExprId.h
  Expr/ExprAdaptors.h
  Expr/ExprParm.h
  Expr/ExprResult.h
  Expr/ExprValueView.h
  Expr/ExprValueIterator.h
  Expr/ExprVisData.h
  Expr/ExternalFunction.h
  Expr/FlagArray.h
  Expr/FlagIf.h
  Expr/GaussianCoherence.h
  Expr/GaussianSource.h
  Expr/HamakerDipole.h
  Expr/JonesInvert.h
  Expr/JonesVisData.h
  Expr/Literal.h
  Expr/LMN.h
  Expr/Matrix.h
  Expr/MatrixComplexArr.h
  Expr/MatrixComplexSca.h
  Expr/MatrixMul2.h
  Expr/MatrixMul3.h
  Expr/MatrixRealArr.h
  Expr/MatrixRealSca.h
  Expr/MatrixRep.h
  Expr/MatrixSum.h
  Expr/MatrixTmp.h
  Expr/MergeFlags.h
  Expr/MIM.h
  Expr/Mul.h
  Expr/PhaseShift.h
  Expr/PiercePoint.h
  Expr/PointCoherence.h
  Expr/PointSource.h
  Expr/RefCounting.h
  Expr/Request.h
  Expr/Resampler.h
  Expr/Source.h
  Expr/SpectralIndex.h
  Expr/StatUVW.h
  Expr/ValueSet.h
  Expr/YatawattaDipole.h
  DESTINATION include/${PACKAGE_NAME}/Expr)
