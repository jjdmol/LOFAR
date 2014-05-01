# $Id$

include(LofarPackageVersion)
include(PythonInstall)

lofar_add_library(lofarft
  Package__Version.cc
  FFTCMatrix.cc
  ATerm.cc
  ATermLofar.cc
#  ATermPython.cc
  ConvolutionFunction.cc
  Imager.cc
  CFStore.cc
  FTMachine.cc
  FTMachineSimpleWB.cc
  FTMachineSplitBeamWStackWB.cc
  WTerm.cc
  VisBuffer.cc
  VisibilityIterator.cc
  VisImagingWeight.cc
  VisResampler.cc
  VisResamplerMatrixWB.cc
  SkyEquation.cc
)

lofar_add_bin_program(awimager 
  Package__Version.cc
  awimager.cc 
  Operation.cc 
  OperationImage.cc 
  OperationClean.cc 
  OperationEmpty.cc
  OperationPredict.cc
)
  

# lofar_add_library(ftmachinesimple
#   FTMachineSimple.cc
# )

lofar_add_bin_program(versionlofarft versionlofarft.cc)


# set_source_files_properties(
#   FTMachineSimple.cc
#   PROPERTIES COMPILE_FLAGS
#     "-fno-implicit-templates"
# )

# lofar_add_bin_program(versionlofarft versionlofarft.cc)
# lofar_add_bin_program(makefftwisdom2d makefftwisdom2d.cc)
# lofar_add_bin_program(fillRootImageGroup fillRootImageGroup.cc)
# lofar_add_bin_program(makebeamimage makebeamimage.cc)

install(PROGRAMS
  addImagingInfo
  DESTINATION bin)

# Python modules.
python_install(
  addImagingInfo.py 
  get_rms_noise.py
  DESTINATION lofar)

python_install(
  __init__.py 
  myaterm.py
  DESTINATION lofar/imager)

  
  
# Suppress compiler warnings from casarest by disabling them for some sources
set_source_files_properties(
  CubeSkyEquation.cc
  FTMachine.cc
  FTMachineWsplit.cc
  Imager.cc
  VisResampler.cc
  VisResamplerOld.cc
  awimager.cc
  PROPERTIES COMPILE_FLAGS
    "-Wno-extra -Wno-ignored-qualifiers -Wno-overloaded-virtual -Wno-unused"
)

