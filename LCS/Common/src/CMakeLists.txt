#make_package_version (${LCS_SOURCE_DIR}/Common/src TRUE TRUE)

## ------------------------------------------------------------------------------
## Build the library

if (Backtrace_FOUND)
  file (GLOB backtrace_SRCS AddressTranslator.cc
                            Backtrace.cc
			    SymbolTable.cc )
endif (Backtrace_FOUND)

file (GLOB libcommon_SOURCES Allocator.cc
			     DataConvert.cc
			     Exception.cc
			     FileLocator.cc
			     hexdump.cc
			     KVpair.cc
			     Lock.cc
			     lofar_bitset.cc
			     LofarLogCout.cc
			     LofarLogger.cc
			     Numeric.cc
			     ParameterSet.cc
			     ParameterSetImpl.cc
			     ParameterValue.cc
			     PrettyUnits.cc
			     Process.cc
			     ReadLine.cc
			     RunOnNode.cc
			     Stopwatch.cc
			     StreamUtil.cc
			     StringUtil.cc
			     SystemUtil.cc
			     Timer.cc
			     TypeNames.cc
			     Net/FdSet.cc
			     Net/Socket.cc
			     ${backtrace_SRCS})

add_library (common ${libcommon_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS common
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

