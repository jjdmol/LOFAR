find_path (HAVE_LINK_IONPROC IONProc ${CMAKE_CURRENT_SOURCE_DIR})
if (HAVE_LINK_IONPROC)
  set (HAVE_LINK_IONPROC TRUE CACHE BOOL "Have link IONProc?" FORCE)
  execute_process(COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/IONProc )
endif (HAVE_LINK_IONPROC)

execute_process(COMMAND ln -s ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/IONProc )


## ------------------------------------------------------------------------------
## Include files

file (GLOB libinterface_HEADERS BeamletBuffer.h
				CN_Personality.h
				InputSection.h
				InputThread.h
				InputThreadAsm.h
				ION_Allocator.h
				LockedRanges.h
				LogThread.h
				OutputThread.h
				OutputSection.h
				ReaderWriterSynchronization.h
				RSP.h
				Scheduling.h
				SlidingPointer.h
				WallClockTime.h
				WH_DelayCompensation.h
				FCNP_ServerStream.h)
 
enable_language(ASM-BGP)

if (CMAKE_ASM-BGP_COMPILER_WORKS)
  set(amsSrcs InputThreadAsm.S)
endif (CMAKE_ASM-BGP_COMPILER_WORKS)  

## ------------------------------------------------------------------------------
## Build the library

file (GLOB libionproc_SOURCES BeamletBuffer.cc
			      CN_Personality.cc
			      InputSection.cc
			      InputThread.cc
			      ION_Allocator.cc
			      LogThread.cc
			      OutputThread.cc
			      OutputSection.cc
			      ReaderWriterSynchronization.cc
			      Scheduling.cc
			      WH_DelayCompensation.cc
			      FCNP_ServerStream.cc
			      ${amsSrcs})

add_library (ionproc ${libionproc_SOURCES})

## linker instructions
target_link_libraries (  ionproc 
                         amcimpl
			 amcbase
			 plc
			 transport
			 blob
			 interface
			 stream
			 fcnp_cn
			 fcnp_ion
			 applcommon
                         common
			 ${Boost_LIBRARIES}
			 ${CASA_LIBRARIES}
			 ${BACKTRACE_LIBRARIES}
			 )

set_target_properties (ionproc
  PROPERTIES
  LINK_FLAGS "-lpthread"
)

add_executable(IONProc ION_main.cc)

target_link_libraries (IONProc
                       ionproc
                      )

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS ionproc IONProc
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  )
