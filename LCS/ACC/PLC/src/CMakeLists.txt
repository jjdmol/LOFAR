## ------------------------------------------------------------------------------
## Build the library

file (GLOB libplc_SOURCES DH_ProcControl.cc
			  PCCmd.cc
			  ProcControlComm.cc
			  ProcControlServer.cc
			  ProcCtrlProxy.cc
			  ProcCtrlCmdLine.cc
			  ProcCtrlRemote.cc
			  ACCmain.cc)

add_library (plc ${libplc_SOURCES})

## ------------------------------------------------------------------------------
## Installation

install (
  TARGETS plc
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
