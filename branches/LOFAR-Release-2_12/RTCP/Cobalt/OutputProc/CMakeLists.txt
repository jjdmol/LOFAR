# $Id$

lofar_package(OutputProc 1.0 DEPENDS Common Stream CoInterface MSLofar LofarStMan MACIO MessageBus)

include(LofarFindPackage)

lofar_find_package(OpenMP REQUIRED)
lofar_find_package(Boost REQUIRED)
lofar_find_package(Casacore COMPONENTS casa ms tables REQUIRED)
lofar_find_package(DAL REQUIRED)  # https://github.com/nextgen-astrodata/DAL
# Skip explicit lofar_find_package(HDF5 REQUIRED) needed for casacore and DAL.
# On RHEL/CentOS 7 an explicit find HDF5 returns into OutputProc:
#   -D_BSD_SOURCE -D_FORTIFY_SOURCE=2 -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE
# always _BSD_SOURCE is risky; _FORTIFY_SOURCE produces warnings on gnu_debug (missing -O).

lofar_find_package(UnitTest++)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(scripts)
