# $Id$

# Do not split the following line, otherwise makeversion will fail!
#lofar_package(CUDaemons 1.0 DEPENDS Common ApplCommon GCFTM MACIO)

#add_definitions(-DBOOST_DISABLE_THREADS)

#add_subdirectory(src)
##add_subdirectory(test)    # Tests currently do not compile!



#include(LofarPackageVersion)

#lofar_add_library(cudaemons Package__Version.cc)

lofar_add_package(Service_Broker)
lofar_add_package(CT_StartDaemon)
