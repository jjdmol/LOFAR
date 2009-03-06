## ------------------------------------------------------------------------------
## Include files

file (GLOB libplc_HEADERS DH_ProcControl.h
			  BGLConnection.h
			  PCCmd.h
			  ProcControlComm.h
			  ProcControlServer.h
			  ProcessControl.h
			  ProcCtrlProxy.h
			  ProcCtrlCmdLine.h
			  ProcCtrlRemote.h
			  ACCmain.h )

## ------------------------------------------------------------------------------
## Installation
   
install (FILES ${libplc_HEADERS}
  DESTINATION include/PLC
  )
