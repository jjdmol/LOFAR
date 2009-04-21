find_path (HAVE_LINK_CNPROC CNProc ${CMAKE_CURRENT_SOURCE_DIR})
if (HAVE_LINK_CNPROC)
  set (HAVE_LINK_CNPROC TRUE CACHE BOOL "Have link CNProc?" FORCE)
  execute_process(COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/CNProc )
endif (HAVE_LINK_CNPROC)

execute_process(COMMAND ln -s ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/CNProc )


## ------------------------------------------------------------------------------
## Include files

file (GLOB libinterface_HEADERS BandPass.h
				LocationInfo.h
				CorrelatorAsm.h
				FFT_Asm.h
				FIR_Asm.h
				InputData.h
				TransposedData.h
				FIR.h
				PPF.h
				AsyncTranspose.h
				BeamFormer.h
				BeamFormerAsm.h
				Correlator.h
				CN_Processing.h
				FCNP_ClientStream.h
				AsyncCommunication.h
				PencilBeams.h
				Stokes.h)
 
enable_language(ASM-BGP)

if (CMAKE_ASM-BGP_COMPILER_WORKS)
  file(GLOB amsSrcs BeamFormerAsm.S
		    BeamFormerAsm3St6Bm.inc
		    BeamFormerAsm6St3Bm.inc
		    CorrelatorAsm.S
                    FIR_Asm.S
		    FFT_Asm.S)
endif (CMAKE_ASM-BGP_COMPILER_WORKS)  

## ------------------------------------------------------------------------------
## Build the library

file (GLOB libcnproc_SOURCES BandPass.cc
			     LocationInfo.cc
			     FIR.cc
			     AsyncTranspose.cc
			     BeamFormer.cc
			     PPF.cc
			     Correlator.cc
			     CN_Processing.cc
			     FCNP_ClientStream.cc
			     AsyncCommunication.cc
			     PencilBeams.cc
			     Stokes.cc
			     ${amsSrcs})

add_library (cnproc ${libcnproc_SOURCES})

## linker instructions
target_link_libraries (  cnproc 
			 interface
			 stream
			 fcnp_cn
			 fcnp_ion
			 applcommon
                         common
			 ${Boost_LIBRARIES}
			 ${CASA_LIBRARIES}
			 ${BACKTRACE_LIBRARIES}
			 ${WITH-LIBS}
			 )

add_executable(CN_Processing CN_Processing_main.cc)

target_link_libraries (CN_Processing
                       cnproc
                      )

SET (CMAKE_ASM-BGP_LINK_EXECUTABLE "${ASM-BGP} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set_target_properties(CN_Processing
                      PROPERTIES LINK_FLAGS "-static -g -O2 -W -Wall -Woverloaded-virtual -Wno-unknown-pragmas -B/bgsys/drivers/ppcfloor/gnu-linux/powerpc-bgp-linux/bin"
                                 LINKER_LANGUAGE ASM-BGP
)

#set_target_properties(CN_Processing
#                      LINK_SEARCH_END_STATIC TRUE
#)

#set_target_properties(CN_Processing
#                      LINK_TYPE STATIC
#)

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS cnproc CN_Processing
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  )
