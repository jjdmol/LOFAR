
foreach (LCS_Common_test
    testLogger
    tAllocator
    tDataConvert
    tFileLocator
    tStringUtil
    tStreamUtil
    tHexdump
    testSocket
    tComplex
    tProcess
    tRunOnNode
    tTimer
    tNumeric
    tSingleton
#    tObjectFactory
    tSystemUtil
    tSaxpy
    tPrettyUnits
    )
  ## compiler instructions
  add_executable (${LCS_Common_test} ${LCS_Common_test}.cc)
  ## linker instructions
  target_link_libraries (${LCS_Common_test} common)
  ## testing instructions
  add_test (${LCS_Common_test} ${LCS_Common_test})
endforeach (LCS_Common_test)