## ------------------------------------------------------------------------------
## Build the tests

## --- tObjectFactory -------------------------------

file (GLOB tObjectFactory_SRCS tObjectFactory.cc
                               Shapes.h
			       Shapes.cc
			       Coordinates.h
			       Coordinates.cc )

## compiler instructions
add_executable (tObjectFactory ${tObjectFactory_SRCS})
## linker instructions
target_link_libraries (tObjectFactory 
                       common 
	               ${LOG4CPLUS_LIBRARIES}
		       ${BACKTRACE_LIBRARIES} )
			 
## testing instructions
add_test (tObjectFactory tObjectFactory)

## --- Test -----------------------------------------

foreach (Common_test
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
    tSystemUtil
    tSaxpy
    tPrettyUnits
    tBacktrace
    tParameterValue
    tParameterSet
    tParameterSetPerf
    )

  ## compiler instructions
  add_executable (${Common_test} ${Common_test}.cc)
  ## linker instructions
  target_link_libraries (${Common_test} 
                         common 
			 ${LOG4CPLUS_LIBRARIES}
			 ${BACKTRACE_LIBRARIES} )

  ## testing instructions
  add_test (${Common_test} ${Common_test})
endforeach (Common_test)
