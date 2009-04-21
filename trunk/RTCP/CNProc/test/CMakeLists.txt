cmake_minimum_required (VERSION 2.6)

include(LOFARTest)
include_directories ( ${LOFAR_SOURCE_DIR}/RTCP/CNProc/test )

enable_language(ASM-BGP)

if (CMAKE_ASM-BGP_COMPILER_WORKS)
  file(GLOB amsSrcs ../src/CorrelatorAsm.S
                    ../src/FFT_Asm.S
		    ../src/FIR_Asm.S)
endif (CMAKE_ASM-BGP_COMPILER_WORKS) 

## ------------------------------------------------------------------------------
## Sourses

file (GLOB tCN_Processing_SRCS         tBeamForming.cc
				       tCN_Processing.cc
				       ../src/BandPass.cc
				       ../src/CN_Processing.cc
				       ../src/Correlator.cc
				       ../src/FIR.cc
				       ../src/BeamFormer.cc
				       ../src/LocationInfo.cc
				       ../src/PPF.cc
				       ../src/Transpose.cc
				       ../src/PencilBeams.cc
				       ../src/Stokes.cc
				       ../src/AsyncCommunication.cc
				       ../src/AsyncTranspose.cc
				       ${amsSrcs} )
				       
## --- Macro(s) -------------------------------------

MACRO (TARGETLINKLIBRARIES_CNPROC_TEST _PROG)
  target_link_libraries (${_PROG} 
                         cnproc )
ENDMACRO (TARGETLINKLIBRARIES_CNPROC_TEST)

## --- Programs to be built -------------------------

set (CHECKPROGS
     tBeamForming
     tCN_Processing
)

foreach (prog ${CHECKPROGS})
  add_executable (${prog} ${${prog}_SRCS})
  TARGETLINKLIBRARIES_CNPROC_TEST (${prog})
  set_target_properties(${prog} PROPERTIES LINK_FLAGS "-static")
endforeach (prog ${CHECKPROGS})				       

