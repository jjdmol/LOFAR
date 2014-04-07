# $Id$

include(LofarPackageVersion)
include(PythonInstall)

lofar_add_library(lofarft
  Package__Version.cc
  FFTCMatrix.cc
  LofarATerm.cc
  LofarATermOld.cc
  LofarConvolutionFunction.cc
  LofarConvolutionFunctionOld.cc
  LofarImager.cc
  LofarCFStore.cc
  LofarFTMachine.cc
  LofarFTMachineOld.cc
  LofarWTerm.cc
  LofarVisResampler.cc
  LofarVisResamplerOld.cc
  LofarCubeSkyEquation.cc
  PythonFTMachine.cc
  VisBufferProxy.cc
  VisibilityResamplerProxy.cc
  ROVisibilityIteratorProxy.cc
)

lofar_add_bin_program(versionlofarft versionlofarft.cc)
lofar_add_bin_program(awimager awimager.cc)
lofar_add_bin_program(makefftwisdom2d makefftwisdom2d.cc)
lofar_add_bin_program(fillRootImageGroup fillRootImageGroup.cc)
lofar_add_bin_program(makebeamimage makebeamimage.cc)

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
 lofarftmachine.py
 rovisibilityiterator.py
 myftmachine.py 
 DESTINATION lofar/imager)

lofar_add_library(pylofarft MODULE pylofarft.cc)
SET_TARGET_PROPERTIES(pylofarft PROPERTIES
 PREFIX ""
 LIBRARY_OUTPUT_DIRECTORY ${PYTHON_BUILD_DIR}/lofar/imager)
target_link_libraries(pylofarft lofarft)

# This is a quick-and-dirty fix to install the Python binding module in the
# right place. It will now be installed twice, because lofar_add_library()
# will install it in $prefix/$libdir
install(TARGETS pylofarft
 DESTINATION ${PYTHON_INSTALL_DIR}/lofar/imager)
