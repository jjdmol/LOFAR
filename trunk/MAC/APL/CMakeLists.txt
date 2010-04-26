# $Id$

# Create directory APL in the binary include directory; it will hold symbolic
# links to the different subcomponents of APL.
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
  ${CMAKE_BINARY_DIR}/include/APL)

lofar_add_package(APLCommon)       # Common tasks and functions
lofar_add_package(RTCCommon)       # Real Time Control functions
lofar_add_package(RTDBCommon)      # RealTimeDataBase common software
lofar_add_package(TestSuite)       # 
lofar_add_package(CUDaemons)       # Control Unit Daemons
lofar_add_package(CURTDBDaemons)   # Control Unit RTDB Daemons
lofar_add_package(PIC)             # Physical Instrument Components (?)
lofar_add_package(PAC)             # ???
lofar_add_package(VHECR)           # VHECR related code
lofar_add_package(Appl_Controller) # ACC based controller
lofar_add_package(CEPCU)           # CEP-related MAC controllers
lofar_add_package(MainCU)          # Main Control Unit
lofar_add_package(StationCU)       # Station Control Unit software
lofar_add_package(TestCtlr)        # Testprograms for MAC controllers
