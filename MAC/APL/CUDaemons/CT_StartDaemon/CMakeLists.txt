# $Id$

lofar_package(CT_StartDaemon 1.0 DEPENDS Common ApplCommon GCFTM MACIO APLCommon)

include(LofarPackageVersion)

lofar_add_library(ctstartdaemon
  Package__Version.cc)

lofar_add_bin_program(CTStartDaemon CTStartDaemonMain.cc CTStartDaemon.cc)

install(PROGRAMS startController.sh DESTINATION bin)
install(FILES CTStartDaemon.conf DESTINATION etc)
