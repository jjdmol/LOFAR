# $Id$

include(LofarPackageVersion)
include(PythonInstall)

lofar_add_library(awimager2lib
  Package__Version.cc
  FFTCMatrix.cc
  ATerm.cc
  ATermLofar.cc
  ATermPython.cc
  ConvolutionFunction.cc
  ConvolutionFunctionDiagonal.cc
  Imager.cc
  CFStore.cc
  FTMachine.cc
#   FTMachineSimple.cc
#   FTMachineSimpleWB.cc
  FTMachineWStackWB.cc
#   FTMachineIDG.cc
  WTerm.cc
  VisBuffer.cc
  VisibilityIterator.cc
  VisImagingWeight.cc
  VisImagingWeightRobust.cc
  VisResampler.cc
  VisResamplerMatrixWB.cc
  VisResamplerDiagonalWB.cc
  SkyEquation.cc
  util.cc
  ScopedTimer.cc
)

lofar_add_bin_program(awimager2 
  Package__Version.cc
  awimager2.cc 
  Operation.cc 
  OperationImage.cc 
  OperationClean.cc 
  OperationEmpty.cc
  OperationPredict.cc
)
  
if (IDG_FOUND)
  lofar_add_library(ftmachineidg
    FTMachineIDG.cc
  )
endif(IDG_FOUND)

# lofar_add_library(ftmachinesimple
#   FTMachineSimple.cc
# )

lofar_add_bin_program(versionawimager2 versionawimager2.cc)


python_install(
  __init__.py 
  myaterm.py
  atermimages.py
  DESTINATION lofar/imager)
  
  
# Suppress compiler warnings from casarest by disabling them for some sources
set_source_files_properties(
  FTMachine.cc
  FTMachineWsplit.cc
  Imager.cc
  VisResampler.cc
  VisResamplerOld.cc
  awimager.cc
  PROPERTIES COMPILE_FLAGS
    "-Wno-extra -Wno-ignored-qualifiers -Wno-overloaded-virtual -Wno-unused"
)

set_source_files_properties(
  VisResamplerMatrixWB.cc
  ConvolutionFunction.cc
  PROPERTIES COMPILE_FLAGS
    "-O3 -Wno-extra -Wno-ignored-qualifiers -Wno-overloaded-virtual -Wno-unused"
)
#  -march=native 
