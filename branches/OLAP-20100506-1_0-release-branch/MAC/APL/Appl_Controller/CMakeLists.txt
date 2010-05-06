# $Id: CMakeLists.txt 13516 2009-06-30 10:05:44Z loose $

lofar_package(Appl_Controller 2.0 DEPENDS Common MACIO ALC PLC)

lofar_add_bin_program(ApplController 
  ApplControllerMain.cc
  ACCmdImpl.cc
  ACDaemonComm.cc
  APAdmin.cc
  APAdminPool.cc
  CmdStack.cc
  ItemList.cc
  ProcRule.cc
  ProcRuler.cc
  PR_Shell.cc
  PR_MPI.cc
  PR_BGL.cc
  StateEngine.cc
  ApplController.cc)
                                                  
lofar_add_bin_program(ACDaemon 
  ACDaemonMain.cc
  ACRequestPool.cc
  ACDaemon.cc)

lofar_add_bin_program(ACcli ACcli.cc)

lofar_add_bin_program(ACuserMenu ACuserMenu.cc)

install(PROGRAMS 
  startAP.sh
  startBGL.sh
  startMPI.sh
  stopAP.sh
  stopBGL.sh 
  stopMPI.sh
  DESTINATION bin)
  
install(FILES
  ACDaemon.log_prop 
  ACDaemon.conf
  ApplController.log_prop
  ACuserMenu.log_prop
  DESTINATION etc)
