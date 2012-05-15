# $Id$

include(LofarCTest)
include(checkCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-fstrict-aliasing" COMPILER_SUPPORTS_FSTRICT_ALIASING)

lofar_add_test(tFillRow tFillRow.cc)
lofar_add_test(tConvolveBLAS tConvolveBlAS.cc)
lofar_add_test(tModelImageFFT tModelImageFFT.cc)
#lofar_add_test(tJonesCMul3 tJonesCMul3.cc utils.cc)

# Set compile flag for strict-aliasing (don't know why this is not triggered by O2 already)
if ("${COMPILER_SUPPORTS_FSTRICT_ALIASING}")
   SET_TARGET_PROPERTIES (tConvolveBLAS PROPERTIES COMPILE_FLAGS "-fstrict-aliasing")
endif ("${COMPILER_SUPPORTS_FSTRICT_ALIASING}")

# Support for BLAS on OS X through -framework veclib
if (APPLE)
   SET_TARGET_PROPERTIES (tModelImageFFT PROPERTIES COMPILE_FLAGS "-framework veclib")
   SET_TARGET_PROPERTIES (tModelImageFFT PROPERTIES COMPILE_FLAGS "-framework veclib")
endif (APPLE)

