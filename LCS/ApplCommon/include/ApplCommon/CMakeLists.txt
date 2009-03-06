## ------------------------------------------------------------------------------
## Include files

file (GLOB libapplcommon_HEADERS Observation.h )

## ------------------------------------------------------------------------------
## Installation
   
install (FILES ${libapplcommon_HEADERS}
  DESTINATION include/ApplCommon
  )
