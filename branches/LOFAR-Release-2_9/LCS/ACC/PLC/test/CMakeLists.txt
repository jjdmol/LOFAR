include(LofarCTest)

set(CHECK_PROGRAMS
  tPCCmd)

foreach(prog ${CHECK_PROGRAMS})
  lofar_add_executable(${prog} ${prog}.cc)
endforeach(prog ${CHECK_PROGRAMS})
lofar_add_executable(tACCmain tACCmain.cc APCmdImpl.cc)
