# $Id$

# Generate package/version sources
include(LofarPackageVersion)

# Create symbolic links to include directory.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/fft
  ${CMAKE_BINARY_DIR}/include/fft)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/fitting
  ${CMAKE_BINARY_DIR}/include/fitting)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR}/utils
  ${CMAKE_BINARY_DIR}/include/utils)

# Add library
lofar_add_library(askap_scimath
  Package__Version.cc
  fft/FFTWrapper.cc
  fitting/Axes.cc
  fitting/ComplexDiff.cc
  fitting/ComplexDiffMatrix.cc
  fitting/CompositeEquation.cc
  fitting/DesignMatrix.cc
  fitting/Equation.cc
  fitting/GenericEquation.cc
  fitting/GenericNormalEquations.cc
  fitting/GSLSVDReplacement.cc
  fitting/ImagingEquation.cc
  fitting/ImagingNormalEquations.cc
  fitting/ISerializable.cc
  fitting/LinearSolver.cc
  fitting/ParamsCasaTable.cc
  fitting/Params.cc
  fitting/ParamsTable.cc
  fitting/PolynomialEquation.cc
  fitting/Quality.cc
  fitting/Solveable.cc
  fitting/Solver.cc
  utils/MultiDimArrayPlaneIter.cc
  utils/PaddingUtils.cc
  utils/PolConverter.cc)
