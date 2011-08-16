# $Id$

# Do not split the following line, otherwise makeversion will fail!
lofar_package(CUDaemons 1.0 DEPENDS Common ApplCommon APLCommon GCFTM MACIO)

add_definitions(-DBOOST_DISABLE_THREADS)

add_subdirectory(src)
#add_subdirectory(test)    # Tests currently do not compile!
